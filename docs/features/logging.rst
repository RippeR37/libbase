Logging
=======

About
-----

The ``libbase`` library comes preintegrated with a
`GLOG <https://github.com/google/glog>`_ library that allows you to add logs and
assertions to your code in the same format as possible within Chromium's code.

You can read more on how to use GLOG
`here <https://github.com/google/glog/blob/v0.7.1/README.rst#user-guide>`_ or
see the same user-guide (from the version ``v0.7.1``) included below.

.. note::

   To use logging system, simply include ``base/logging.h`` file.

.. attention::

   The user-guide included below is created and maintained by the GLOG_
   authors. It (``README.rst`` file) is simply included here from the original
   repo for ease-of-use. See GLOG's
   `LICENSE <https://github.com/google/glog/blob/master/COPYING>`_ file for
   its license details.


GLOG's user guide
-----------------

.. include:: ../../docs/build/third_party/glog/README.rst
   :start-after: User Guide
   :end-before: How to Contribute
