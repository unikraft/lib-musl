# Musl

[Musl](https://musl.libc.org/) is a standard C library (libc) providing POSIX support on top of the Linux system call API.
These are support files for Musl support in Unikraft.

Musl is the default and recommended libc for Unikraft.
[`newlib`](https://github.com/unikraft/lib-newlib) is another libc available for Unikraft.
Musl provides thread support natively, while `newlib` requires [`pthread-embedded`](https://github.com/unikraft/lib-pthread-embedded/);
Musl doesn't require (and must not be built with) `pthread-embedded`.

Note that [`lwip`](https://github.com/unikraft/lib-lwip) is required for networking support.

## Using Musl with Unikraft

Musl is to be used as the standard C library in Unikraft applications, such as [`app-nginx`](https://github.com/unikraft/app-nginx).
The application build system must be aware of the location of the local clone of the Musl repository.
When Musl is selected the [internal Unikraft `nolibc` library](https://github.com/unikraft/unikraft/tree/staging/lib/nolibc) will be unselected.

## Remarks

The default configuration doesn't include complex number support, i.e. the `LIBMUSL_COMPLEX` option is disabled (see `Config.uk`);
see [commit `45c4aa58`](https://github.com/unikraft/lib-musl/commit/45c4aa586fa0a215ebd1bc2950f5fe249d1b965c).
This is because, in case of using Clang to build Musl, the symbol `_muldc3` (used for complex numbers) is undefined;
in case of using Clang and requiring complex number support, [`compiler-rt`](https://github.com/unikraft/lib-compiler-rt/) is to be included to the build.
