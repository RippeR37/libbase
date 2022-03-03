#include "benchmark/benchmark.h"

#include "base/bind.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"

namespace {

void TestSingleThreaded(base::SequencedTaskRunner* tr,
                        int count,
                        base::WaitableEvent* event) {
  if (count > 0) {
    tr->PostTask(FROM_HERE,
                 base::BindOnce(&TestSingleThreaded, tr, count - 1, event));
  } else {
    event->Signal();
  }
}

void TestDoubleThreaded(base::SequencedTaskRunner* tr1,
                        base::SequencedTaskRunner* tr2,
                        int count,
                        base::WaitableEvent* event) {
  if (count > 0) {
    tr1->PostTask(FROM_HERE, base::BindOnce(&TestDoubleThreaded, tr2, tr1,
                                            count - 1, event));
  } else {
    event->Signal();
  }
}

void BM_TestSingleThreaded(benchmark::State& state) {
  base::Thread t1;
  t1.Start();
  base::WaitableEvent event{base::WaitableEvent::ResetPolicy::kAutomatic};

  for (auto _ : state) {
    TestSingleThreaded(t1.TaskRunner().get(), 1000, &event);
    event.Wait();
  }
}

void BM_TestDoubleThreaded(benchmark::State& state) {
  base::Thread t1;
  base::Thread t2;
  t1.Start();
  t2.Start();
  base::WaitableEvent event{base::WaitableEvent::ResetPolicy::kAutomatic};

  for (auto _ : state) {
    TestDoubleThreaded(t2.TaskRunner().get(), t1.TaskRunner().get(), 1000,
                       &event);
    event.Wait();
  }
}

#define LIBBASE_BENCHMARK(x)                                                  \
  BENCHMARK(x)                                                                \
      ->Unit(::benchmark::TimeUnit::kMillisecond)                             \
      ->Repetitions(10)                                                       \
      ->ComputeStatistics(                                                    \
          "min",                                                              \
          [](auto& values) {                                                  \
            return *(std::min_element(std::begin(values), std::end(values))); \
          })                                                                  \
      ->ComputeStatistics(                                                    \
          "max",                                                              \
          [](auto& values) {                                                  \
            return *(std::max_element(std::begin(values), std::end(values))); \
          })                                                                  \
      ->DisplayAggregatesOnly()

LIBBASE_BENCHMARK(BM_TestSingleThreaded);
LIBBASE_BENCHMARK(BM_TestDoubleThreaded);

#undef LIBBASE_BENCHMARK

}  // namespace
