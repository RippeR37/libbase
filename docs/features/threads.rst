Threads and sequences
=====================

Introduction
------------

Chromium introduces a lot of new concepts to facilitate writing faster and safer
multithreaded code. It is crucial to understand them to fully utilize provided
tools. ``libbase`` tries to bring the most useful of them to you.

Concepts
~~~~~~~~

.. _task:

**Task**
   A single unit of work. It can be represented with |OnceCB| or |RepeCB|
   callbacks, while most often it is :class:`base::OnceClosure`.

   .. seealso::

      See :doc:`callbacks` page for more details.

.. _thread:

**Thread**
   Threads are provided by the operating system and are used to execute code in
   parallel. Special care has to be taken if multiple threads can access the
   same data simultaneously.

   :class:`base::Thread` is an utility class that wraps a physical thread and
   allows you to execute arbitrary :ref:`tasks <task>` queued on its *task
   queue*.

.. _thread_pool:

**Thread pool**
   A *thread pool* is a pool of threads that share a common *task queue*. Each
   task enqueued on it will be executed by one of the threads within the pool
   but it is not specified by which one.

   :class:`base::ThreadPool` is an utility class that provides you the *thread
   pool* functionality while also exposing multiple different
   :ref:`task runners <task_runner>` to schedule work on them.

.. _sequence:

**Sequence**
   A sequence (sometimes also called *virtual thread*) is a logical construct
   specifying the rules of execution of :ref:`tasks <task>`. All tasks that are
   to be executed on a given sequence will be executed in a specified order and
   none will overlap (only one task can be executed at the same time) on some
   physical thread(s), but there are no guarantees on which physical threads of
   execution the tasks will be executed and it may change between the tasks.

.. _task_runner:

**Task runner**
  Task runners are objects that can be used to enqueue :ref:`tasks <task>` on a
  specific *task queue* to which they are bound. Such task will later be
  executed on the corresponding physcial thread of execution. Queuing (or
  *scheduling*) of tasks through a task runner is often referred to as
  **post-tasking** them.

  There are 3 types of task runners available to use:

  * :class:`base::TaskRunner`
       Provides no guarantees about the order of execution of posted tasks.

  * :class:`base::SequencedTaskRunner`
       All posted tasks will be invoked in the same order in which they were
       posted and none will overlap. There are no guarantees specifying on which
       physical thread of execution given tasks will execute.

  * :class:`base::SingleThreadTaskRunner`
       All posted tasks will be invoked in the same order they were posted on
       the same physical thread of execution.

Task types
~~~~~~~~~~

When dealing with a group of tasks to be executed in parallel, we can categorize
them in these groups:

* **Thread unsafe**
   These tasks do not use any synchronization primitives and may need external
   synchronization if executed in parallel on the same set of data.
   Alternatively, they can be executed on a single :ref:`thread <thread>` or
   a single :ref:`sequence <sequence>`.

.. _thread-affine:

* **Thread-affine**
   These tasks require to be run only on a single (possibly on a specific
   instance of) :ref:`thread <thread>`.

* **Thread-safe**
   These tasks can be safely executed from any :ref:`threads <thread>` and/or
   :ref:`sequences <sequence>` in parallel.


Sequences vs Threads
--------------------

.. important::

   There are a number of benefits to executing your tasks in sequences over the
   physical threads and as such, **it is highly preferred to write code that can
   be executed on any sequence instead of being**
   :ref:`thread-affine <thread-affine>`.

   Some benefits to using sequences over physical threads:

   * Code is easier to understand and reason about.
   * Code is easier to reuse in different components.
   * Code is easier to be parallelized.
   * Fewer physical threads means smaller overhead.
   * You can post tasks to any number of sequences that are tied to a number of
     threads that match your hardware. This allows you to fully utilize the CPU
     power without the overhead of context switches.


:class:`base::TaskRunner`
-------------------------

The :class:`base::TaskRunner` interface has two main methods and a few helpers
to make it easier to write your code. The main method is:

