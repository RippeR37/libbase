#include "base/threading/thread_pool.h"

#include <thread>

#include "base/message_loop/message_loop_impl.h"
#include "base/message_loop/message_pump_impl.h"
#include "base/sequenced_task_runner_helpers.h"

namespace base {

namespace {
// TODO: move into separate classes (& try to solve DRY)
// TODO: it shouldn't always inherit from `SingleThreadTaskRunner`
class ThreadPoolTaskRunnerImpl : public SingleThreadTaskRunner {
 public:
  ThreadPoolTaskRunnerImpl(std::weak_ptr<MessagePump> pump,
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

struct ThreadPool::ThreadData {
  std::unique_ptr<MessageLoop> message_loop;
  std::unique_ptr<std::thread> thread;
};

ThreadPool::ThreadPool(size_t initial_size) : initial_size_(initial_size) {}

ThreadPool::~ThreadPool() {
  Join();
}

void ThreadPool::Start() {
  auto message_pump = std::make_shared<MessagePumpImpl>();

  for (size_t thread_idx = 0; thread_idx < initial_size_; ++thread_idx) {
    const MessagePump::ExecutorId executor_id = thread_idx;
    auto message_loop =
        std::make_unique<MessageLoopImpl>(executor_id, message_pump);
    auto thread =
        std::make_unique<std::thread>(&MessageLoop::Run, message_loop.get());

    threads_.push_back({std::move(message_loop), std::move(thread)});
  }

  std::weak_ptr<MessagePump> weak_message_pump = message_pump;
  task_runner_ = std::make_shared<ThreadPoolTaskRunnerImpl>(
      weak_message_pump, std::nullopt, std::nullopt);
}

void ThreadPool::Join() {
  for (auto& thread : threads_) {
    thread.message_loop->Stop();
    thread.thread->join();
  }
  threads_.clear();
}

std::shared_ptr<TaskRunner> ThreadPool::GetTaskRunner() const {
  return task_runner_;
}

std::shared_ptr<SequencedTaskRunner> ThreadPool::CreateSequencedTaskRunner() {
  return std::make_shared<ThreadPoolTaskRunnerImpl>(
      pump_, detail::SequenceIdGenerator::GetNextSequenceId(), std::nullopt);
}

std::shared_ptr<SingleThreadTaskRunner>
ThreadPool::CreateSingleThreadTaskRunner() {
  MessagePump::ExecutorId allowed_executor_id =
      0;  // TODO: should be random from existing threads!

  return std::make_shared<ThreadPoolTaskRunnerImpl>(
      pump_, detail::SequenceIdGenerator::GetNextSequenceId(),
      allowed_executor_id);
}

}  // namespace base
