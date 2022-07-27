Logging
=======

About
-----

``libbase`` library comes *packaged* with a preintegrated
`GLOG <https://github.com/google/glog>`_ library that allows you to add logs and
assertions to your code in the same format as possible within Chromium's code.

You can read more on how to use GLOG
`here <https://github.com/google/glog#user-guide>`_ or see the same user-guide
(from the version actually used in this build) included below.

.. note::

   To use logging system, simply include ``base/logging.h`` file.

.. attention::

   The user-guide included below is created and maintained by the GLOG_
   authors. It is simply included here from the in-source ``README.rst`` file
   for ease-of-use. See GLOG's
   `LICENSE <https://github.com/google/glog/blob/master/COPYING>`_ file for
   its license details.


GLOG's user guide
-----------------

.. include:: ../../third_party/glog/README.rst
   :start-after: User Guide
   :end-before: How to Contribute
