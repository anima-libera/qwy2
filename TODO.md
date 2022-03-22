
# Qwy2 TODO list

## Build system

- **Optimization:** Save last successful link date to avoid unnecessary links.
- **Optimization:** Save last successful build date for each translation unit to avoid rebuilding those that already passed.
- **Optimization:** Scanning the header and source files for `#include` directives and obtain a file dependency graph to use to reduce the set of effectively changed source files when some header files are changed.
- **Optimization:** Support [precompiled header files](https://gcc.gnu.org/onlinedocs/gcc/Precompiled-Headers.html).
- **Optimization:** Support running multiple instances of the compiler at once via nonblocking calls to make use of more cores (up to some input number of cores).
- **Refactoring:** Split it into multiple scripts.
- **Refactoring:** Factorize the code that handles the permanent build data sync for some arbitrary Python object.
- **Feature:** Support Windows (using something like [MinGW](https://www.mingw-w64.org) and maybe other compilers too).
- **Feature:** Ask for auto install missing dependencies.

## Qwy2

- **Feature:** Allow placing and breaking blocks.
- **Feature:** When in first-person view, select a block via a raycast.
- **Feature:** Display some sort of star in the sun's direction.
