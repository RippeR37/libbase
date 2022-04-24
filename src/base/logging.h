#pragma once

#ifdef NOMINMAX

#include "glog/logging.h"

#else  // #ifdef NOMINMAX

// Define and undefine `NOMINMAX` to ensure that min()/max() macros are not
// leaked by glog's implementation by mistake.
#define NOMINMAX
#include "glog/logging.h"
#undef NOMINMAX

#endif  // #ifdef NOMINMAX
