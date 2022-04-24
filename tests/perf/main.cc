#include "base/init.h"
#include "benchmark/benchmark.h"

// Implementation should be in-sync with the logic specified in
// `BENCHMARK_MAIN()` macro with `base` module initialized at the beginning
// and deinitialized at the end.
int main(int argc, char** argv) {
  base::Initialize(argc, argv);

  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc, argv)) {
    return 1;
  }
  ::benchmark::RunSpecifiedBenchmarks();
  ::benchmark::Shutdown();

  base::Deinitialize();
  return 0;
}
