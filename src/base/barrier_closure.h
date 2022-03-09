#pragma once

#include <cstddef>

#include "base/callback.h"

namespace base {

RepeatingClosure BarrierClosure(size_t required_run_count,
                                OnceClosure callback);

}  // namespace base
