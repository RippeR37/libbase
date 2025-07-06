Introduction
============

About ``libbase``
-----------------

The ``libbase`` is a small library that provides its users with a
reimplementation of many useful low-level utilities known from Chromium's
``//base`` module without the need to depend on the whole (or parts of the)
Chromium itself.

The library is written in C++17 and uses CMake as a build system. The source
code is formatted in Chromium style and is consistent with the
`Google C++ Style Guide <https://google.github.io/styleguide/cppguide.html>`_.
The library also pre-integrates Google's logging library:
:doc:`GLOG <../features/logging>` known from Chromium's source code.


Goals
-----

* Ease of use.
* Provide most useful programming utilities (e.g. callbacks, threads,
  task runners, etc.).
* Provide familiar application developing environment for people working with
  Chromium.

   * Try to match ``//base`` module APIs as close as possible.
   * Try to match ``//base`` module file hierarchy.

* Require as few external dependencies as possible.
* Reduce usage of custom-built components if there are viable alternatives
  already in the C++ standard library.


Non-goals
---------

* Provide any interoperability with an actual ``//base`` module, such as:

  * Any ABI compatibility,
  * Full API compatibility.

* Reimplement *everything* from the ``//base`` module.
* Match or surpass ``//base`` in terms of performance or compatibility.

.. note::

   While the performance and memory usage are very important and ``libbase`` is
   being designed with these things in mind and constantly being optimized, the
   ``//base`` module will simply be more mature and probably better optimized
   for a long time anyway.


Differences from ``//base``
---------------------------

* Some constructs are replaced altogether with C++ STL alternatives (e.g.
  ``std::shared_ptr<T>`` is used instead of ``scoped_refptr<T>``) and will NOT
  be reimplemented at all.
* Unless absolutely needed, ``libbase`` will use STL constructs over Abseil's.
* Copies of the same ``base::RepeatingCallback`` objects do not share the
  bounded state.
