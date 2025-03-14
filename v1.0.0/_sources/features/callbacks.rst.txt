Callbacks
=========

Function objects
----------------

The ``libbase`` library provides two implementations of templated functions
objects:

* |OnceCBF|
* |RepeCBF|

These two types represent generic functors that take ``Arguments...``
arguments as an input and return result of type ``Result`` (or nothing, if
``Result`` is ``void``). The main differences between them are whether they are
copyable and at most how many times they can be run/called.

.. list-table::
   :align: center
   :widths: auto
   :header-rows: 1

   * - Type
     - Copyable
     - Moveable
     - Max runs
   * - |OnceCB|
     - ✕
     - ✓
     - 1
   * - |RepeCB|
     - ✓
     - ✓
     - ∞

Callbacks that don't take any arguments and return nothing are called
*closures*. There are two handy aliases defined for them:

* :type:`base::OnceClosure`
* :type:`base::RepeatingClosure`

.. admonition:: Example
   :class: admonition-example-code

   ``base::OnceCallback<int(float, std::string)>``
      refers to callbacks that can be run at most once, take two arguments to
      run: a float and a string, and the result of running them will be of the
      integer type.

   ``base::RepeatingCallback<void()>`` (or ``base::RepeatingClosure``)
      refers to callbacks that can be run any number of times, take no arguments
      and return nothing from each run.

