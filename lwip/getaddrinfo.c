/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2023, Unikraft GmbH and The Unikraft Authors.
 * Licensed under the BSD-3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 */

#include <netdb.h>
#include <lwip/netdb.h>

int getaddrinfo(const char *restrict host, const char *restrict serv,
		const struct addrinfo *restrict hint,
		struct addrinfo **restrict res)
{
	return lwip_getaddrinfo(host, serv, hint, res);
}

void freeaddrinfo(struct addrinfo *ai)
{
	lwip_freeaddrinfo(ai);
}
