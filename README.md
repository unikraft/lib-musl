musl for Unikraft
===================

This first release provides a functional version of musl (compiles and
links) on Unikraft but some functions are not supported (especially
the ones related to threads and network). Furthermore, some functions
can have unexpected behaviour since they use musl internal thread
support.

## Build

In order to avoid multiple definitions errors, the following components
MUST NOT BE INCLUDED with musl:

* `lwip->socket`
* `pthread-embedded`

## Known Issues

* At this stage, we noticed that there are some issues with buffering
on stdout. We are currently investigating this issue but a temporary
fix is to call `fflush()` after each output on the console.

As things stabilize, we will update this file to reflect this.

## Further information

Please refer to the `README.md` as well as the documentation in the `doc/`
subdirectory of the main unikraft repository.
