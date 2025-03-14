Using in your project
=====================

There are many ways for adding ``libbase`` to your project. It is up to you to
decide which one you prefer. Below you will find a recommended (and probably the
easiest) way to do so.


Recommended way (CMake)
-----------------------

This will be a step-by-step guide to create a new project that uses ``libbase``
library using a Git and CMake. If you wish to add ``libbase`` to your existing
project, please skip :ref:`here <skip_existing_project>`.

#. Create a new Git repository and add initial files.

   .. code-block:: console

      $ git init
      $ git add [...]
      $ git commit -m "Initial commit"

#. Create a new CMake script and a simple C++ source file.

   .. code-block:: cmake
      :caption: CMakeLists.txt

      cmake_minimum_required(VERSION 3.13)
      project(project-name VERSION 1.0 LANGUAGES CXX)

      add_executable(project-name "")

      target_sources(project-name
        PRIVATE
          src/main.cc
      )

   .. code-block:: cpp
      :caption: src/main.cc

      #include <iostream>

      int main() {
        std::cout << "Hello World!" << std::endl;
        return 0;
      }

   .. _skip_existing_project:

#. Add ``libbase`` library as a Git submodule and initialize it

   .. code-block:: console

      $ git submodule add https://github.com/RippeR37/libbase third_party/libbase
      $ git submodule update --init --recursive

   .. note::

      It is not required to fetch/store ``libbase`` as a submodule. It is,
      however, a recommended way.

#. Add ``libbase`` subdirectory to your CMake script.

   .. code-block:: cmake
      :caption: CMakeLists.txt
      :emphasize-lines: 4,8,9

      cmake_minimum_required(VERSION 3.13)
      project(project-name VERSION 1.0 LANGUAGES CXX)

      add_subdirectory(third_party/libbase)

      add_executable(project-name "")

      target_compile_options(project-name PRIVATE ${LIBBASE_COMPILE_FLAGS})
      target_link_libraries(project-name PRIVATE libbase)
      target_sources(project-name
        PRIVATE
          src/main.cc
      )

#. Use ``libbase`` library in your project.

   .. code-block:: cpp
      :caption: src/main.cc
      :emphasize-lines: 3,6

      #include <iostream>

      #include "base/callback.h"

      int main() {
        base::BindOnce([]() { std::cout << "Hello World!" << std::endl; }).Run();
        return 0;
      }

#. Compile, build and run!

   .. code-block:: console

      $ cmake -S . -b build
      $ cmake --build build
      $ ./build/project-name
      Hello World!

.. tip::

   Repository with the above project can also be viewed here:
   `RippeR37/libbase-example-cmake <https://github.com/RippeR37/libbase-example-cmake>`_.


Other build systems
-------------------

.. todo::

   Unfortunately, ``libbase`` library at this time comes only with a
   preconfigured way of integrating it within another CMake project. Other build
   systems are not yet supported out-of-the-box.

.. note::

   If you manage to integrate ``libbase`` with a different build system, feel
   free to make a pull request with any necessary changes.
