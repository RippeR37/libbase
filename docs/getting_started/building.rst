Building ``libbase``
====================

Prerequisites
-------------

.. attention::

   Ensure you have all prerequisites installed and configured. See
   :doc:`prerequisites` for more details.


Obtaining the source code
-------------------------

The recommended way to obtain the source code is:

.. code-block:: console

   $ git clone https://github.com/RippeR37/libbase.git

You can pass an additional ``--depth <depth>`` argument to limit the repository
history to be downloaded (e.g. ``--depth 1``).


Resolving libbase dependencies
------------------------------

To build ``libbase`` you need to either build and install required dependencies
manually or configure a package manager like ``vcpkg`` to resolve them
automatically.


Building with CMake and pre-installed dependencies
--------------------------------------------------

Assuming all required dependencies are already installed on your system, you can
build ``libbase`` with CMake simply by running:

.. code-block:: console

   $ cmake -S . -B build
   $ cmake --build build [-j <parallel_jobs>] [--config <Release|Debug>]

This will configure ``libbase`` with the default
:ref:`configuration options <configuration>` and build it in the ``build/``
subdirectory.


Building with CMake and vcpkg
-----------------------------

If you don't have the required dependencies installed on your system, the
recommended and simplest way to build ``libbase`` is to use the ``vcpkg``
package manager.

If you already have ``vcpkg`` installed and want to use it to resolve the
dependencies, you can do so by running the following commands:

.. code-block:: console

   $ export VCPKG_ROOT=/path/to/vcpkg
   $ cmake -S . -B build
   $ cmake --build build [-j <parallel_jobs>] [--config <Release|Debug>]

or

.. code-block:: console

   $ cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
   $ cmake --build build [-j <parallel_jobs>] [--config <Release|Debug>]

Alternatively, if you don't have (or don't want to use) pre-existing ``vcpkg``
installation you can ask ``libbase`` to set up its own internal ``vcpkg``
installation just for task of building itself with:

.. code-block:: console

   $ cmake -S . -B build -DLIBBASE_AUTO_VCPKG=1
   $ cmake --build build [-j <parallel_jobs>] [--config <Release|Debug>]


Configuration
-------------

You can configure ``libbase`` build or library itself by overriding default
options defined in CMake scripts. You can do this by specifying the
``-DOPTION_NAME=[ON/OFF]`` switch for a given option during CMake configuration
phase. For example, to build ``libbase`` with the documentation, you would have
to execute:

.. code-block:: console

   $ cmake -S . -B build -DLIBBASE_BUILD_DOCS=ON
   $ cmake --build build

.. seealso::

   Check out the :doc:`configuration` page to see available options and their
   description.


Running the tests
-----------------

If you've built the library with unit tests (see :doc:`configuration`), you can
run them all with:

.. code-block:: console

   $ ctest --test-dir build
