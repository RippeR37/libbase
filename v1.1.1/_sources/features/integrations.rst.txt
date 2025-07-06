Integrations
============

Introduction
------------

To make it easier for users to use ``libbase`` with other libraries, frameworks
or systems, we're providing (optional) special integration modules that allows
you to integrate ``libbase`` systems (such as task runners/sequences) with
external or native functionality for seamless interaction.


WinAPI
------

This integration is enabled by default when building ``libbase`` on Windows and
can be used by linking with new target ``libbase_win``.

It provides one new class - :class:`base::win::WinMessageLoopAttachment` that
can be used to integrate native Windows/WinAPI message queue with ``libbase``
message loop which in turn will allow you to post tasks to/from WinAPI threads.

.. admonition:: Example
   :class: admonition-example-code

   .. code-block:: cpp

      auto loop_attachement = base::win::WinMessageLoopAttachment::Create();
      if (!loop_attachement) {
        // Creation has failed, some error handling here
      }

      // As long as the `loop_attachement` lives, current thread will have
      // associated task runner that will post task to Window's message
      // queue and all such tasks will be executed between other messages
      // on that queue.

      // Create application window
      HWND hwnd = CreateWindowEx(/* ... */);

      // Main loop example
      MSG msg = {};
      while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }

For a full example on how to use this module, please refer to the
:doc:`../examples/winapi_integration`.


wxWidgets
---------

This integration is enabled by building ``libbase`` with
``LIBBASE_BUILD_MODULE_WX`` build option or installing it through ``vcpkg`` with
optional ``wx`` feature enabled. It's available on all major platforms (Windows,
Linux and MacOS). To use it, make sure to link with target ``libbase_wx``.

It provides one new class - :class:`base::wx::WxMessageLoopAttachment` that
can be used to integrate wxWidgets event system with ``libbase`` message loop
which in turn will allow you to post tasks to/from wxWidgets threads.

.. admonition:: Example
   :class: admonition-example-code

   .. code-block:: cpp

      bool WxApp::OnInit() {
        // After this, application can post tasks to/from this thread as if it
        // were a base::Thread. Tasks will be executed in correct order related
        // to when wx events are posted for this App.
        message_loop_attachment_ =
            std::make_unique<base::wx::WxMessageLoopAttachment>(this);

        // ...
      }

      int WxApp::OnExit() {
        // ...

        message_loop_attachment_.reset();
        return 0;
      }

For a full example on how to use this module, please refer to the
:doc:`../examples/wxwidgets_integration`.
