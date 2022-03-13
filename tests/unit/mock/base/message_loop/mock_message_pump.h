#pragma once

#include "base/message_loop/message_pump.h"

#include "gmock/gmock.h"

#include "base/callback.h"

class MockMessagePump : public base::MessagePump {
 public:
  // MessagePump
  MOCK_METHOD(PendingTask, GetNextPendingTask, (ExecutorId), (override));
  MOCK_METHOD(bool, QueuePendingTask, (PendingTask), (override));
  MOCK_METHOD(void, Stop, (PendingTask), (override));
};
