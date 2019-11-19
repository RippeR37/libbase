#include "base/threading/thread.h"

#include "base/message_loop/message_loop_impl.h"
#include "base/message_loop/message_pump_impl.h"
#include "base/sequenced_task_runner_helpers.h"

namespace base {

namespace {
class ThreadTaskRunnerImpl : public SingleThreadTaskRunner {
 public:
  ThreadTaskRunnerImpl(std::weak_ptr<MessagePump> pump,
                       std::optional<SequenceId> sequence_id,
                       std::optional<MessagePump::ExecutorId> executor_id)
      : pump_(std::move(pump)),
        sequence_id_(std::move(sequence_id)),
        executor_id_(std::move(executor_id)) {}

  // TaskRunner
  bool PostTask(SourceLocation location, OnceCallback<void()> task) override {
    (void)location;

    if (auto pump = pump_.lock()) {
      pump->QueuePendingTask({std::move(task), sequence_id_, executor_id_});
      return true;
    }

    return false;
  }

  // SequencedTaskRunner
  bool RunsTasksInCurrentSequence() const override {
    return sequence_id_ &&
           detail::CurrentSequenceIdHelper::IsCurrentSequence(*sequence_id_);
  }

 private:
  const std::weak_ptr<MessagePump> pump_;
  const std::optional<SequenceId> sequence_id_;
  const std::optional<MessagePump::ExecutorId> executor_id_;
};
}  // namespace

Thread::Thread() {}

std::thread::id Thread::Id() const {
  if (!thread_) {
    return std::thread::id{};
  }
  return thread_->get_id();
}

void Thread::Start() {
  auto message_pump = std::make_shared<MessagePumpImpl>();

  const MessagePump::ExecutorId executor_id = 0;
  message_loop_ = std::make_unique<MessageLoopImpl>(executor_id, message_pump);
  thread_ =
      std::make_unique<std::thread>(&MessageLoop::Run, message_loop_.get());

  std::weak_ptr<MessagePump> weak_message_pump = message_pump;
  task_runner_ = std::make_unique<ThreadTaskRunnerImpl>(
      weak_message_pump, detail::SequenceIdGenerator::GetNextSequenceId(),
      executor_id);
}

void Thread::Join() {
  message_loop_->Stop();
  thread_->join();

  thread_.reset();
  message_loop_.reset();
}

std::shared_ptr<SingleThreadTaskRunner> Thread::TaskRunner() {
  return task_runner_;
}

}  // namespace base
