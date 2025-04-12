#pragma once

#if defined(LIBBASE_IS_WINDOWS)

#if defined(NOMINMAX)

#include <Windows.h>

#else  // defined(NOMINMAX)

#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX

#endif  // !defined(NOMINMAX)

#endif  // defined(LIBBASE_IS_WINDOWS)
