Using in your project
=====================

There are many ways for adding ``libbase`` to your project. It is up to you to
decide which one you prefer. Below you will find a recommended (and probably the
easiest) way to do so.


Recommended way (CMake + vcpkg)
-------------------------------

This will be a step-by-step guide to create a new project that uses ``libbase``
library using Git, CMake and vcpkg. If you wish to add ``libbase`` to your
existing project, please skip :ref:`here <skip_existing_project>`.

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
               "dependencies": [
                 // list features that you need here
               ]
             },
             // ...
           ]
         }


#. Add ``libbase`` dependency and link with it in your CMake script.

   .. code-block:: cmake
      :caption: CMakeLists.txt
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

      $ export VCPKG_ROOT=/path/to/vcpkg
      $ cmake -S . -b build
      $ cmake --build build
      $ ./build/project-name
      Hello World!

.. tip::

   Repository with the above project can also be viewed here:
   `RippeR37/libbase-example-cmake <https://github.com/RippeR37/libbase-example-cmake>`_.
