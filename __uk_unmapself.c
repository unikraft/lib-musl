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

#include <sys/types.h>
#include <uk/print.h>
#include <uk/alloc.h>
#include <uk/sched.h>

/*
 * This glue code source is meant to replace the `__unmapself()`
 * call in musl. When a thread is detached it must clear its mapped
 * regions (e.g. stack). This is not a problem when running on Linux,
 * because there is a stack in the kernel. For Unikraft we use a
 * single stack. `__unmapself()' is an architecture-dependent function,
 * but there is also a generic version in `src/thread/__unmapself.c'
 * that uses a shared stack on which the code is run. However, we also
 * have our version in which we deal with this special case in our own
 * way. `__uk_unmapself()' will call a special version of `exit()'
 * exposed by the sched library through which we can register a callback.
 * This callback will be executed by another thread and will call `unmap()`.
 */


struct __map_area {
	void *map_base;
	size_t map_size;
};

int __munmap(void *map, size_t size);

/*
 * This will be called in the context of the `idle` thread
 */
static void __uk_unmapself_cb(struct uk_thread *t, void *mapped_area)
{
	uk_pr_debug("unmapself's callback is called\n");
	struct __map_area *m = (struct __map_area *) mapped_area;

	__munmap(m->map_base, m->map_size);
}

static __uk_tls struct __map_area __per_thread_map_area = {};

void __uk_unmapself(void *base, size_t size)
{
	/*
	 * In order not to allocate memory (e.g. with malloc()) that could fail,
	 * we will use a variable in TLS to store the map address and size.
	 * When the callback will be run from the context of the `idle` thread,
	 * the TLS of this thread will be accessed, after which everything
	 * will be deallocated.
	 */
	__per_thread_map_area.map_base = base;
	__per_thread_map_area.map_size = size;

	uk_sched_thread_exit2(__uk_unmapself_cb, &__per_thread_map_area);
}
