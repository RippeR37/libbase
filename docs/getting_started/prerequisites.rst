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

   * GCC (>= 10)
   * Clang (>= 13)
   * MSVC (>= 2022 19.43)

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

``libbase`` depends on some third-party libraries which must already be
installed in the system or ``vcpkg`` must be used to resolve them:

* `GLOG <https://github.com/google/glog>`_
     Provides convinient logging framework (see :doc:`../features/logging`).


Optional dependencies
---------------------

``libbase`` can optionally use other third-party libraries (also either already
installed or used with ``vcpkg``) or applications for optional tasks. These are:

* `GTest and GMock <https://github.com/google/googletest>`_
     For building and running unit tests.

* `Google Benchmark <https://github.com/google/benchmark>`_
     For building and running performance tests.

* `Doxygen <https://doxygen.nl>`_ and `Sphinx <https://www.sphinx-doc.org>`_
     For building this documentation.
