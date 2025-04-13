Networking
==========

Introduction
------------

While not part of Chromium's ``//base`` itself, realizing that applications
built today almost always require access to network, ``libbase`` provides users
with simple yet quite powerful networking utilities built on top of ``libcurl``
and fully integrated with task runners/sequences. The API is loosely based on
Chromium's ``//net`` module.


Usage
~~~~~

While Networking module is enabled by default, it **is** optional so before
starting make sure that ``libbase`` build you're using was built with it (the
``LIBBASE_BUILD_MODULE_NET`` build option enabled).

Once built, be sure to link your application with ``libbase_net`` module target.
All code in networking module is located in ``base/net/`` directory and all
classes in that module are in ``base::net`` namespace.

Lastly, before using documented below functionality, be sure to initialize and
deinitialize networking module in runtime (on application's startup and
shutdown, respectively) with calls to these functions:

* :func:`base::net::Initialize`
* :func:`base::net::Deinitialize`


:class:`base::net::SimpleUrlLoader`
-----------------------------------

This class provides two functions that you can use to perform a simple network
request:

* :func:`base::net::SimpleUrlLoader::DownloadUnbounded`
* :func:`base::net::SimpleUrlLoader::DownloadLimited`

First variant performs a network request for a resource specified by passed
:class:`base::net::ResourceRequest` struct as parameter. For HTTP(s) request,
this may be a ``GET``, ``POST`` or ``HEAD`` request and once finished, it will
execute provided ``on_done_callback`` with response
(:class:`base::net::ResourceResponse`). The second variant allows you to specify
maximum size of a response and if there would be more data, it will abort the
request.

.. warning::

   Downloading unknown resource without size limitations may lead to exhausting
   all of memory available for your application. It is recommended to either
   know the response size before making an ``Unbounded`` request, or using the
   ``Limited`` version of the function.

   You can also use more advanced utility :class:`base::net::UrlRequest`
   described below where you will have much greater control over the process and
   e.g. will be able to stop in the middle of fetching response.

Both of these functions take optional argument of
``std::shared_ptr<base::TaskRunner>`` type. If provided, your callback will be
executed on that task runner, otherwise provided callback will be executed on
the task runner on which the request (call to ``Download*()`` function) was
made.

They also return an instance of :class:`base::net::RequestCancellationToken`
class which can be passed to :func:`base::net::SimpleUrlLoader::CancelRequest`
function to **asynchronously** cancel related request.

.. warning::

   Request cancellation is an asynchronous process. Calling
   :func:`base::net::SimpleUrlLoader::CancelRequest` function does **not**
   guarantee that provided ``on_done_callback`` will not be executed. If you
   need to guarantee that - you need to add your own logic to check for that -
   for example by binding the callback to a :class:`base::WeakPtr` that you can
   invalidate when needed.

.. admonition:: Example
   :class: admonition-example-code

   .. code-block:: cpp

      void PerformRequest(std::string resource_url) {
        base::net::SimpleUrlLoader::DownloadUnbounded(
            base::net::ResourceRequest{resource_url},
            base::BindOnce(&OnResponse));
      }
      void OnResponse(base::net::ResourceResponse response) {
        // Check response status and other metadata
      }

For a full example of :class:`base::net::SimpleUrlLoader` usage, refer to
:doc:`../examples/networking`.


:class:`base::net::UrlRequest`
------------------------------

This class provides similar functionality to :class:`base::net::SimpleUrlLoader`
but provides much more control over the process. Instead of calling a function
and passing a callback that will be executed when the request finishes and
provide all metadata and data from the response, this class is used by creating
an instance of it and storing it for the duration of the request during which it
will perform calls to specified *client*.

To create it, you need to pass a pointer to an instance of
:class:`base::net::UrlRequest::Client` interface that will receive calls
marking of a progress on the request such as:

* :func:`base::net::UrlRequest::Client::OnResponseStarted`
    This method will be called with metadata (such as request status code,
    final URL - if there were any redirects, and received headers) when the
    connection will be established and those metadata received.

    .. caution::

       If the request fails before connection was established, this method will
       **not** be called therefore users must **not** design logic to depend on
       it being called at least once.

* :func:`base::net::UrlRequest::Client::OnWriteData`
   This method will be called every time new chunk of data will be received by
   ``libbase`` network stack. User is expected to process or buffer this data
   on their own as it will not be buffered and sent all-in-one call later.

   .. caution::

      Same as with the previous method - if the request fails before any
      response data (body) will be received, this function may not be called
      even once. Furthermore, if the request was made with ``headers_only`` set
      to ``true``, this method will not be called even for a successful
      requests.

      Users however can depend on it never being called before/without a call to
      :func:`base::net::UrlRequest::Client::OnResponseStarted` method.

* :func:`base::net::UrlRequest::Client::OnRequestFinished`
   This member function will be called when processing of that network request
   will finish. Users should inspect returned :class:`base::net::Result`
   parameter to know if the request has finished or not.

   .. note::

      This callback is guaranteed to be called exactly once when the request
      will finish, unless the :class:`base::net::UrlRequest` was cancelled
      (either by calling the :func:`base::net::UrlRequest::Cancel` method or by
      destroying the object itself) in which case the callback will **not** be
      executed.

   .. caution::

      Be aware that unless an explicit timeout was specified when making the
      request, the request can still take arbitrary amount of time until it
      finishes. If the request has to finish within certain amount of time,
      please either provide a timeout or monitor and cancel it manually.

Once :class:`base::net::UrlRequest` is created and you are ready to perform the
request, you should call :func:`base::net::UrlRequest::Start` and provide
details about the network request to be made.

After the request processing has been started, you may call
:func:`base::net::UrlRequest::Cancel` method at any moment or destroy the
underlying :class:`base::net::UrlRequest` object to cancel the request. Once
either of these are done, it is guaranteed that there will be no more calls to
any of the methods on the provided client and the request will be asynchronously
cancelled.

.. warning::

   You can perform only one network request at a time with a single instance of
   :class:`base::net::UrlRequest`.

.. tip::

   To perform multiple request at the same time you can create a separate
   instances of this class for each request you want to start in parallel. You
   can then compare provided pointers to :class:`base::net::UrlRequest` objects
   (which are first arguments in each method in the
   :class:`base::net::UrlRequest::Client` interface) to know for which request
   the calls are made.

For a full example on how to use this class, please refer to the
:doc:`../examples/networking`.