* :func:`base::TaskRunner::PostTask`

   This function takes two arguments - a task to be executed and a location in
   source code (aquired via :c:macro:`FROM_HERE` macro) from where the post-task
   operation is done. When called, the passed task will be queued on a task
   queue associated with that task runner.

   .. important::

      Remember: there are no guarantees as to ordering of execution between two
      tasks posted to the same :class:`base::TaskRunner` or whether they will be
      executed on the same physical thread at all.

   .. admonition:: Example
      :class: admonition-example-code

      .. code-block:: cpp

         void ScheduleTwoTasks(std::shared_ptr<base::TaskRunner> task_runner) {
           DCHECK(task_runner) << "task_runner should be provided";

           base::OnceClosure task_1 = /* acquire task_1 */;
           base::OnceClosure task_2 = /* acquire task_2 */;

           task_runner->PostTask(FROM_HERE, std::move(task_1));
           task_runner->PostTask(FROM_HERE, std::move(task_2));

           // `task_1` and `task_2` will be executed in some order in the future
         }

* :func:`base::TaskRunner::PostDelayedTask`

   This function behaves similarly to the above one, but takes one more
   parameter (:class:`base::TimeDelta delay <base::TimeDelta>`) and ensures that
   the posted task will **not** be executed before ``delay`` time has passed.

   .. admonition:: Example
      :class: admonition-example-code

      .. code-block:: cpp

         void ScheduleTwoDelayedTasks(std::shared_ptr<base::TaskRunner> task_runner) {
           DCHECK(task_runner) << "task_runner should be provided";

           base::OnceClosure task_1 = /* acquire task_1 */;
           base::OnceClosure task_2 = /* acquire task_2 */;

           task_runner->PostDelayedTask(FROM_HERE, std::move(task_1), base::Seconds(1));
           task_runner->PostDelayedTask(FROM_HERE, std::move(task_2), base::Seconds(2));

           // `task_1` will be executed after at least one second has passed
           // `task_2` will be executed after at least two seconds have passed
         }

   .. caution::

      In the above example it is still **not** guaranteed that ``task_1`` will
      be executed before ``task_2``!

There are also two additional helper functions defined in that class:

* :func:`base::TaskRunner::PostTaskAndReply`
     This helper can be used to post-task operation that - when finished - will
     automatically post-task a *reply* task on the original task runner from
     which the original call was made.

     .. note::

        In this case, the ``reply`` callback is guaranteed to be run after the
        ``task`` callback.

     .. caution::

        This method can be called **only** from a thread with a task queue
        (:class:`base::Thread` or :class:`base::ThreadPool`)!

* :func:`base::TaskRunner::PostTaskAndReply`
     Similar to the above, but ``task`` callback should return a result that
     will be passed to the ``reply`` callback.


:class:`base::SequencedTaskRunner`
----------------------------------

This interface inherits from :class:`base::TaskRunner` and adds an additional
method called :func:`base::SequencedTaskRunner::RunsTasksInCurrentSequence` that
can be used to check if currently-executed task is executing within the same
sequence as the one affiliated with that task runner.

.. important::

   All tasks posted with task runners of this type will be executed in the same
   sequence in order in which they were posted.

.. admonition:: Example
   :class: admonition-example-code

   .. code-block:: cpp

      void ScheduleTwoSequencedTasks(
          std::shared_ptr<base::SequencedTaskRunner> sequenced_task_runner) {
        DCHECK(task_runner) << "task_runner should be provided";

        base::OnceClosure task_1 = /* acquire task_1 */;
        base::OnceClosure task_2 = /* acquire task_2 */;

        sequenced_task_runner->PostTask(FROM_HERE, std::move(task_1));
        sequenced_task_runner->PostTask(FROM_HERE, std::move(task_2));

        // It is guaranteed that `task_1` will finish before `task_2` will be
        // started and that `task_2` will *see* all effects of `task_1`'s work.
      }


:class:`base::SingleThreadTaskRunner`
-------------------------------------

This interface inherits from :class:`base::SequencedTaskRunner` and adds an additional
method called :func:`base::SingleThreadTaskRunner::BelongsToCurrentThread`
which is just an alias for
:func:`base::SequencedTaskRunner::RunsTasksInCurrentSequence`.


.. important::

   All tasks posted with task runners of this type will be executed on the same
   physical thread in order in which they were posted.

