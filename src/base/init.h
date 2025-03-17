#pragma once

namespace base {

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
void Initialize(int argc, char* argv[]);
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
void InitializeForTests(int argc, char* argv[]);
void Deinitialize();

}  // namespace base
