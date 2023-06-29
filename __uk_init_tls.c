// SPDX-License-Identifier: BSD-3-Clause
/*
 * Authors: Dragos Iulian Argint <dragosargint21@gmail.com>
 *
 * Copyright (c) 2022, University POLITEHNICA of Bucharest. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <uk/tcb_impl.h>
#include <elf.h>
#include <limits.h>
#include <sys/mman.h>
#include <string.h>
#include <stddef.h>
#include "pthread_impl.h"
#include "libc.h"
#include "atomic.h"
#include "syscall.h"

#include <uk/alloc.h>
#include <uk/assert.h>

/*
 * This glue code source is meant to replace `__libc_start_main()`
 * and tls related stuff.
 * If the pthread API is used, musl will allocate the TLS and will
 * provide a pointer to it in the `clone()` syscall. Musl will allocate
 * in fact a contiguous region which will have the following layout (Please
 * refer to the `pthread_create.c` file):
 * map -----------------------------------------------------------------
 *                     ^
 *                     |
 *                     | 4096      GUARD PAGE
 *                     |
 *                     v
 * map + guard ---------------------------------------------------------
 *                     ^
 *                     |
 *                     |
 *                     |           STACK
 *                     |
 *                     |
 *                     |
 *                     v
 * stack ---------------------------------------------------------------
 *                     ^           ^
 *                     |           |       TLS SPACE
 *                     |           v
 *               new ->|------------------------------------------------
 *                     |           ^
 *                     |           |
 *                     |           |
 *                     |           | 280   pthread structure
 *                     |           |
 *                     v           v
 * tsd -----------------------------------------------------------------
 *                     ^
 *                     | 1024
 *                     v
 * map + size ----------------------------------------------------------
 */

static const size_t __uk_tsd_size = sizeof(void *) * PTHREAD_KEYS_MAX;

void *__uk_copy_tls(unsigned char *mem)
{
	pthread_t td;
	void *tls_area;

	mem -= (uintptr_t)mem & (libc.tls_align-1);
	tls_area = mem;
	ukarch_tls_area_init(tls_area);

	td = (pthread_t) ukarch_tls_tcb_get(ukarch_tls_tlsp(tls_area));
	td->dtv = tls_area;

	return td;
}

/*
 * This function is only called for the main thread
 */
static int __uk_init_tp(void *p)
{
	pthread_t td = p;

	/*
	 * Clean and initialize pthread structure for init thread.
	 * FIXME: Some fields from pthread are not initialized yet:
	 * - information about stack
	 * - canary
	 */
	memset(td, 0, sizeof(*td));

	td->tsd = (void *)uk_memalign(uk_alloc_get_default(), __PAGE_SIZE,
							__uk_tsd_size);
	if (!td->tsd)
		UK_CRASH("Failed to initialize init thread tsd\n");

	memset(td->tsd, 0, __uk_tsd_size);

	/*
	 * The initial thread in the new image shall be joinable, as if
	 * created with the detachstate attribute set to
	 * PTHREAD_CREATE_JOINABLE.
	 */
	td->detach_state = DT_JOINABLE;

	/* Musl maintains a circular doubly linked list for threads. */
	td->self = td->next = td->prev = td;
	/*
	 * Set the `$fs` register for the current thread.
	 * In the original code of musl this will use an `arch_prtcl`
	 * syscall to fill the `$fs` register.
	 */
	ukplat_tlsp_set((unsigned long) TP_ADJ(p));
	libc.can_do_threads = 1;
	/*
	 * The original musl code will invoke here a `SYS_set_tid_address`
	 * syscall, to set the tid user space address in the Kernel.
	 * FIXME: Currently this can fail in two ways. The first way is a
	 * crash, which we avoid by checking if the current thread is NULL.
	 * The reason why we introduced this check is that the scheduler is not
	 * initialized when this function gets executed.
	 *`uk_syscall_r_set_tid_address()` will call `uk_syscall_r_gettid()`
	 * which may use the current uk_thread. If the uk_thread is NULL then
	 * it will crash when trying to access an invalid address.
	 * The second way it can fail is by returning an invalid tid. The call
	 * can return an error code (-95, -ENOTSUP) because posix_process_init
	 * has not been called at this stage, but will be called via uk_late_initcall.
	 * It is not a really big problem right now. Since this is the main thread,
	 * nobody should ever wait for it, and we can just assume thread id 0.
	 * The workaround for the moment is to set the tid to 0 whenever an error
	 * might happen.
	 */
	if (uk_thread_current()) {
		td->tid = (int)uk_syscall_r_set_tid_address((long)&td->tid);
		if (td->tid < 0)
			td->tid = 0;
	} else {
		td->tid = 0;
	}
	td->locale = &libc.global_locale;
	td->robust_list.head = &td->robust_list.head;
	return 0;
}

/*
 * This fucntion is only called for the main thread.
 */
static void __uk_init_tls(void *tls_area)
{
	libc.tls_size = ukarch_tls_area_size();
	libc.tls_align = ukarch_tls_area_align();

	/* Failure to initialize thread pointer is always fatal. */
	if (__uk_init_tp(tls_area) < 0)
		UK_CRASH("Failed to initialize the main thread\n");
}

/* We initialize the auxiliary vector to point to a zero value */
static size_t __libc_auxv[1] = { 0 };

static void __uk_init_libc(void)
{
	libc.auxv = __libc_auxv;
	__hwcap = 0;
	__sysinfo = 0;
	libc.page_size = __PAGE_SIZE;
}

/*
 * This callback will only be called for threads that are NOT
 * created with the pthread API but we still want them to be
 * compatible with Musl.
 */
int uk_thread_uktcb_init(struct uk_thread *thread, void *tcb)
{
	struct pthread *self = pthread_self();
	struct pthread *td = (struct pthread *) tcb;

	uk_pr_debug("%s uk_thread %p, tcb %p\n", __func__, thread, tcb);

	td->stack = thread->_mem.stack;
	td->stack_size = __STACK_SIZE;
	td->self = td;
	td->tsd = (void *)uk_memalign(
		uk_alloc_get_default(),
		__PAGE_SIZE,
		__uk_tsd_size);
	/* musl expects that the tsd area is zero-initialized and will not
	 * zero the pointers on pthread_key_create.
	 */
	memset(td->tsd, 0, __uk_tsd_size);
	td->locale = &libc.global_locale;
	td->next = self->next;
	td->prev = self;
	td->next->prev = td;
	td->prev->next = td;

	return 0;
}

/* This callback will only be called for threads that are NOT
 * created with the pthread API
 */
void uk_thread_uktcb_fini(struct uk_thread *thread, void *tcb)
{
	struct pthread *td = (struct pthread *) tcb;

	uk_pr_debug("%s uk_thread %p, tcb %p\n", __func__, thread, tcb);
	uk_free(uk_alloc_get_default(), td->tsd);
}

/*
 * This callback is called for every thread, but we only use it
 * to initialize the `libc` global variable and the main thread.
 */
void ukarch_tls_tcb_init(void *tcb)
{
	uk_pr_debug("%s tcb %p\n", __func__, tcb);

	/*
	 * The first thread (main) will fill the `libc` global variable.
	 * This includes `tls_size`, `tls_align`. After this the
	 * `__uk_init_tls()` function is called which will initialise
	 * the `struct pthread` structure for the main thread.
	 * After this sequence of calls `libc.can_do_threads` is set to 1.
	 */
	if (!libc.can_do_threads) {
		__uk_init_libc();
		__uk_init_tls(tcb);
	}
}
