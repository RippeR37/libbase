#include <iostream>
#include <thread>

#include "base/bind.h"
#include "base/callback.h"

#include "base/threading/thread.h"
#include "base/threading/thread_pool.h"

namespace {
std::shared_ptr<base::SequencedTaskRunner> tr1;
std::shared_ptr<base::SequencedTaskRunner> tr2;
}  // namespace

void Task1(std::shared_ptr<base::TaskRunner> current,
           std::shared_ptr<base::TaskRunner> next,
           int n) {
  std::cout << "Writing from thread " << std::this_thread::get_id()
            << " with n=" << n << "(tr1: " << tr1->RunsTasksInCurrentSequence()
            << ", tr2: " << tr2->RunsTasksInCurrentSequence() << ")"
            << std::endl;

  if (n > 0) {
    next->PostTask(FROM_HERE, base::BindOnce(&Task1, next, current, n - 1));
  }
}

void ThreadExample() {
  base::Thread t1{};
  base::Thread t2{};

  t1.Start();
  t2.Start();

  tr1 = t1.TaskRunner();
  tr2 = t2.TaskRunner();

  tr1->PostTask(FROM_HERE, base::BindOnce(&Task1, tr1, tr2, 10));

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
