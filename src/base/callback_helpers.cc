#include "base/callback_helpers.h"

namespace base {

ScopedClosureRunner::ScopedClosureRunner() = default;

ScopedClosureRunner::ScopedClosureRunner(OnceClosure closure)
    : closure_(std::move(closure)) {}

ScopedClosureRunner::ScopedClosureRunner(ScopedClosureRunner&& other)
    : closure_(std::move(other.closure_)) {}

ScopedClosureRunner::~ScopedClosureRunner() {
  RunAndReset();
}

ScopedClosureRunner& ScopedClosureRunner::operator=(
    ScopedClosureRunner&& other) {
  if (this != &other) {
    RunAndReset();
    ReplaceClosure(other.Release());
  }
  return *this;
}

ScopedClosureRunner::operator bool() const {
  return !!closure_;
}

void ScopedClosureRunner::RunAndReset() {
  if (closure_) {
    std::move(closure_).Run();
  }
}

void ScopedClosureRunner::ReplaceClosure(OnceClosure new_closure) {
  closure_ = std::move(new_closure);
}

OnceClosure ScopedClosureRunner::Release() {
  return std::move(closure_);
}

}  // namespace base
