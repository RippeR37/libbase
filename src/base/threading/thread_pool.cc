#include "base/threading/thread_pool.h"

#include <thread>

#include "base/callback.h"
#include "base/logging.h"
#include "base/message_loop/message_loop_impl.h"
#include "base/message_loop/message_pump_impl.h"
#include "base/sequenced_task_runner_helpers.h"
#include "base/threading/delayed_task_manager_shared_instance.h"
#include "base/threading/task_runner_impl.h"

namespace base {

struct ThreadPool::ThreadData {
  std::unique_ptr<MessageLoop> message_loop;
  std::unique_ptr<std::thread> thread;
};

ThreadPool::ThreadPool(size_t initial_size)
    : initial_size_(initial_size), random_generator_(std::random_device{}()) {
  DCHECK_GT(initial_size_, 0u);
}

ThreadPool::~ThreadPool() {
  Stop();
}

void ThreadPool::Start() {
  auto message_pump = std::make_shared<MessagePumpImpl>(initial_size_);

  for (size_t thread_idx = 0; thread_idx < initial_size_; ++thread_idx) {
    const MessagePump::ExecutorId executor_id = thread_idx;
    auto message_loop =
        std::make_unique<MessageLoopImpl>(executor_id, message_pump);
    auto thread =
        std::make_unique<std::thread>(&MessageLoop::Run, message_loop.get());

    threads_.push_back({std::move(message_loop), std::move(thread)});
  }

  std::weak_ptr<MessagePump> weak_message_pump = message_pump;
  task_runner_ = TaskRunnerImpl::Create(
      std::move(weak_message_pump),
      DelayedTaskManagerSharedInstance::GetOrCreateSharedInstance());
}

void ThreadPool::Stop() {
  for (auto& thread : threads_) {
    thread.message_loop->Stop({});
    thread.thread->join();
  }
  threads_.clear();
}

void Stop(SourceLocation location, OnceClosure last_task);

std::shared_ptr<TaskRunner> ThreadPool::GetTaskRunner() const {
  return task_runner_;
}

std::shared_ptr<SequencedTaskRunner> ThreadPool::CreateSequencedTaskRunner() {
  return SequencedTaskRunnerImpl::Create(
      pump_, detail::SequenceIdGenerator::GetNextSequenceId(),
      DelayedTaskManagerSharedInstance::GetOrCreateSharedInstance());
}

std::shared_ptr<SingleThreadTaskRunner>
ThreadPool::CreateSingleThreadTaskRunner() {
  std::uniform_int_distribution<MessagePump::ExecutorId>
      executor_id_distribution(0, initial_size_ - 1);
  const MessagePump::ExecutorId allowed_executor_id =
      executor_id_distribution(random_generator_);

  return SingleThreadTaskRunnerImpl::Create(
      pump_, detail::SequenceIdGenerator::GetNextSequenceId(),
      allowed_executor_id,
      DelayedTaskManagerSharedInstance::GetOrCreateSharedInstance());
}

}  // namespace base
