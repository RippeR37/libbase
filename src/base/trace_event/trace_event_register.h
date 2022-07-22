#pragma once

#ifndef LIBBASE_TRACING_DISABLE

#include <fstream>
#include <list>
#include <mutex>
#include <string>

#include "base/time/time.h"
#include "base/trace_event/trace_events.h"
#include "base/trace_event/trace_json_writer.h"
#include "base/trace_event/trace_platform.h"

namespace base {
namespace detail {

class EventRegister {
 public:
  template <typename... Args>
  static void RegisterEvent(std::string categories,
                            std::string name,
                            char phase,
                            Args&&... args) {
    const char* const kEmptyId = "";
    GetInstance().PushGenericEvent(
        std::move(categories), std::move(name), kEmptyId, phase,
        ArgumentPacker::PackStringArguments(std::forward<Args>(args)...));
  }

  template <typename... Args>
  static void RegisterAsyncEvent(std::string categories,
                                 std::string name,
                                 void* id,
                                 char phase,
                                 Args&&... args) {
    GetInstance().PushGenericEvent(
        std::move(categories), std::move(name),
        std::to_string(reinterpret_cast<uintptr_t>(id)), phase,
        ArgumentPacker::PackStringArguments(std::forward<Args>(args)...));
  }

  template <typename... Args>
  static void RegisterCompleteEvent(std::string categories,
                                    std::string name,
                                    uint64_t duration,
                                    Args&&... args) {
    GetInstance().PushCompleteEvent(
        std::move(categories), std::move(name), duration,
        ArgumentPacker::PackStringArguments(std::forward<Args>(args)...));
  }

  template <typename... Args>
  static void RegisterCounter(std::string categories,
                              std::string name,
                              Args&&... args) {
    GetInstance().PushCounter(
        std::move(categories), std::move(name),
        ArgumentPacker::PackStringArguments(std::forward<Args>(args)...));
  }

  template <typename... Args>
  static void RegisterCounterId(std::string categories,
                                std::string name,
                                void* id,
                                Args&&... args) {
    GetInstance().PushCounterId(
        std::move(categories), std::move(name),
        std::to_string(reinterpret_cast<uintptr_t>(id)),
        ArgumentPacker::PackStringArguments(std::forward<Args>(args)...));
  }

  template <typename... Args>
  static void RegisterInstantEvent(std::string categories,
                                   std::string name,
                                   char scope,
                                   Args&&... args) {
    GetInstance().PushInstantEvent(
        std::move(categories), std::move(name), std::move(scope),
        ArgumentPacker::PackStringArguments(std::forward<Args>(args)...));
  }

  template <typename... Args>
  static void FlushEventsToFile(std::string file) {
    std::ofstream file_stream{file, std::ofstream::out};
    GetInstance().FlushAllEvents(file_stream);
  }

  template <typename... Args>
  static void FlushEventsToStream(std::ostream& stream) {
    GetInstance().FlushAllEvents(stream);
  }

 private:
  static EventRegister& GetInstance() {
    static EventRegister instance;
    return instance;
  }

  uint64_t GetTs() {
    static const auto origin = base::Time::Now();
    return (base::Time::Now() - origin).InMicroseconds();
  }

  uint64_t GetPid() { return TracePlatform::GetPid(); }
  uint64_t GetTid() { return TracePlatform::GetTid(); }

  void PushGenericEvent(std::string categories,
                        std::string name,
                        std::string id,
                        char phase,
                        TraceEvent::Arguments args) {
    std::lock_guard<std::mutex> guard{mutex_};
    events_.push_back({std::move(name), std::move(categories), std::move(id),
                       phase, GetTs(), GetPid(), GetTid(), args});
  }

  void PushCompleteEvent(std::string categories,
                         std::string name,
                         uint64_t duration,
                         TraceCompleteEvent::Arguments args) {
    const auto now = GetTs();
    if (now < duration) {
      return;
    }
    const auto ts = now - duration;

    std::lock_guard<std::mutex> guard{mutex_};
    complete_events_.push_back({std::move(name), std::move(categories), ts,
                                duration, GetPid(), GetTid(), std::move(args)});
  }

  void PushCounter(std::string categories,
                   std::string name,
                   TraceCounter::Arguments args) {
    std::lock_guard<std::mutex> guard{mutex_};
    counter_events_.push_back({std::move(name), std::move(categories), GetTs(),
                               GetPid(), std::move(args)});
  }

  void PushCounterId(std::string categories,
                     std::string name,
                     std::string id,
                     TraceCounterId::Arguments args) {
    std::lock_guard<std::mutex> guard{mutex_};
    counter_id_events_.push_back({std::move(name), std::move(categories),
                                  std::move(id), GetTs(), GetPid(),
                                  std::move(args)});
  }

  void PushInstantEvent(std::string categories,
                        std::string name,
                        char scope,
                        TraceInstantEvent::Arguments args) {
    std::lock_guard<std::mutex> guard{mutex_};
    instant_events_.push_back({std::move(name), std::move(categories),
                               std::move(scope), GetTs(), GetPid(), GetTid(),
                               std::move(args)});
  }

  void FlushAllEvents(std::ostream& stream) {
    std::lock_guard<std::mutex> guard{mutex_};

    JsonWriter::WriteAll(stream, events_, complete_events_, counter_events_,
                         counter_id_events_, instant_events_);

    events_ = std::list<TraceEvent>{};
    complete_events_ = std::list<TraceCompleteEvent>{};
    counter_events_ = std::list<TraceCounter>{};
    counter_id_events_ = std::list<TraceCounterId>{};
    instant_events_ = std::list<TraceInstantEvent>{};
  }

  std::mutex mutex_;
  std::list<TraceEvent> events_;
  std::list<TraceCompleteEvent> complete_events_;
  std::list<TraceCounter> counter_events_;
  std::list<TraceCounterId> counter_id_events_;
  std::list<TraceInstantEvent> instant_events_;
};

}  // namespace detail
}  // namespace base

#endif
