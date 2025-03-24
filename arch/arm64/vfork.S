/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2025, Unikraft GmbH and The Unikraft Authors.
 * Licensed under the BSD-3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 */

/*
 * NOTE: This code here is replicated from the core Unikraft repository,
 * since we can't use musl's vfork wrapper.
 */
.global vfork
vfork:
	b	uk_syscall_e_vfork