.. admonition:: Example
   :class: admonition-example-code

   .. code-block:: cpp

      void ScheduleTwoSingleThreadedTasks(
          std::shared_ptr<base::SingleThreadTaskRunner> single_thread_task_runner) {
        DCHECK(task_runner) << "task_runner should be provided";

        base::OnceClosure task_1 = /* acquire task_1 */;
        base::OnceClosure task_2 = /* acquire task_2 */;

        single_thread_task_runner->PostTask(FROM_HERE, std::move(task_1));
        single_thread_task_runner->PostTask(FROM_HERE, std::move(task_2));

        // It is guaranteed that both tasks will be executed on the same physical
        // thread and that `task_1` will finish before `task_2` will be started.
      }


:class:`base::Thread`
---------------------

This class can be used to create a new physical thread of execution. Once
created, it needs to be started (with :func:`base::Thread::Start`) to start
execution of tasks on its task queue. If not stopped before being destroyed, it
will stop and join in its destructor.

After the thread is started, you can obtain a
:class:`base::SingleThreadTaskRunner` by calling
:func:`base::Thread::TaskRunner` member function.

.. admonition:: Example - :class:`base::Thread`
   :class: admonition-example-code

   .. code-block:: cpp
      :linenos:
      :caption: Program

      #include <iostream>

      #include "base/bind.h"
      #include "base/callback.h"
      #include "base/single_thread_task_runner.h"
      #include "base/threading/thread.h"

      void SayHello(const std::string& text) {
        std::cout << "Hello " << text << "!" << std::endl;
      }

      int main() {
        base::Thread thread;
        auto task_runner = thread.TaskRunner();

        task_runner->PostTask(FROM_HERE, base::BindOnce(&SayHello, "World"));
        task_runner->PostTask(FROM_HERE, base::BindOnce(&SayHello, "Everyone"));

        thread.Stop();
        return 0;
      }

   .. code-block:: console
      :caption: Program output

      Hello World!
      Hello Everyone!


:class:`base::ThreadPool`
-------------------------

This class can be used to create a pool of physical threads of execution. To
create it, you need to specify the initial number of physical threads that will
be created in that pool. Once created, it needs to be started (with
:func:`base::ThreadPool::Start`) to start execution of tasks on its task queue.
If not stopped before being destroyed, it will stop and join in its destructor.

After the thread is started, you can obtain or create different task runners to
this thread pool with these methods:

* :func:`base::ThreadPool::GetTaskRunner`
    This member function returns a :class:`base::TaskRunner` that schedules
    tasks for execution on the thread pool without any guarantees about ordering
    with different tasks scheduled to it and without specifying on which thread
    within the pool the task will be executed.

* :func:`base::ThreadPool::CreateSequencedTaskRunner`
   This member function creates a new :class:`base::SequencedTaskRunner` that
   schedules tasks for execution on the thread pool within a single
   :ref:`sequence <sequence>`.

   .. caution::

      Calling this method multiple times will return you task runners belonging
      to new and unique sequences! If you want to ensure that tasks end up being
      posted to the same sequence, you need to hold on to the already obtained
      task runners and reuse them.

* :func:`base::ThreadPool::CreateSingleThreadTaskRunner`
   This member function creates a new :class:`base::SingleThreadTaskRunner` that
   schedules tasks for execution on a single (but unspecified which) physical
   thread within the thread.

   .. caution::

      Calling this method multiple times will return you task runners thay may
      be bound to a different physical threads! If you want to ensure that tasks
      end up being posted to the same physical thread, you need to hold on to
      the already obtained task runners and reuse them.

