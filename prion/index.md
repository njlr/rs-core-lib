# Prion Library #

#### Minimal common utilities by Ross Smith ####

* [`"prion/core.hpp"`](core.html) - Core utilities.
* [`"prion/unit-test.hpp"`](unit-test.html) - Unit testing utilities.

Prion is a header-only library that basically represents the absolute minimum
set of common utilities that I've found a common need for in other projects,
without having to duplicate the code in all of them.

Everything in this library is in `namespace Prion`, and can be used either by
explicit namespace qualification or via `using namespace Prion`. In practise,
you should never use the `prion/core` module or its namespace explicitly; it
will be included automatically in any other library that needs it.

Since Prion is header-only, it is not necessary to build Prion before using it
with any other library. The makefile included here exists mainly to be
referenced by inclusion in other projects' makefiles; it can also be used to
run Prion's own unit tests.

Prion is written in C++14; you will need an up-to-date C++ compiler, probably
a recent version of Clang or GCC. I developed it mainly on macOS, currently
using Clang 7.0, as well as testing with GCC 5.x. All test builds are made
using strict compilation mode (`-Wall -Wextra -Werror`). It will not work with
any existing release of Microsoft Visual C++.

Because I'm primarily targeting a shortlist of compilers rather than a
specific edition of the C++ standard, the code makes a few assumptions that
are not guaranteed by the standard but are reliably true on all the system I
care about. These include:

* `char` is always 8 bits.
* 128-bit integers are available as `__int128`.
* `float` and `double` are IEEE 32 and 64 bit types.
* Symbols from the C library are always in the global namespace.

On Microsoft Windows builds, including Prion will bring in the whole of
`<windows.h>`. This is annoying but unfortunately unavoidable for a library
that needs to make calls to system APIs while remaining header-only. (Under
Cygwin, this will only happen if `_WIN32` is defined.)

Rebuilding the documentation (`make doc`) requires Python 3.4+ and the
[Markdown module](https://pypi.python.org/pypi/Markdown). The scripts expect
Python 3 to be callable as `python3`.
