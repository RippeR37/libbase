Prerequisites
=============

Hard requirements
-----------------

``libbase`` aims to require as little as possible from its users. The only
hard requirements are:

* C++17-supporting compiler
* CMake (at least v3.13)

.. note::

   Project's continuous integration workflows test library on all major
   compilers:

   * GCC (>= 7)
   * Clang (>= 9)
   * MSVC (>= 2019.29)

   on all major OSes:

   * Linux
   * Windows
   * MacOS

.. seealso::

   Check out the
   `GitHub Workflows <https://github.com/RippeR37/libbase/actions>`_ page to
   see currently tested configurations.


Internal dependencies
---------------------

``libbase`` depends on some third-party libraries which are already integrated
into the project and thus require **no** additional work to use it:

* `GLOG <https://github.com/google/glog>`_ - provides convinient logging
  framework (see :doc:`../features/logging`).


Optional dependencies
---------------------

``libbase`` can optionally use other external libraries (also already
preconfigured and integrated into the project) for optional tasks. These are:

* `GTest and GMock <https://github.com/google/googletest>`_ - for unit tests.
* `Google Benchmark <https://github.com/google/benchmark>`_ - for performance
  tests.
