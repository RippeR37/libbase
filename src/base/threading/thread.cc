#include "base/threading/thread.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/message_loop/message_loop_impl.h"
#include "base/message_loop/message_pump_impl.h"
#include "base/sequenced_task_runner_helpers.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/delayed_task_manager_shared_instance.h"
#include "base/threading/task_runner_impl.h"

namespace base {

Thread::Thread() = default;

Thread::~Thread() {
  Stop();
}

std::thread::id Thread::Id() const {
  if (!thread_) {
    return {};
  }
  return thread_->get_id();
}

void Thread::Start() {
  const size_t executor_count = 1;
  auto message_pump = std::make_shared<MessagePumpImpl>(executor_count);

  const MessagePump::ExecutorId executor_id = 0;
  message_loop_ = std::make_unique<MessageLoopImpl>(executor_id, message_pump);
  thread_ =
      std::make_unique<std::thread>(&MessageLoop::Run, message_loop_.get());

  std::weak_ptr<MessagePump> weak_message_pump = message_pump;
  task_runner_ = std::make_unique<SingleThreadTaskRunnerImpl>(
      weak_message_pump, detail::SequenceIdGenerator::GetNextSequenceId(),
      executor_id,
      DelayedTaskManagerSharedInstance::GetOrCreateSharedInstance());
}

void Thread::Stop() {
  Stop(FROM_HERE, base::OnceClosure{});
}

void Thread::Stop(SourceLocation location, OnceClosure last_task) {
  if (message_loop_) {
    (void)location;  // TODO: use `location`
    message_loop_->Stop(std::move(last_task));
  }
  if (thread_) {
    thread_->join();
  }

  thread_.reset();
  message_loop_.reset();
  task_runner_.reset();
}

std::shared_ptr<SingleThreadTaskRunner> Thread::TaskRunner() {
  return task_runner_;
}

void Thread::FlushForTesting() {
  if (!thread_) {
    return;
  }

  WaitableEvent waitable_event{WaitableEvent::ResetPolicy::kAutomatic,
                               WaitableEvent::InitialState::kNotSignaled};
  TaskRunner()->PostTask(
      FROM_HERE,
      BindOnce([](WaitableEvent* event) { event->Signal(); }, &waitable_event));
  waitable_event.Wait();
}

}  // namespace base
