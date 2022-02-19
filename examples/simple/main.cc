#include <thread>

#include "base/bind.h"
#include "base/callback.h"
#include "base/logging.h"
#include "base/synchronization/auto_signaller.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"
#include "base/threading/thread_pool.h"
#include "base/vlog_is_on.h"

namespace {
std::shared_ptr<base::SequencedTaskRunner> tr1;
std::shared_ptr<base::SequencedTaskRunner> tr2;
}  // namespace

void Task1(std::shared_ptr<base::TaskRunner> current,
           std::shared_ptr<base::TaskRunner> next,
           base::WaitableEvent* five_left_event,
           base::AutoSignaller finished_event,
           int n) {
  CHECK_GE(n, 0) << "`n` must be greater or equal to 0";
  LOG(INFO) << __FUNCTION__ << "() Writing from thread "
            << std::this_thread::get_id() << " with n=" << n
            << "(tr1: " << tr1->RunsTasksInCurrentSequence()
            << ", tr2: " << tr2->RunsTasksInCurrentSequence() << ")"
            << std::endl;

  if (n == 5) {
    five_left_event->Signal();
  }
  if (n > 0) {
    next->PostTask(FROM_HERE,
                   base::BindOnce(&Task1, next, current, five_left_event,
                                  std::move(finished_event), n - 1));
  }
}

void ThreadExample() {
  base::Thread t1{};
  base::Thread t2{};

  t1.Start();
  t2.Start();

  tr1 = t1.TaskRunner();
  tr2 = t2.TaskRunner();

  base::WaitableEvent five_left_event{};
  base::WaitableEvent finished_event{};
  tr1->PostTask(FROM_HERE,
                base::BindOnce(&Task1, tr1, tr2, &five_left_event,
                               base::AutoSignaller{&finished_event}, 10));

  five_left_event.Wait();
  LOG(INFO) << __FUNCTION__ << "() (at most 5 calls left)...";
  finished_event.Wait();
  LOG(INFO) << __FUNCTION__ << "() finished";

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

int main(int /*argc*/, char* argv[]) {
  FLAGS_logtostderr = true;
  FLAGS_v = 3;
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  ThreadExample();
  ThreadPoolNonSequencedExample();
  ThreadPoolSequencedExample();
  ThreadPoolSingleThreadExample();

  google::ShutdownGoogleLogging();

  return 0;
}
