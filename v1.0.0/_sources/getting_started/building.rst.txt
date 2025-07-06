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

   $ git clone --recurse-submodules https://github.com/RippeR37/libbase.git

You can pass an additional ``--depth <depth>`` argument to limit the repository
history to be downloaded (e.g. ``--depth 1``).

.. caution::

   Note that manually downloaded sources from the
   `GitHub repository <https://github.com/RippeR37/libbase>`_ do **not**
   include the source code of any third-party libraries added as a submodule
   which will prevent you from successfully building or using the library!

.. tip::

   If you're having problems with building the library, verify that all
   submodules are correctly initialized.


Building with CMake
-------------------

.. code-block:: console

   $ cd libbase
   $ cmake -S . -B build
   $ cmake --build build

This will configure ``libbase`` with the default
:ref:`configuration options <configuration>` and build it in the ``build/``
subdirectory.


Running the tests
-----------------

If you've built the library with unit tests (see :doc:`configuration`), you can
run them all with:

.. code-block:: console

   $ ctest --test-dir build


Configuration
-------------

You can configure ``libbase`` build or library itself by overriding default
options defined in CMake scripts. You can do this by specifying the
``-DOPTION_NAME=[ON/OFF]`` switch for a given option during CMake configuration
phase. For example, to build ``libbase`` with the documentation, you would have
to execute:

.. code-block:: console

   $ cmake -DLIBBASE_BUILD_DOCS=ON -S . -B build
   $ cmake --build build

.. seealso::

   Check out the :doc:`configuration` page to see what are the available options
   and their description.