.. admonition:: Example - :class:`base::ThreadPool`
   :class: admonition-example-code

   .. code-block:: cpp
      :linenos:
      :caption: Program

      #include <iostream>
      #include <mutex>

      #include "base/bind.h"
      #include "base/callback.h"
      #include "base/single_thread_task_runner.h"
      #include "base/threading/thread.h"

      std::mutex g_cout_mutex;

      void Print(const std::string& text) {
        std::lock_guard<std::mutex> guard(g_cout_mutex);
        std::cout << text << std::endl;
      }

      int main() {
        base::ThreadPool thread_pool{4};

        auto task_runner = thread_pool.GetTaskRunner();
        auto seq_task_runner = thread_pool.CreateSequencedTaskRunner();
        auto st_task_runner = thread_pool.CreateSingleThreadTaskRunner();

        task_runner->PostTask(FROM_HERE, base::BindOnce(&Print, "Generic1"));
        task_runner->PostTask(FROM_HERE, base::BindOnce(&Print, "Generic2"));

        seq_task_runner->PostTask(FROM_HERE, base::BindOnce(&Print, "Seq1"));
        seq_task_runner->PostTask(FROM_HERE, base::BindOnce(&Print, "Seq2"));
        seq_task_runner->PostTask(FROM_HERE, base::BindOnce(&Print, "Seq3"));

        st_task_runner->PostTask(FROM_HERE, base::BindOnce(&Print, "St1"));
        st_task_runner->PostTask(FROM_HERE, base::BindOnce(&Print, "St2"));
        st_task_runner->PostTask(FROM_HERE, base::BindOnce(&Print, "St3"));

        thread_pool.Stop();
        return 0;
      }

   .. code-block:: console
      :caption: Possible program output

      St1
      St2
      Generic2
      Seq1
      St3
      Generic1
      Seq2
      Seq3

.. hint::

   The only guarantees about the output of the above program are that:

   * ``Seq1`` will be printed before ``Seq2`` and both will be printed before
     ``Seq3``.
   * ``St1`` will be printed before ``St2``, both will be printed before
     ``Seq3``, and all of these three outputs will be printed from the same
     physical thread.


Obtaining current :class:`base::SequencedTaskRunner`
----------------------------------------------------

You can obtain the :class:`base::SequencedTaskRunner` on which the currently
executed task is executed with :func:`base::SequencedTaskRunnerHandle::Get()`
static function.

.. warning::

   This method may only be called from tasks executed within a sequence. If
   you're not sure where the task is executed, you need to first call a
   :func:`base::SequencedTaskRunnerHandle::IsSet()` static function to check if
   current task is executed on a sequence.


.. admonition:: Example - :class:`base::SequencedTaskRunnerHandle`
   :class: admonition-example-code

   .. code-block:: cpp
      :linenos:

      #include "base/bind.h"
      #include "base/callback.h"
      #include "base/sequenced_task_runner.h"

      void VerifyRunOnSpecificTaskRunner(
          std::shared_ptr<base::SequencedTaskRunner> task_runner) {
        CHECK(task_runner == base::SequencedTaskRunnerHandle::Get());
      }

      void Test(std::shared_ptr<base::SequencedTaskRunner> task_runner) {
        task_runner->PostTask(
            FROM_HERE,
            base::BindOnce(&VerifyRunOnSpecificTaskRunner, task_runner));
      }


Ensuring sequence affinity
--------------------------

When writing a code that must be executed within the same sequence, it is a good
practice to add (possibly debug-only) checks that will verify if - for example -
all calls to such class are made from the correct sequence.

You can use :class:`base::SequenceChecker` helper class to do this. Objects of
this class, when constructed, bind to the sequence the current task runs in and
later on you can verify that the object is used on correct (the same) sequence
by calling :func:`base::SequenceChecker::CalledOnValidSequence` member function.
Invalid usage of code protected with that check will trigger a ``CHECK()`` macro
that will crash your application.

You can also detach a bound :class:`base::SequenceChecker` from current sequence
and allow it to bind to the one on which it will be used the next time. This
**must** be done from the previously bound sequence and is often useful when
creating objects on some sequence and them passing them to another sequence,
possibly for the rest of their lifetime. This way allows you to acquire - for
example - a weak pointer to that object, so that you can safely post tasks to it
even if you're not sure about whether it is still alive.

.. hint::

   There are also helper macros defined for you that can be used to create a
   :class:`base::SequenceChecker` and perform checks **but only within a debug
   builds**:

   * :c:macro:`SEQUENCE_CHECKER(name) <SEQUENCE_CHECKER>`
        Creates a :class:`base::SequenceChecker` object with given name. This is
        usually used to define a debug-only member variable.

   * :c:macro:`DCHECK_CALLED_ON_VALID_SEQUENCE(name) <DCHECK_CALLED_ON_VALID_SEQUENCE>`
        Verifies that current call is done on a correct sequence through a given
        :class:`base::SequenceChecker` instance.

   * :c:macro:`DETACH_FROM_SEQUENCE(name) <DETACH_FROM_SEQUENCE>`
        Detaches given :class:`base::SequenceChecker` instance from currently
        bound sequence.