.. note::

   These types act similarly to ``std::function<...>`` [#std_function]_ template
   class from the standard library, except that they provide additional
   functionality and better integration with the rest of ``libbase`` components.

.. important::

   Prefer using |OnceCB| when possible, as it provides clearer ownership and
   lifetime semantics.

.. hint::

   |RepeCB| callbacks are implicitly convertible to |OnceCB| counterparts.

.. caution::

   When using the original implementations from Chromium's ``//base`` module,
   copies of the same |RepeCB| callbacks share any bound state. In ``libbase``,
   however, they do not and the state is copied whenever the callback is copied.
   Your application should **not** depend on it as this may change in the future
   versions of ``libbase`` library.


Running callbacks
-----------------

To run (execute/call) |OnceCB| you need to ``std::move()`` the object to
obtain rvalue-reference to it. This is done to signal to the readers of code
that this callback will be reset ("moved-away") after running it.

.. admonition:: Example - running |OnceCB|
   :class: admonition-example-code

   .. code-block:: cpp

       base::OnceCallback<...> my_callback = /* ... */;
       auto result = std::move(my_callback).Run(/* arguments */);
       CHECK(!my_callback);

|RepeCB| can be run both with lvalue-reference and rvalue-reference - the
former will keep the callback unmodified, while the latter will reset it.

.. admonition:: Example - running |RepeCB|
   :class: admonition-example-code

   .. code-block:: cpp

      base::RepeatingCallback<...(...)> my_callback = /* ... */;
      auto result_1 = my_callback.Run(/* arguments */);
      CHECK(my_callback);
      auto result_2 = std::move(my_callback).Run(/* arguments */);
      CHECK(!my_callback);


Binding callbacks
-----------------

To create a callback you need to bind a functor (*function*, *member function*,
*captureless lambda* or *another callback*) and - optionally - perform a partial
application of *some* arguments.

``libbase`` provides two functions that allow you to create a callback of a
given type and perform partial application:

* :func:`base::BindOnce` - binds functor and returns |OnceCB|.
* :func:`base::BindRepeating` - binds functor and returns |RepeCB|.

All passed arguments are then copied (or moved) into the callback state and will
be used when the callback will be run.

.. note::

   These functions are similar to ``std::bind_front()`` [#std_bind_front]_ from
   the standard library.

.. attention::

   You cannot bind move-only types with a :func:`base::BindRepeating` to
   |RepeCB| if they will be passed as an argument taken by value. This is
   because |RepeCB| are allowed to be copied and execute any number of times,
   while the bound argument would have to be moved-into the bound function.


.. admonition:: Example - :func:`base::BindOnce`
   :class: admonition-example-code

   .. code-block:: cpp
      :linenos:

      #include "base/bind.h"
      #include "base/callback.h"

      int GetNext(int x) {
        return x + 1;
      }

      int main() {
        base::OnceCallback<int(int)> cb_1 = base::BindOnce(&GetNext);
        CHECK(std::move(cb_1).Run(5) == 6);
        CHECK(!cb_1);  // `cb_1` can be executed only once

        base::OnceCallback<int()> cb_2 = base::BindOnce(&GetNext, 3);
        CHECK(std::move(cb_1).Run() == 4);
        CHECK(!cb_2);  // `cb_2` can be executed only once too

        return 0;
      }

.. admonition:: Example - :func:`base::BindRepeating`
   :class: admonition-example-code

   .. code-block:: cpp
      :linenos:

      #include "base/bind.h"
      #include "base/callback.h"

      int IncrementBy(int increment, int value) {
        return value + increment;
      }

      int main() {
        base::RepeatingCallback<int(int)> get_next =
            base::BindOnce(&IncrementBy, 1);

        CHECK(get_next.Run(5) == 6);
        CHECK(get_next.Run(2) == 3);
        CHECK(get_next);  // `get_next` is still valid

        return 0;
      }


Binding adapters
----------------

Member functions
~~~~~~~~~~~~~~~~

``libbase`` library **forbids** binding member functions directly to a raw
pointers to the object of that class as this is often source of many
lifetime-related bugs. To bind a callback pointing to a member function, you
need to use one of the provided adapters.

Shared pointers
^^^^^^^^^^^^^^^

One of the ways you can bind a member function to a specific object is to pass a
``std::shared_ptr<>`` to that object wrapped in :func:`base::RetainedRef`
adapter. This adapter binds the shared pointer within the callback state.

.. warning::

   This means that the object's lifetime will be extended with the lifetime of
   the callback and all of its copies!

.. admonition:: Example - binding member function to ``std::shared_ptr<>``
   :class: admonition-example-code

   .. code-block:: cpp

      #include <iostream>
      #include <memory>

      #include "base/bind.h"
      #include "base/callback.h"

      struct Foo {
        void bar() { std::cout << "Hello World!\n"; }
      };

      int main() {
        std::shared_ptr<Foo> foo = std::make_shared<Foo>();
        auto callback = base::BindRepeating(&Foo::bar, base::RetainedRef(foo));
        callback.Run();  // will print "Hello World!";
        return 0;
      }

Weak pointers
^^^^^^^^^^^^^

If you do not want to use ``shared_ptr`` or possibly extend the lifetime of your
object, you may use :class:`base::WeakPtr`. These weak pointers allow you to
check if the pointed-to object is still alive and - if so - access it safely.

When a callback to a member function bound with a :class:`base::WeakPtr` is
run, the member function will only be executed if the original object pointed to
by the :class:`base::WeakPtr` is still alive, otherwise the call will be ignored
and there will be no attempt to access the (now probably destroyed) original
object. There are no special/extra adapter for weak pointers - the
:class:`base::WeakPtr` is itself recognized as such and allowed to be used.

.. caution::

   Using weak pointers is a bit more complicated than as described here. See
   :doc:`weak_ptrs` page for more details.

.. attention::

   It is forbidden to bind a member function, that has a non-void return type,
   to a weak pointer.

.. admonition:: Example - binding member function to ``base::WeakPtr<>``
   :class: admonition-example-code

   .. code-block:: cpp

      #include <iostream>
      #include <memory>

      #include "base/bind.h"
      #include "base/callback.h"
      #include "base/memory/weak_ptr.h"

      struct Foo {
        void bar() { std::cout << "Hello World!\n"; }
      };

      int main() {
        std::unique_ptr<Foo> foo = std::make_unique<Foo>();
        base::WeakPtr<Foo> weak_foo = /* obtain the weak pointer */;
        auto callback = base::BindRepeating(&Foo::bar, weak_foo);

        callback.Run();  // will print "Hello World!";
        foo.reset();     // this invalidates all existing weak pointers to `foo`
        callback.Run();  // NO-OP, will do nothing
                         // note that `callback` is NOT reset/empty at this point
        return 0;
      }

Manual lifetime management
^^^^^^^^^^^^^^^^^^^^^^^^^^

In scenarios where you are sure that object to which the member function will be
bound will outlive the callback, you may disable the lifetime management logic
and force the callback to bind to - essentially - a raw pointer to that object.
This can be done with :func:`base::Unretained` adapter.

.. caution::

   While this construct reduces the overhead of the callback execution, it is
   often a source of lifetime-related bugs. Be careful when using it!

.. admonition:: Example - binding member function to :func:`base::Unretained`/raw pointer
   :class: admonition-example-code

   .. code-block:: cpp

      #include <iostream>

      #include "base/bind.h"
      #include "base/callback.h"

      struct Foo {
        void bar() { std::cout << "Hello World!\n"; }
      };

      int main() {
        std::unique_ptr<Foo> foo = std::make_unique<Foo>();
        auto callback = base::BindRepeating(&Foo::bar, base::Unretained(foo.get());

        callback.Run();  // will print "Hello World!";
        foo.reset();
        // WARNING:
        //   calling `callback.Run()` now would invoke Undefined Behavior
        //   (use-after-free)
        return 0;
      }

Ignore result
~~~~~~~~~~~~~

In some scenarios, it might be required to drop the result from a callback (e.g.
to match some APIs that don't expect any return values). In such cases, it is
possible to do so with the :func:`base::IgnoreResult` adapter.

.. admonition:: Example - binding function with :func:`base::IgnoreResult` adapter
   :class: admonition-example-code

   .. code-block:: cpp

      #include <iostream>
      #include <string>

      #include "base/bind.h"
      #include "base/callback.h"

      int PrintAndReturnLength(const std::string& text) {
        std::cout << text << std::endl;
        return text.length();
      }

      void DoSomething(base::RepeatingCallback<void(const std::string&)> print_cb);

      int main() {
        base::RepeatingCallback<void(const std::string&)> print_cb =
            base::BindRepeating(base::IgnoreResult(&PrintAndReturnLength));
        DoSomething(std::move(print_cb);
        return 0;
      }

Binding arguments into the callback
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can bind arguments into a callback such that it will own them and pass them
to the function (via pointer or reference).

.. admonition:: Example - binding arguments with :func:`base::Owned` and :func:`base::OwnedRef`
   :class: admonition-example-code

   .. code-block:: cpp

      #include <iostream>
      #include <string>

      #include "base/bind.h"
      #include "base/callback.h"

      void PrintByPtr(int* value) {
        std::cout << *value << std::endl;
      }

      void PrintByRef(int& value) {
        std::cout << value << std::endl;
      }

      int main() {
        auto cb_ptr =
            base::BindRepeating(&PrintByPtr, base::Owned(std::make_unique<int>(5)));
        cb_ptr.Run();  // will print `6`

        auto cb_ref = base::BindRepeating(&PrintByRef, base::OwnedRef(1));
        cb_ref.Run();  // will print `1`

        return 0;
      }


Chaining callbacks
------------------

If you need to run multiple callbacks in a sequence, possibly passing result
from the previous callback as an input to the next one, you can use
``.Then(/* next_callback */)`` method to obtain a new callback composed from the
provided two provided ones.

   .. code-block:: cpp

      auto chained_cb = first_cb.Then(second_cb);
      // Running `chained_cb` is equivalent to calling:
      // - `first_cb.Run(); second_cb.Run();` - if `first_cb` returns nothing
      // - `second_cb.Run(first_cb.Run());`   - if `first_cb` returns a result

Normal lvalue/rvalue-reference rules applies - |OnceCB| must be *moved* while
|RepeCB| can be either copied or moved into the chained callback.

When chaining |OnceCB| with any callback, the result will be of |OnceCB| type.
|RepeCB| callbacks can be chained only with other |RepeCB| callbacks.

.. admonition:: Example - chaining callbacks
   :class: admonition-example-code

   .. code-block:: cpp

      #include <string>

      #include "base/bind.h"
      #include "base/callback.h"

      int GetNext(int value) {
        return value + 1;
      }

      std::string AsString(int value) {
        return std::to_string(value);
      }

      int main() {
        base::RepeatingCallback<std::string(int)> get_next_as_string =
            base::BindRepeating(&GetNext).Then(base::BindRepeating(&AsString));
        CHECK(get_next_as_string.Run(5) == "6");

        return 0;
      }


Chaining callbacks across different task runners
------------------------------------------------

.. caution::

   Before reading this section, familiarize yourself with threads, sequences and
   task runners on the :doc:`threads` page.

There may be a situation where you will have to pass a callback to receive a
notification of some action which will have to be processed on a specific thread
or sequence, and you won't be sure on which thread/sequence it will be invoked.

In cases like this, you can bind an existing callback to a post-task operation.
This will result in creating a new callback that - when executed - will
post-task the original callback to the target task runner. This is possible with
:func:`base::BindPostTask`.

.. admonition:: Example - :func:`base::BindPostTask`
   :class: admonition-example-code

   .. code-block:: cpp
      :linenos:

      #include "base/bind.h"
      #include "base/callback.h"

      // No guarantee on which thread/sequence the `on_done` callback will be
      // called.
      void DoSomeWorkAsync(base::OnceClosure on_done);

      // Our code
      void WorkManager::StartSomeWork() {
        auto on_done_callback = base::BindOnce(&WorkManager::WorkDone, weak_this_);
        DoSomeWorkAsync(
            base::BindPostTask(current_task_runner_, std::move(on_done_callback)));
      }

      void WorkManager::WorkDone() {
        // The check below is safe to perform
        DCHECK(current_task_runner_->RunsTasksInCurrentSequence());

        // ...
      }


Splitting a OnceCallback
------------------------

Sometimes you may have to bridge two APIs: one that takes two different
callbacks where exactly one of them will be called - one on success and the
other on failure, while on the other hand have single |OnceCB| that takes
a boolean or an enum and has to be called to notify if the operation succeded or
failed.

Due to |OnceCB| not being copyable, you cannot simply create two copies and bind
one with success value while the other with failure value.
There is however a tool to help you with this scenario:
:func:`base::SplitOnceCallback` function. It takes a single |OnceCB| callback
and returns a pair of new |OnceCB| callbacks. Running either of the new
callbacks will run the original one. Running the other will result in trggering
a ``CHECK()`` and crashing.

.. admonition:: Example - :func:`base::SplitOnceCallback`
   :class: admonition-example-code

   .. code-block:: cpp
      :linenos:

      #include "base/bind.h"
      #include "base/callback.h"

      // Assumed API that we cannot change
      void DoSomeWork(base::OnceClosure on_success_callback,
                      base::OnceClosure on_failure_callback);

      // Our implementation that has to adapt to external APIs
      void DoSomeWorkAndReport(base::OnceCallback<void(bool)> on_done_callback) {
        auto callback_pair = base::SplitOnceCallback(std::move(on_done_callback));
        DoSomeWork(base::BindOnce(std::move(callback_pair.first), true),
                   base::BindOnce(std::move(callback_pair.first), false));
      }

.. caution::

   Be careful not to overuse this functionality as it can complicate the logic
   and control flow of your application.


BarrierCallback and BarrierClosure
----------------------------------

You might be required to collect some data from several places asynchronously,
and once all the data is collected, do something with it only then. To
facilitate this scenario, ``libbase`` has :func:`base::BarrierCallback`
function.

To create a ``BarrierCallback<T>`` you need to pass two arguments:

* ``size_t required_run_count`` - number of times the callback must be run (and
  how many data chunks need to be collected).
* ``base::OnceCallback<void(Container<T>)> done_callback`` - a callback that
  will be invoked with the collected data chunks.

This will create a new |RepeCB| which will take argument of ``T`` type. Then,
after it (or its copies) will be invoked exactly ``required_run_count`` times,
the ``done_callback`` will be invoked with all the collected data chunks.

If you don't need to collect any data, you can also use
:func:`base::BarrierClosure` method.

.. tip::

   Resulting barrier callback of |RepeCB| is thread-safe, meaning that you can
   safely copy it to different threads and run it without any additional
   synchronization between them. The ``done_callback`` will be run on the same
   thread/sequence as the final call to the barrier callback.

.. admonition:: Example - :func:`base::BarrierClosure`
   :class: admonition-example-code

   .. code-block:: cpp
      :linenos:

      #include <iostream>

      #include "base/barrier_closure.h"
      #include "base/bind.h"

      int main() {
        base::RepeatingClosure barrier_cb = base::BarrierClosure(
            3, base::BindOnce([]() { std::cout << "Hello World!\n"; }));

        barrier_cb.run();  // nothing
        barrier_cb.run();  // nothing
        barrier_cb.run();  // prints "Hello World!" on the screen
        // `barrier_cb` must NOT be called any more!

        return 0;
      }


.. hint::

   You can find more details about callbacks and binding them `here \
   <https://chromium.googlesource.com/chromium/src.git/+/HEAD/docs/callback.md>`_.


.. Footnotes

.. [#std_function] https://en.cppreference.com/w/cpp/utility/functional/function
.. [#std_bind_front] https://en.cppreference.com/w/cpp/utility/functional/bind_front


.. Aliases for `base::{Once,Repeating}Callback<R(Args...)>` cross-references

.. |OnceCBF| replace:: :cpp:class:`base::OnceCallback\<Result(Arguments...)> \
  <template\<typename ReturnType, typename... ArgumentTypes> \
  base::OnceCallback\<ReturnType(ArgumentTypes...)>>`
.. |RepeCBF| replace:: :cpp:class:`base::RepeatingCallback\<Result(Arguments...)> \
  <template\<typename ReturnType, typename... ArgumentTypes> \
  base::RepeatingCallback\<ReturnType(ArgumentTypes...)>>`
.. |OnceCB| replace:: :cpp:class:`OnceCallback\<...> \
  <template\<typename ReturnType, typename... ArgumentTypes> \
  base::OnceCallback\<ReturnType(ArgumentTypes...)>>`
.. |RepeCB| replace:: :cpp:class:`RepeatingCallback\<...> \
  <template\<typename ReturnType, typename... ArgumentTypes> \
  base::RepeatingCallback\<ReturnType(ArgumentTypes...)>>`
