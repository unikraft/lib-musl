/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Felipe Huici <felipe.huici@neclab.eu>
 *
 * Copyright (c) 2017, NEC Europe Ltd., NEC Corporation. All rights reserved.
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
 *
 * THIS HEADER MAY NOT BE EXTRACTED OR MODIFIED IN ANY WAY.
 */

#include "syscall.h"
#include <errno.h>
#include <stdarg.h>
#include <uk/print.h>
#include <bits/ioctl.h>

long __syscall0(long n)
{
  return __syscall(n);
}

long __syscall1(long n, long a1)
{
  return __syscall(n, a1);
}

long __syscall2(long n, long a1, long a2)
{
  return __syscall(n, a1, a2);
}

long __syscall3(long n, long a1, long a2, long a3)
{
  return __syscall(n, a1, a2, a3);
}

long __syscall4(long n, long a1, long a2, long a3, long a4)
{
  return __syscall(n, a1, a2, a3, a4);  
}

long __syscall5(long n, long a1, long a2, long a3, long a4, long a5)
{
  return __syscall(n, a1, a2, a3, a4, a5);  
}

long __syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6)
{
  return __syscall(n, a1, a2, a3, a4, a5, a6);  
}


long __do_syscall(long n, va_list args)
{
  switch(n)
  {
    case SYS_ioctl:
    {
      int fd = va_arg(args, int);      
      unsigned long request = va_arg(args, unsigned long);
	  
      switch(request)
      {
        case TIOCGWINSZ:
        {
          struct winsize *ws = va_arg(args, struct winsize *);
	  /* Set some reasonable values */
	  ws->ws_row = 200;
	  ws->ws_col = 80;	  
	  return 0;
        }
        default:
        {
	  uk_printd(DLVL_EXTRA, "unknown or unimplemented ioctl! %ld\n", request);
          return -1;
        }
      }

      
    case SYS_writev:
    {
      int fd = va_arg(args, int);      
      struct iovec *iov = va_arg(args, struct iovec *);
      int iovcnt = va_arg(args, int);
      return uk_syscall_writev(fd, iov, iovcnt);
    }
      
    default:
    {
      uk_printd(DLVL_EXTRA, "unknown or unimplemented syscall! %ld\n", n);
      return -1;
    }
  }
  
  //  errno = ENOSYS;
  return -1;
  }
}

long syscall(long n, ...)
{
  long ret;
  va_list args;
  va_start(args, n);
  ret = __do_syscall(n, args);
  va_end(args);

  return ret;
}

long __syscall(long n, ...)
{
  long ret;
  va_list args;
  va_start(args, n);
  ret = __do_syscall(n, args);
  va_end(args);

  return ret;
}
