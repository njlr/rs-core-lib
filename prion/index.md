Title: Prion Library
CSS: style.css

# Prion Library #

#### Minimal common utilities by Ross Smith ####

This is a header-only library that basically represents the absolute minimum
set of common utilities I need for other libraries without having to duplicate
the code in all of them.

Everything in this library is in `namespace Prion`, and can be used either by
explicit namespace qualification or via `using namespace Prion`. In practise,
you should never use the `prion/core` module or its namespace explicitly; it
will be included automatically in any other library that needs it.

Since Prion is header-only, it is not necessary to build Prion before using it
with any other library. The makefile included here exists mainly to be
referenced by inclusion in other projects' makefiles; it can also be used to
run Prion's own unit tests.

* [`"prion/core.hpp"`](core.html) -- Core utilities.
* [`"prion/unit-test.hpp"`](unit-test.html) -- Unit testing utilities.
