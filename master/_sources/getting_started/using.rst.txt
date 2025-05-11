Using in your project
=====================

There are many ways to add ``libbase`` to your project. It is up to you to
decide which one suits your needs the best. Below you will find a recommended
(and probably the easiest) way to do so.


Recommended way (CMake + vcpkg)
-------------------------------

This is a step-by-step guide to create a new project that uses ``libbase``
library using Git, CMake and ``vcpkg``. If you wish to add ``libbase`` to your
existing project, please skip :ref:`here <skip_existing_project>`.

#. Create a new Git repository and add initial files.

   .. code-block:: console

      $ git init
      $ git add [...]
      $ git commit -m "Initial commit"

#. Create a new CMake script and a simple C++ source file.

   .. code-block:: cmake
      :caption: CMakeLists.txt
      :linenos:

      cmake_minimum_required(VERSION 3.13)
      project(project-name VERSION 1.0 LANGUAGES CXX)

      add_executable(project-name "")

      target_sources(project-name
        PRIVATE
          src/main.cc
      )

   .. code-block:: cpp
      :caption: src/main.cc
      :linenos:

      #include <iostream>

      int main() {
        std::cout << "Hello World!" << std::endl;
        return 0;
      }

   .. _skip_existing_project:

#. Initialize ``vcpkg`` and add ``libbase`` library as a dependency

   .. important::

      To avoid name clashes with other libraries, the ``vcpkg`` port for this
      library is called ``ripper37-libbase``. This is the name you should use
      when adding it to your project.

   .. code-block:: console

      $ vcpkg new --application
      $ vcpkg add port ripper37-libbase

   .. note::

      You can customize which parts of ``libbase`` you want to use by specifying
      which features you want to enable. To do that, you need to modify the
      dependency entry in the ``vcpkg.json`` file to look like:

      .. code-block:: json
         :caption: vcpkg.json

         {
           "dependencies": [
             // ...
             {
               "name": "ripper37-libbase",
               "default-features": false,
               "features": [
                 // list features that you need here
               ]
             },
             // ...
           ]
         }

      Currently available features are:

      - ``net`` - enables networking module (enabled by default)
      - ``win`` - enables WinAPI integration module
      - ``wx`` - enables wxWidgets integration module

#. Add ``libbase`` dependency and link with it in your CMake script.

   .. code-block:: cmake
      :caption: CMakeLists.txt
      :linenos:
      :emphasize-lines: 4,7

      cmake_minimum_required(VERSION 3.13)
      project(project-name VERSION 1.0 LANGUAGES CXX)

      find_package(libbase CONFIG REQUIRED)

      add_executable(project-name "")
      target_link_libraries(project-name PRIVATE libbase::libbase)
      target_sources(project-name
        PRIVATE
          src/main.cc
      )

   If you want to use optional modules, you need to add corresponding component
   names to the ``find_package()`` function call and link with their targets in
   ``target_link_libraries()``.

   .. list-table:: Available components and their target names
      :widths: 50 25 25
      :header-rows: 1

      * - Module
        - Component
        - Target
      * - Networking module
        - ``net``
        - ``libbase::libbase_net``
      * - WinAPI integration module
        - ``win``
        - ``libbase::libbase_win``
      * - wxWidgets integration module
        - ``wx``
        - ``libbase::libbase_wx``

#. Use ``libbase`` library in your project.

   .. code-block:: cpp
      :caption: src/main.cc
      :linenos:
      :emphasize-lines: 3,6

      #include <iostream>

      #include "base/callback.h"

      int main() {
        base::BindOnce([]() { std::cout << "Hello World!" << std::endl; }).Run();
        return 0;
      }

#. Compile, build and run!

   .. code-block:: console

      $ export VCPKG_ROOT=/path/to/vcpkg
      $ cmake -S . -b build
      $ cmake --build build
      $ ./build/project-name
      Hello World!

.. tip::

   More advanced example project using this method can be viewed here:
   `RippeR37/libbase-example-vcpkg <https://github.com/RippeR37/libbase-example-vcpkg>`_.


Alternative ways
----------------

If you don't want to use ``vcpkg`` to resolve dependencies, you can replace step
3 and not export ``VCPKG_ROOT`` environment variable and use one of these
methods instead:

- Manually build and install ``libbase`` and all of its dependencies in your
  system. For more details on how to do that, please refer to the
  :doc:`building <building>` page.

- Use CMake's ``FetchContent`` module to download and build ``libbase`` as part
  of your project. To do this, add below snippet to your CMake script:

  .. code-block:: cmake
     :caption: CMakeLists.txt

     include(FetchContent)
     FetchContent_Declare(
         libbase
         GIT_REPOSITORY https://github.com/ripper37/libbase.git
         GIT_TAG        <commit_or_tag_to_fetch>
     )
     FetchContent_MakeAvailable(libbase)

  .. caution::

     This doesn't auto-resolve ``libbase`` required dependencies by itself. You
     will still need to build and install them manually or use ``FetchContent``
     to declare and make them available in your CMake project before including
     the ``libbase`` library.

   .. tip::

      Simple example project using this method can be viewed here:
      `RippeR37/libbase-example-fetchcontent <https://github.com/RippeR37/libbase-example-fetchcontent>`_.

- Use CMake's ``add_subdirectory()`` to add ``libbase`` to your project whil
  will work similarly to the above ``FetchContent`` method. To get ``libbase``
  source files you can download them or add ``libbase`` as a Git submodule.

  .. code-block:: console
     :caption: Terminal

     $ git submodule add https://github.com/RippeR37/libbase
     $ git submodule update --init

  .. code-block:: cmake
     :caption: CMakeLists.txt

     add_subdirectory(libbase)

  .. caution::

     Similarly to the ``FetchContent`` method, this also doesn't resolve
     ``libbase`` dependencies by itself. Furthermore, this type of dependency
     management is not recommended by many - please consider alternatives before
     choosing it.

   .. tip::

      Simple example project using this method can be viewed here:
      `RippeR37/libbase-example-submodules <https://github.com/RippeR37/libbase-example-submodules>`_.
