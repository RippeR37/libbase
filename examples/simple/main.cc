#include <thread>

#include "base/bind.h"
#include "base/bind_post_task.h"
#include "base/callback.h"
#include "base/init.h"
#include "base/logging.h"
#include "base/net/simple_url_loader.h"
#include "base/synchronization/auto_signaller.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"
#include "base/threading/thread_pool.h"
#include "base/timer/elapsed_timer.h"
#include "base/trace_event/trace_event.h"
#include "base/trace_event/trace_flow.h"
#include "base/trace_event/trace_flush.h"

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
  TRACE_EVENT("thread_example", "task1", "n", std::to_string(n));
  TRACE_EVENT_WITH_FLOW_STEP("thread_example", "flow:ThreadExample", &tr1);
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
  TRACE_EVENT("thread_example", "ThreadExample");
  base::Thread t1{};
  base::Thread t2{};

  t1.Start();
  t2.Start();

  tr1 = t1.TaskRunner();
  tr2 = t2.TaskRunner();

  base::WaitableEvent five_left_event{};
  base::WaitableEvent finished_event{};

  {
    TRACE_EVENT("thread_example", "Async work start");
    TRACE_EVENT_WITH_FLOW_BEGIN("thread_example", "flow:ThreadExample", &tr1);
    tr1->PostTask(FROM_HERE,
                  base::BindOnce(&Task1, tr1, tr2, &five_left_event,
                                 base::AutoSignaller{&finished_event}, 10));
  }

  five_left_event.Wait();
  LOG(INFO) << __FUNCTION__ << "() (at most 5 calls left)...";
  finished_event.Wait();
  LOG(INFO) << __FUNCTION__ << "() finished";

  {
    TRACE_EVENT_WITH_FLOW_END("thread_example", "flow:ThreadExample", &tr1);
    TRACE_EVENT("thread_example", "Async work done");
  }
  t2.Stop();
  t1.Stop();
}

void ThreadDelayedExample() {
  base::Thread thread{};
  base::WaitableEvent finished_event{};

  thread.Start();

  thread.TaskRunner()->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(
          []([[maybe_unused]] base::AutoSignaller finished_signaller) {
            LOG(INFO)
                << "ThreadDelayedExample() delayed (300ms) task executing";
          },
          base::AutoSignaller{&finished_event}),
      base::Milliseconds(300));

  thread.TaskRunner()->PostTask(
      FROM_HERE, base::BindOnce([]() {
        LOG(INFO) << "ThreadDelayedExample() non-delayed task executing";
      }));

  thread.TaskRunner()->PostDelayedTask(
      FROM_HERE, base::BindOnce([]() {
        LOG(INFO) << "ThreadDelayedExample() delayed (100ms) task executing";
      }),
      base::Milliseconds(100));

  finished_event.Wait();
  LOG(INFO) << __FUNCTION__ << "() finished";

  thread.Stop();
}

void ThreadPoolNonSequencedExample() {
  base::ThreadPool pool{1};
  pool.Start();

  auto generic_tr1 = pool.GetTaskRunner();

  pool.Stop();
}

void ThreadPoolSequencedExample() {
  base::ThreadPool pool{4};
  pool.Start();

  auto sequenced_tr1 = pool.CreateSequencedTaskRunner();
  auto sequenced_tr2 = pool.CreateSequencedTaskRunner();

  pool.Stop();
}

void ThreadPoolSingleThreadExample() {
  base::ThreadPool pool{4};
  pool.Start();

  auto single_thread_tr1 = pool.CreateSingleThreadTaskRunner();

  pool.Stop();
}

void NetExample() {
  base::Thread thread{};
  thread.Start();

  base::WaitableEvent finished_event{};
  auto response_callback = base::BindOnce(
      [](base::WaitableEvent* event, base::net::ResourceResponse response) {
        LOG(INFO) << "Result: " << static_cast<int>(response.result);
        LOG(INFO) << "HTTP code: " << response.code;
        LOG(INFO) << "Final URL: " << response.final_url;
        LOG(INFO) << "Downloaded " << response.data.size() << " bytes";
        LOG(INFO) << "Latency: " << response.timing_connect.InMilliseconds()
                  << "ms";
        LOG(INFO) << "Headers";
        for (const auto& [h, v] : response.headers) {
          LOG(INFO) << "  " << h << ": " << v;
        }
        LOG(INFO) << "Content:\n"
                  << std::string{response.data.begin(), response.data.end()};
        event->Signal();
      },
      &finished_event);

  // Try to download and signal on finish
  base::net::SimpleUrlLoader::DownloadUnbounded(
      base::net::ResourceRequest{
          "https://www.google.com/robots.txt",
          base::net::kDefaultHeaders,
          base::net::kNoTimeout,
          base::net::kNoTimeout,
          true,
          true,
      },
      base::BindPostTask(thread.TaskRunner(), std::move(response_callback),
                         FROM_HERE));

  // Timeout = 5s
  thread.TaskRunner()->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(
          [](base::WaitableEvent* event) {
            LOG(WARNING) << "Failed to download in 5 seconds";
            event->Signal();
          },
          &finished_event),
      base::Seconds(5));

  finished_event.Wait();
}

int main(int argc, char* argv[]) {
  base::InitOptions init_options;
  init_options.InitializeNetworking = true;

  base::Initialize(argc, argv, std::move(init_options));

  const auto timer = base::ElapsedTimer{};

  ThreadExample();
  ThreadDelayedExample();
  ThreadPoolNonSequencedExample();
  ThreadPoolSequencedExample();
  ThreadPoolSingleThreadExample();

  NetExample();

  LOG(INFO) << "Example finished in " << timer.Elapsed().InMillisecondsF()
            << "ms";

  TRACE_EVENT_FLUSH_TO_STREAM(LOG(INFO) << "Trace:\n");
  base::Deinitialize();
  return 0;
}
