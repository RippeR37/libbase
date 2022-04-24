#pragma once

#include "base/sequenced_task_runner.h"

#include "gmock/gmock.h"

class MockSequencedTaskRunner : public base::SequencedTaskRunner {
 public:
  // SequencedTaskRunner
  MOCK_METHOD(bool,
              PostDelayedTask,
              (base::SourceLocation, base::OnceClosure, base::TimeDelta),
              (override));
  MOCK_METHOD(bool, RunsTasksInCurrentSequence, (), (const override));
};
