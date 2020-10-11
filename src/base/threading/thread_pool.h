#pragma once

#include <memory>
#include <random>
#include <vector>

#include "base/single_thread_task_runner.h"

namespace base {

class MessagePump;

class ThreadPool {
 public:
  ThreadPool(size_t initial_size);
  ~ThreadPool();

  void Start();
  void Join();

  std::shared_ptr<TaskRunner> GetTaskRunner() const;
  std::shared_ptr<SequencedTaskRunner> CreateSequencedTaskRunner();
  std::shared_ptr<SingleThreadTaskRunner> CreateSingleThreadTaskRunner();

 private:
  struct ThreadData;

  const size_t initial_size_;
  std::weak_ptr<MessagePump> pump_;
  std::vector<ThreadData> threads_;
  std::shared_ptr<TaskRunner> task_runner_;
  std::mt19937 random_generator_;
};

}  // namespace base
