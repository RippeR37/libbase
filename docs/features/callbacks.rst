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

   ``base::OnceCallback<int(float, std::string)>``
      refers to callbacks that can be run at most once, take two arguments to
      run: a float and a string, and the result of running them will be of the
      integer type.

   ``base::RepeatingCallback<void()>`` (or ``base::RepeatingClosure``)
      refers to callbacks that can be run any number of times, take no arguments
      and return nothing from each run.

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

.. code-block:: cpp

   base::OnceCallback<...> my_callback = /* ... */;
   auto result = std::move(my_callback).Run(/* arguments */);
   CHECK(!my_callback);

|RepeCB| can be run both with lvalue-reference and rvalue-reference - the
former will keep the callback unmodified, while the latter will reset it.

.. code-block:: cpp

   base::RepeatingCallback<...(...)> my_callback = /* ... */;
   auto result_1 = my_callback.Run(/* arguments */);
   CHECK(my_callback);
   auto result_2 = std::move(my_callback).Run(/* arguments */);
   CHECK(!my_callback);

.. note::

   These types act similarly to ``std::function<...>`` [#std_function]_ template
   class from the standard library, except that they provide additional
   functionality and better integration with the rest of ``libbase`` components.

.. important::

   Prefer using |OnceCB| when possible, as it provides clearer ownership and
   lifetime semantics.

.. hint::

   |RepeCB| callbacks are implicitly convertible to |OnceCB| counterparts.


Binding callbacks
-----------------

To create a callback you need to bind a functor (*function*, *member function*,
*captureless lambda* or *another callback*) and - optionally - perform a partial
application of *some* arguments.

``libbase`` provides two functions that allow you to create a callback of a
given type and perform partial application:

* :func:`base::BindOnce` - binds functor and returns |OnceCB|.
* :func:`base::BindRepeating` - binds functor and returns |RepeCB|.

.. note::

   These functions are similar to ``std::bind_front()`` [#std_bind_front]_ from
   the standard library.


.. admonition:: Example - :func:`base::BindOnce`

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


Binding modifiers
-----------------

.. todo::

   Add this section.


Chaining callbacks
------------------

If you need to run multiple callbacks in a sequence, possibly passing result
from the previous callback as an input to the next one, you can use
``.Then(/* next_callback */)`` method to obtain a new callback composed from the
provided two provided ones.

  .. code-block:: cpp

     auto chained_cb = first_cb.Then(second_cb);
     // Running `chained_cb` is equivalent to calling:
     // * `first_cb.Run(); second_cb.Run()` - if `first_cb` returns nothing
     // * `second_cb.Run(first_cb.Run())`   - if `first_cb` returns a result

Normal lvalue/rvalue-reference rules applies - |OnceCB| must be *moved* while
|RepeCB| can be either copied or moved into the chained callback.

When chaining |OnceCB| with any callback, the result will be of |OnceCB| type.
|RepeCB| callbacks can be chained only with other |RepeCB| callbacks.


Chaining callbacks across different task runners
------------------------------------------------

.. caution::

   Before reading this section, familiarize yourself with threads, sequences and
   task runners on the :doc:`threads` page.

.. todo::

   Add this section.


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

   Take care in not overusing this functionality as it can complicate the logic
   and control flow of your application.


BarrierCallback
---------------

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
