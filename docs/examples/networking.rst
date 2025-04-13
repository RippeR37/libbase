Networking example
==================

Below you can find source code of a ``networking`` example that showcases usage
of the ``libbase`` functionality for performing network requests. This example
is located in the ``/examples/networking/`` directory in the repository and - if
:ref:`enabled <configuration-libbase-build-examples>` - will be built along the
``libbase`` library itself.

.. literalinclude:: ../../examples/networking/CMakeLists.txt
   :language: cmake
   :caption: CMakeLists.txt
   :linenos:

.. literalinclude:: ../../examples/networking/main.cc
   :language: cpp
   :caption: main.cc
   :linenos:
