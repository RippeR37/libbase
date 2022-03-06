#pragma once

namespace base {

template <typename Signature>
class OnceCallback;

template <typename Signature>
class RepeatingCallback;

using OnceClosure = OnceCallback<void()>;
using RepeatingClosure = RepeatingCallback<void()>;

}  // namespace base
