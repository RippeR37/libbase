#include <iostream>
#include <thread>

#include "base/bind.h"
#include "base/callback.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"
#include "base/threading/thread_pool.h"

namespace {
std::shared_ptr<base::SequencedTaskRunner> tr1;
std::shared_ptr<base::SequencedTaskRunner> tr2;
}  // namespace

void Task1(std::shared_ptr<base::TaskRunner> current,
           std::shared_ptr<base::TaskRunner> next,
           base::WaitableEvent* event,
           int n) {
  std::cout << "Writing from thread " << std::this_thread::get_id()
            << " with n=" << n << "(tr1: " << tr1->RunsTasksInCurrentSequence()
            << ", tr2: " << tr2->RunsTasksInCurrentSequence() << ")"
            << std::endl;

  if (n == 5) {
    event->Signal();
  }
  if (n > 0) {
    next->PostTask(FROM_HERE,
                   base::BindOnce(&Task1, next, current, event, n - 1));
  }
}

void ThreadExample() {
  base::Thread t1{};
  base::Thread t2{};

  t1.Start();
  t2.Start();

  tr1 = t1.TaskRunner();
  tr2 = t2.TaskRunner();

  base::WaitableEvent event{base::WaitableEvent::ResetPolicy::kAutomatic,
                            base::WaitableEvent::InitialState::kNotSignaled};
  tr1->PostTask(FROM_HERE, base::BindOnce(&Task1, tr1, tr2, &event, 10));

  event.Wait();
  std::cout << "(at most 5 calls left)...\n";

  std::this_thread::sleep_for(std::chrono::seconds(1));

  t2.Join();
  t1.Join();
}

void ThreadPoolNonSequencedExample() {
  base::ThreadPool pool{1};
  pool.Start();

  auto generic_tr1 = pool.GetTaskRunner();

  pool.Join();
}

void ThreadPoolSequencedExample() {
  base::ThreadPool pool{4};
  pool.Start();

  auto sequenced_tr1 = pool.CreateSequencedTaskRunner();
  auto sequenced_tr2 = pool.CreateSequencedTaskRunner();

  pool.Join();
}

void ThreadPoolSingleThreadExample() {
  base::ThreadPool pool{4};
  pool.Start();

  auto single_thread_tr1 = pool.CreateSingleThreadTaskRunner();

  pool.Join();
}

int main() {
  ThreadExample();
  ThreadPoolNonSequencedExample();
  ThreadPoolSequencedExample();
  ThreadPoolSingleThreadExample();

  return 0;
}
