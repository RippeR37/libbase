Configuration options
=====================

``libbase`` library can be configured in multiple ways. Some options are
configurable as a CMake options and change what or how it is built, while other
options change the logic of the library or enable/disable some features.

CMake build options
-------------------

CMake options affect what targets are build and how. They have two defaults:

* Internal - used when the ``libbase`` is built as the main project,
* External - used when the ``libbase`` is built as part of another project.

.. _configuration-libbase-build-examples:

.. option:: LIBBASE_BUILD_EXAMPLES=<ON|OFF>

   Build provided examples.

   :Default (internal): ON
   :Default (external): OFF

.. option:: LIBBASE_BUILD_EXAMPLES=<ON|OFF>

   Build provided examples.

   :Default (internal): ON
   :Default (external): OFF

.. option:: LIBBASE_BUILD_TESTS=<ON|OFF>

   Build provided unit tests.

   :Default (internal): ON
   :Default (external): OFF

.. option:: LIBBASE_CODE_COVERAGE=<ON|OFF>

   Build provided unit tests to generate code-coverage data.
   This is useful mostly for CI runners.

   :Default (internal): OFF
   :Default (external): OFF

.. option:: LIBBASE_BUILD_PERFORMANCE_TESTS=<ON|OFF>

   Build provided performance tests.

   :Default (internal): ON
   :Default (external): OFF

.. option:: LIBBASE_BUILD_DOCS=<ON|OFF>

   Build this documentation.

   :Default (internal): OFF
   :Default (external): OFF

.. option:: LIBBASE_CLANG_TIDY=<ON|OFF>

   Build library with clang-tidy.

   :Default (internal): ON
   :Default (external): OFF


Library code options
--------------------

Library code options affect what features are enabled/disabled and how they
work. They may affect runtime behavior of the ``libbase`` constructs.

.. option:: LIBBASE_FEATURE_TRACING=<ON|OFF>

   Build ``libbase`` with the :doc:`../features/tracing` feature enabled.

   :Default: ON

.. option:: (define) LIBBASE_POLICY_LEAK_ON_REPLY_POST_TASK_FAILURE

   When defined, this affects behavior of the
   :func:`base::TaskRunner::PostTaskAndReply` function.

   When undefined, if a reply post-task fails to be scheduled on the original
   task runner, it is destroyed on the posted-to task runner instead of being
   leaked. This is to avoid leaking any resources by default.

   When defined, the behavior is specified to leak the reply callback to avoid
   hitting any sequence checks in the destructors (at the cost of leaking
   resources). This is original behavior of ``//base`` implementation.

   :Default: macro undefined
