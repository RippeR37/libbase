Welcome to ``libbase`` library documentation!
=============================================

.. Table of contents

.. toctree::
   :hidden:
   :caption: Introduction

   introduction/introduction.rst

.. toctree::
   :hidden:
   :caption: Getting started
   :maxdepth: 0

   getting_started/prerequisites.rst
   getting_started/building.rst
   getting_started/configuration.rst
   getting_started/using.rst

.. toctree::
   :hidden:
   :caption: Features

   features/callbacks.rst
   features/threads.rst
   features/weak_ptrs.rst
   features/logging.rst
   features/tracing.rst

.. toctree::
   :hidden:
   :caption: Examples

   examples/simple.rst

.. toctree::
   :maxdepth: 2
   :hidden:
   :caption: Reference

   build/api/index
   genindex

.. toctree::
   :hidden:
   :caption: About

   about/license.rst


.. Content

The ``libbase`` [#libbase]_ is a small library that provides its users with a
reimplementation of many useful low-level utilities known from Chromium's
``//base`` module [#chr_base]_ without the need to depend on the whole (or parts
of the) Chromium itself.

.. list-table::
   :align: center
   :widths: auto
   :header-rows: 1

   * - Branch
     - Ubuntu
     - Windows
     - MacOS
   * - `master <https://github.com/RippeR37/libbase>`_
     - |status_m_u|
     - |status_m_w|
     - |status_m_m|
   * - `develop <https://github.com/RippeR37/libbase/tree/develop>`_
     - |status_d_u|
     - |status_d_w|
     - |status_d_m|


.. Footnotes

.. [#libbase] https://github.com/RippeR37/libbase
.. [#chr_base] https://chromium.googlesource.com/chromium/src/base/


.. Badges

.. |status_m_u| image:: https://github.com/RippeR37/libbase/actions/workflows/ubuntu.yml/badge.svg?branch=master
   :target: https://github.com/RippeR37/libbase/actions/workflows/ubuntu.yml?query=branch:master
.. |status_m_w| image:: https://github.com/RippeR37/libbase/actions/workflows/windows.yml/badge.svg?branch=master
   :target: https://github.com/RippeR37/libbase/actions/workflows/windows.yml?query=branch:master
.. |status_m_m| image:: https://github.com/RippeR37/libbase/actions/workflows/macos.yml/badge.svg?branch=master
   :target: https://github.com/RippeR37/libbase/actions/workflows/macos.yml?query=branch:master
.. |status_d_u| image:: https://github.com/RippeR37/libbase/actions/workflows/ubuntu.yml/badge.svg?branch=master
   :target: https://github.com/RippeR37/libbase/actions/workflows/ubuntu.yml?query=branch:develop
.. |status_d_w| image:: https://github.com/RippeR37/libbase/actions/workflows/windows.yml/badge.svg?branch=master
   :target: https://github.com/RippeR37/libbase/actions/workflows/windows.yml?query=branch:develop
.. |status_d_m| image:: https://github.com/RippeR37/libbase/actions/workflows/macos.yml/badge.svg?branch=master
   :target: https://github.com/RippeR37/libbase/actions/workflows/macos.yml?query=branch:develop
