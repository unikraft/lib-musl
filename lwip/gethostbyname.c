/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2023, Unikraft GmbH and The Unikraft Authors.
 * Licensed under the BSD-3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 */

#define _GNU_SOURCE /* Enables h_errno and constants */
#include <netdb.h>
#include <lwip/netdb.h>
#include <uk/print.h>
#include <uk/essentials.h>

struct hostent *gethostbyname(const char *name) {
	return lwip_gethostbyname(name);
}

struct hostent *gethostbyname2(const char *name __unused, int af __unused) {
	/* Not supported by lwip, but the function is deprecated anyways
	 * (like the other gethostbyname() variants)
	 */
	UK_WARN_STUBBED();

	h_errno = NO_RECOVERY;
	return NULL;
}

int gethostbyname_r(const char *name, struct hostent *ret, char *buf,
		    size_t buflen, struct hostent **result, int *h_errnop)
{
	return lwip_gethostbyname_r(name, ret, buf, buflen, result, h_errnop);
}

int gethostbyname2_r(const char *name __unused, int af __unused,
		     struct hostent *ret __unused, char *buf __unused,
		     size_t buflen __unused, struct hostent **result __unused,
		     int *h_errnop __unused)
{
	/* Not supported by lwip */
	UK_WARN_STUBBED();

	if (h_errnop)
		*h_errnop = NO_RECOVERY;
	return -1;
}
