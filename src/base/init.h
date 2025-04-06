#pragma once

namespace base {

struct InitOptions {
  // Logging
  bool LogToStderr = true;
};

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
void Initialize(int argc, char* argv[], InitOptions options);
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
void InitializeForTests(int argc, char* argv[], InitOptions options);
void Deinitialize();

}  // namespace base