.. admonition:: Example - :class:`base::SequenceChecker`
   :class: admonition-example-code

   .. code-block:: cpp

      #include "base/sequence_checker.h"

      // This class can be created on any sequenced and then all other usages
      // (including its destruction) must be done on the same (but possibly
      // a different one than the one on which the object was created) sequence.
      class Foo {
       public:
        Foo() {
          DETACH_FROM_SEQUENCE(sequence_checker_);
          // ...
        }

        ~Foo() {
          DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
          // ...
        }

        void Bar() {
          DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
          // ...
        }

       private:
        SEQUENCE_CHECKER(sequence_checker_);
        // ...
      };

.. hint::

   It is best to use provided macros to work with :class:`base::SequenceChecker`
   to avoid overhead in release builds.


Canceling posted task
---------------------

By default, once a task is posted, you have no control whether it will be
executed or not. To allow yourself to cancel already posted task (but only if it
wasn't executed yet) you can bind the callback to a :class:`base::WeakPtr` and -
if needed - invalidate it which will stop it from being executed.

.. seealso::

   See more on :doc:`weak_ptrs` page.


Blocking post-tasks
-------------------

.. caution::

   Using this mechanism is not recommended unless there is a good reason for it.
   Overusing it may complicate your code and severly affect performance of your
   application. In most cases a better way is to design components to work
   asynchronously whenever possible. Take special care to minimize usages of
   blocking post-tasks and ensure that they are used in a safe and correct way
   to avoid problems (e.g. deadlocks).

If you need to block execution of the current task or thread until some other
task finishes on the other thread, you can use :class:`base::WaitableEvent`
class.

To create it you need to specify two parameters that decide how the object will
behave:

* :enum:`base::WaitableEvent::ResetPolicy`
     This enum decides whether checking or waiting on a waitable event will
     reset its state back to not-signalled or if the state will be preserved
     until a call to :func:`base::WaitableEvent::Reset`.

* :enum:`base::WaitableEvent::InitialState`
     This enum decides whether newly created :class:`base::WaitableEvent` object
     is signaled from the start or not.

Once created, you can pass a reference or a pointer to it to some callback that
will - eventually - signal it, and wait for it on your thread. This will stop
processing current and any other tasks on this thread and sequence until the
waitable event will be signalled from a different thread.

.. hint::

   To write safer code, you can also use :class:`base::AutoSignaller` that will
   automatically signal the waitable event on its destruction. This can help you
   ensure that the logic will be unblocked even if post-tasking fails or
   something unexpected happens.

.. admonition:: Example - :class:`base::WaitableEvent`
   :class: admonition-example-code

   .. code-block:: cpp

      void DoSomethingOnOtherSequence(base::AutoSignaller) {
        DoSomething();
      }

      void PostDoSomethingOnSequenceAndWait(
          std::shared_ptr<base::TaskRunner> task_runner) {
        base::WaitableEvent event{};

        task_runner->PostTask(
            FROM_HERE,
            base::BindOnce(&DoSomethingOnOtherSequence,
                           base::AutoSignaller{&event}));
        event.Wait();

        // `DoSomething()` has finished by this point (or post-tasking has failed)
      }


.. attention::

   If you need to use :class:`base::WaitableEvent` together with ``std::mutex``,
   you should probably use ``std::mutex`` with ``std::condition_variable``
   instead.


.. seealso::

   For more details, please refer to the Chromium's `Threading and tasks \
   <https://chromium.googlesource.com/chromium/src.git/+/HEAD/docs/threading_and_tasks.md>`_
   documentation page.


.. Aliases

.. |OnceCB| replace:: :cpp:class:`base::OnceCallback\<...> \
  <template\<typename ReturnType, typename... ArgumentTypes> \
  base::OnceCallback\<ReturnType(ArgumentTypes...)>>`
.. |RepeCB| replace:: :cpp:class:`base::RepeatingCallback\<...> \
  <template\<typename ReturnType, typename... ArgumentTypes> \
  base::RepeatingCallback\<ReturnType(ArgumentTypes...)>>`
