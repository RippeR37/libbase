WinAPI integration module example
====================================

Below you can find source code of a ``winapi_integration`` example that
showcases usage of the ``libbase`` integration module for ``WinAPI``.
This example is located in the ``/examples/winapi_integration/`` directory in
the repository and - if :ref:`enabled <configuration-libbase-build-examples>` -
will be built along the ``libbase`` library itself (on Windows).

.. literalinclude:: ../../examples/winapi_integration/CMakeLists.txt
   :language: cmake
   :caption: CMakeLists.txt
   :linenos:

.. literalinclude:: ../../examples/winapi_integration/main.cc
   :language: cpp
   :caption: main.cc
   :linenos:
