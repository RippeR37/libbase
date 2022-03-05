#include "base/bind.h"

#include "base/sequenced_task_runner_helpers.h"
#include "base/threading/thread.h"

#include "gtest/gtest.h"

namespace {

int global_int_arg = -1;

struct Counter {
  void add(int x) { sum += x; }

  int sum = 0;
};

void SetGlobalIntArg(int x) {
  global_int_arg = x;
}

void SetGlobalIntFromUniquePtrArg(std::unique_ptr<int> x) {
  global_int_arg = *x;
}

int add(int x, int y) {
  return x + y;
}

TEST(BindTest, FreeFunc) {
  global_int_arg = -1;

  auto cb = base::BindRepeating(&SetGlobalIntArg);
  cb.Run(11);
  EXPECT_EQ(global_int_arg, 11);
  cb.Run(12);
  EXPECT_EQ(global_int_arg, 12);
}

TEST(BindTest, Lambda) {
  global_int_arg = -1;

  auto cb = base::BindRepeating([](int x) { SetGlobalIntArg(x); });
  cb.Run(11);
  EXPECT_EQ(global_int_arg, 11);
  cb.Run(12);
  EXPECT_EQ(global_int_arg, 12);
}

TEST(BindTest, FreeFuncWithArgs) {
  global_int_arg = -1;

  auto cb = base::BindRepeating(&SetGlobalIntArg, 15);
  cb.Run();
  EXPECT_EQ(global_int_arg, 15);
  global_int_arg = -1;
  cb.Run();
  EXPECT_EQ(global_int_arg, 15);
}

TEST(BindTest, LambdaWithArgs) {
  global_int_arg = -1;

  auto cb = base::BindRepeating([](int x) { SetGlobalIntArg(x); }, 15);
  cb.Run();
  EXPECT_EQ(global_int_arg, 15);
  global_int_arg = -1;
  cb.Run();
  EXPECT_EQ(global_int_arg, 15);
}

TEST(BindTest, ClassMethod) {
  Counter counter;
  auto cb = base::BindRepeating(&Counter::add, base::Unretained(&counter));
  ASSERT_EQ(counter.sum, 0);
  cb.Run(7);
  EXPECT_EQ(counter.sum, 7);
  cb.Run(3);
  EXPECT_EQ(counter.sum, 10);
}

TEST(BindTest, ClassMethodWithArgs) {
  Counter counter;
  auto cb = base::BindRepeating(&Counter::add, base::Unretained(&counter), 21);
  ASSERT_EQ(counter.sum, 0);
  cb.Run();
  EXPECT_EQ(counter.sum, 21);
  cb.Run();
  EXPECT_EQ(counter.sum, 42);
}

TEST(BindTest, MultiArgFreeFunc) {
  auto cb = base::BindRepeating(&add);
  EXPECT_EQ(cb.Run(11, -3), 8);
  EXPECT_EQ(cb.Run(3, -11), -8);
  EXPECT_EQ(cb.Run(0, -11), -11);
}

TEST(BindTest, MultiArgFreeFuncWithSomeArgs) {
  auto cb = base::BindRepeating(&add, 3);
  EXPECT_EQ(cb.Run(11), 14);
  EXPECT_EQ(cb.Run(-3), 0);
  EXPECT_EQ(cb.Run(0), 3);
}

TEST(BindOnceTest, FreeFunc) {
  global_int_arg = -1;

  auto cb = base::BindOnce(&SetGlobalIntArg);
  std::move(cb).Run(11);
  EXPECT_EQ(global_int_arg, 11);
  EXPECT_FALSE(cb);
}

TEST(BindOnceTest, Lambda) {
  global_int_arg = -1;

  auto cb = base::BindOnce([](int x) { SetGlobalIntArg(x); });
  std::move(cb).Run(11);
  EXPECT_EQ(global_int_arg, 11);
  EXPECT_FALSE(cb);
}

TEST(BindOnceTest, FreeFuncWithArgs) {
  global_int_arg = -1;

  auto cb = base::BindOnce(&SetGlobalIntArg, 15);
  std::move(cb).Run();
  EXPECT_EQ(global_int_arg, 15);
  EXPECT_FALSE(cb);
}

TEST(BindOnceTest, LambdaWithArgs) {
  global_int_arg = -1;

  auto cb = base::BindOnce([](int x) { SetGlobalIntArg(x); }, 15);
  std::move(cb).Run();
  EXPECT_EQ(global_int_arg, 15);
  EXPECT_FALSE(cb);
}

TEST(BindOnceTest, ClassMethod) {
  Counter counter;
  auto cb = base::BindOnce(&Counter::add, base::Unretained(&counter));
  ASSERT_EQ(counter.sum, 0);
  std::move(cb).Run(7);
  EXPECT_EQ(counter.sum, 7);
  EXPECT_FALSE(cb);
}

TEST(BindOnceTest, ClassMethodWithArgs) {
  Counter counter;
  auto cb = base::BindOnce(&Counter::add, base::Unretained(&counter), 21);
  ASSERT_EQ(counter.sum, 0);
  std::move(cb).Run();
  EXPECT_EQ(counter.sum, 21);
  EXPECT_FALSE(cb);
}

TEST(BindOnceTest, MultiArgFreeFunc) {
  auto cb = base::BindOnce(&add);
  EXPECT_EQ(std::move(cb).Run(11, -3), 8);
  EXPECT_FALSE(cb);
}

TEST(BindOnceTest, MultiArgFreeFuncWithSomeArgs) {
  auto cb = base::BindOnce(&add, 3);
  EXPECT_EQ(std::move(cb).Run(11), 14);
  EXPECT_FALSE(cb);
}

TEST(BindOnceTest, BindUniquePtrAndMove) {
  global_int_arg = -1;

  auto cb_1 =
      base::BindOnce(&SetGlobalIntFromUniquePtrArg, std::make_unique<int>(3));
  auto cb_2 = std::move(cb_1);
  auto cb_3 = base::BindOnce(std::move(cb_2));

  EXPECT_FALSE(cb_1);
  EXPECT_FALSE(cb_2);
  ASSERT_TRUE(cb_3);

  std::move(cb_3).Run();
  EXPECT_EQ(global_int_arg, 3);
}

TEST(AdvancedBindTest, RepeatingToRepeatingCallbackToFreeFunction) {
  auto cb_1 = base::BindRepeating(&add);
  auto cb_2 = base::BindRepeating(cb_1);
  EXPECT_EQ(cb_1.Run(3, 7), 10);
  EXPECT_EQ(cb_1.Run(3, 7), cb_2.Run(3, 7));

  auto cb_3 = base::BindRepeating(cb_2, 1);
  EXPECT_EQ(cb_3.Run(0), 1);
  EXPECT_EQ(cb_3.Run(-3), -2);

  auto cb_4 = base::BindRepeating(cb_3, 3);
  EXPECT_EQ(cb_4.Run(), 4);
  EXPECT_EQ(cb_4.Run(), 4);
}

TEST(AdvancedBindTest, OnceToRepeatingCallbackToFreeFunction) {
  auto cb_1 = base::BindRepeating(&add);
  auto cb_2 = base::BindOnce(cb_1);
  EXPECT_EQ(cb_1.Run(3, 7), 10);
  EXPECT_EQ(std::move(cb_2).Run(3, 7), 10);
  EXPECT_FALSE(cb_2);

  auto cb_3 = base::BindOnce(cb_1);
  EXPECT_EQ(std::move(cb_3).Run(3, 7), 10);
  EXPECT_TRUE(cb_1);
  EXPECT_FALSE(cb_3);

  auto cb_4 = base::BindOnce(cb_1, 1);
  EXPECT_EQ(std::move(cb_4).Run(3), 4);
  EXPECT_TRUE(cb_1);
  EXPECT_FALSE(cb_4);

  auto cb_5 = base::BindRepeating(cb_1, 1);
  auto cb_6 = base::BindOnce(cb_5, 3);
  EXPECT_EQ(std::move(cb_6).Run(), 4);
  EXPECT_TRUE(cb_5);
  EXPECT_FALSE(cb_6);
}

TEST(AdvancedBindTest, OnceToOnceCallbackToFreeFunction) {
  auto cb_1 = base::BindOnce(&add);
  auto cb_2 = base::BindOnce(std::move(cb_1));
  EXPECT_EQ(std::move(cb_2).Run(3, 7), 10);
  EXPECT_FALSE(cb_1);
  EXPECT_FALSE(cb_2);

  auto cb_3 = base::BindOnce(&add);
  auto cb_4 = base::BindOnce(std::move(cb_3), 1);
  EXPECT_EQ(std::move(cb_4).Run(3), 4);
  EXPECT_FALSE(cb_3);
  EXPECT_FALSE(cb_4);

  auto cb_5 = base::BindOnce(&add);
  auto cb_6 = base::BindOnce(std::move(cb_5), 1);
  auto cb_7 = base::BindOnce(std::move(cb_6), 3);
  EXPECT_EQ(std::move(cb_7).Run(), 4);
  EXPECT_FALSE(cb_5);
  EXPECT_FALSE(cb_6);
  EXPECT_FALSE(cb_7);
}

int ReturnCopyOfConstRefArgument(const int& value) {
  return value;
}

int ReturnCopyAndIncrementRefArgument(int& value) {
  return value++;
}

TEST(WrapUnwrapTests, BindConstRefArgument) {
  int n = 0;
  auto callback = base::BindRepeating(&ReturnCopyOfConstRefArgument, n);
  EXPECT_EQ(callback.Run(), 0);
  n = 1;
  EXPECT_EQ(callback.Run(), 0);
}

TEST(WrapUnwrapTests, BindConstRefArgumentWithReferenceWrapper) {
  int n = 0;
  auto callback =
      base::BindRepeating(&ReturnCopyOfConstRefArgument, std::ref(n));
  EXPECT_EQ(callback.Run(), 0);
  n = 1;
  EXPECT_EQ(callback.Run(), 1);
}

TEST(WrapUnwrapTests, BindNonConstRefArgumentWithReferenceWrapper) {
  int n = 0;
  auto callback =
      base::BindRepeating(&ReturnCopyAndIncrementRefArgument, std::ref(n));
  EXPECT_EQ(callback.Run(), 0);
  EXPECT_EQ(n, 1);
  EXPECT_EQ(callback.Run(), 1);
  EXPECT_EQ(n, 2);
}

class BindWrappedHelper {
 public:
  BindWrappedHelper(int* call_count, bool* destroyed_flag)
      : call_count_(call_count), destroyed_flag_(destroyed_flag) {
    EXPECT_TRUE(call_count_);
    EXPECT_EQ(*call_count, 0);
    EXPECT_TRUE(destroyed_flag_);
    EXPECT_FALSE(*destroyed_flag_);
  }

  ~BindWrappedHelper() { *destroyed_flag_ = true; }

  int Call() { return ++(*call_count_); }

 private:
  int* call_count_;
  bool* destroyed_flag_;
};

TEST(WrapUnwrapTests, BindOnceOwnedMemberCalled) {
  int call_count = 0;
  bool destroyed = false;

  auto object = std::make_unique<BindWrappedHelper>(&call_count, &destroyed);
  auto callback =
      base::BindOnce(&BindWrappedHelper::Call, base::Owned(object.release()));

  EXPECT_FALSE(destroyed);
  EXPECT_EQ(std::move(callback).Run(), 1);
  EXPECT_EQ(call_count, 1);
  EXPECT_TRUE(destroyed);
}

TEST(WrapUnwrapTests, BindOnceOwnedMemberDestroyed) {
  int call_count = 0;
  bool destroyed = false;

  auto object = std::make_unique<BindWrappedHelper>(&call_count, &destroyed);

  EXPECT_FALSE(destroyed);
  {
    auto callback =
        base::BindOnce(&BindWrappedHelper::Call, base::Owned(object.release()));
  }
  EXPECT_EQ(call_count, 0);
  EXPECT_TRUE(destroyed);
}

TEST(WrapUnwrapTests, BindRepeatingMemberOwned) {
  int call_count = 0;
  bool destroyed = false;

  auto object = std::make_unique<BindWrappedHelper>(&call_count, &destroyed);
  auto callback = base::BindRepeating(&BindWrappedHelper::Call,
                                      base::Owned(std::move(object)));

  EXPECT_EQ(callback.Run(), 1);
  EXPECT_EQ(call_count, 1);

  auto callback_copy = callback;

  EXPECT_EQ(callback_copy.Run(), 2);
  EXPECT_EQ(call_count, 2);

  EXPECT_FALSE(destroyed);
  callback = base::RepeatingCallback<int()>{};
  EXPECT_FALSE(destroyed);
  callback_copy = base::RepeatingCallback<int()>{};
  EXPECT_TRUE(destroyed);
}

int CallBindWrapperHelperCall(BindWrappedHelper* helper) {
  return helper->Call();
}

TEST(WrapUnwrapTests, BindOnceOwnedFreeCalled) {
  int call_count = 0;
  bool destroyed = false;

  auto object = std::make_unique<BindWrappedHelper>(&call_count, &destroyed);
  auto callback =
      base::BindOnce(&CallBindWrapperHelperCall, base::Owned(object.release()));

  EXPECT_FALSE(destroyed);
  EXPECT_EQ(std::move(callback).Run(), 1);
  EXPECT_EQ(call_count, 1);
  EXPECT_TRUE(destroyed);
}

TEST(WrapUnwrapTests, BindOnceOwnedFreeDestroyed) {
  int call_count = 0;
  bool destroyed = false;

  auto object = std::make_unique<BindWrappedHelper>(&call_count, &destroyed);

  EXPECT_FALSE(destroyed);
  {
    auto callback = base::BindOnce(&CallBindWrapperHelperCall,
                                   base::Owned(object.release()));
  }
  EXPECT_EQ(call_count, 0);
  EXPECT_TRUE(destroyed);
}

TEST(WrapUnwrapTests, BindRepeatingFreeOwned) {
  int call_count = 0;
  bool destroyed = false;

  auto object = std::make_unique<BindWrappedHelper>(&call_count, &destroyed);
  auto callback = base::BindRepeating(&CallBindWrapperHelperCall,
                                      base::Owned(std::move(object)));

  EXPECT_EQ(callback.Run(), 1);
  EXPECT_EQ(call_count, 1);

  auto callback_copy = callback;

  EXPECT_EQ(callback_copy.Run(), 2);
  EXPECT_EQ(call_count, 2);

  EXPECT_FALSE(destroyed);
  callback = base::RepeatingCallback<int()>{};
  EXPECT_FALSE(destroyed);
  callback_copy = base::RepeatingCallback<int()>{};
  EXPECT_TRUE(destroyed);
}

int CallBindWrapperHelperCallRef(BindWrappedHelper& helper) {
  return helper.Call();
}

TEST(WrapUnwrapTests, BindRepeatingOwnedRef) {
  int call_count = 0;
  bool destroyed = false;

  BindWrappedHelper object{&call_count, &destroyed};
  auto callback = base::BindRepeating(&CallBindWrapperHelperCallRef,
                                      base::OwnedRef(std::move(object)));

  EXPECT_FALSE(destroyed);
  EXPECT_EQ(call_count, 0);

  EXPECT_EQ(callback.Run(), 1);
  EXPECT_EQ(call_count, 1);
  EXPECT_FALSE(destroyed);

  callback = base::RepeatingCallback<int()>{};
  EXPECT_EQ(call_count, 1);
  EXPECT_TRUE(destroyed);
}

TEST(BindToSharedPtrTest, BindOnce) {
  int call_count = 0;
  bool destroyed = false;

  auto object = std::make_shared<BindWrappedHelper>(&call_count, &destroyed);
  auto callback =
      base::BindOnce(&BindWrappedHelper::Call, base::RetainedRef(object));

  object.reset();
  EXPECT_EQ(call_count, 0);
  EXPECT_FALSE(destroyed);

  EXPECT_EQ(std::move(callback).Run(), 1);
  EXPECT_EQ(call_count, 1);
  EXPECT_TRUE(destroyed);
}

TEST(BindToSharedPtrTest, BindRepeating) {
  int call_count = 0;
  bool destroyed = false;

  auto object = std::make_shared<BindWrappedHelper>(&call_count, &destroyed);
  auto callback1 =
      base::BindRepeating(&BindWrappedHelper::Call, base::RetainedRef(object));
  auto callback2 = callback1;

  object.reset();
  EXPECT_EQ(call_count, 0);
  EXPECT_FALSE(destroyed);

  EXPECT_EQ(callback1.Run(), 1);
  EXPECT_EQ(call_count, 1);
  EXPECT_FALSE(destroyed);

  EXPECT_EQ(callback2.Run(), 2);
  EXPECT_EQ(call_count, 2);
  EXPECT_FALSE(destroyed);

  EXPECT_EQ(callback1.Run(), 3);
  EXPECT_EQ(call_count, 3);
  EXPECT_FALSE(destroyed);

  EXPECT_EQ(std::move(callback2).Run(), 4);
  EXPECT_EQ(call_count, 4);
  EXPECT_FALSE(destroyed);

  EXPECT_EQ(std::move(callback1).Run(), 5);
  EXPECT_EQ(call_count, 5);
  EXPECT_TRUE(destroyed);
}

template <typename ReturnType>
class MemberFunctions {
 public:
  ReturnType Function01() {
    value |= (1 << 0);
    return static_cast<ReturnType>(value);
  }
  ReturnType Function02() const {
    value |= (1 << 1);
    return static_cast<ReturnType>(value);
  }
  ReturnType Function03() volatile {
    value |= (1 << 2);
    return static_cast<ReturnType>(value);
  }
  ReturnType Function04() const volatile {
    value |= (1 << 3);
    return static_cast<ReturnType>(value);
  }

  ReturnType Function05() & {
    value |= (1 << 4);
    return static_cast<ReturnType>(value);
  }
  ReturnType Function06() const& {
    value |= (1 << 5);
    return static_cast<ReturnType>(value);
  }
  ReturnType Function07() volatile& {
    value |= (1 << 6);
    return static_cast<ReturnType>(value);
  }
  ReturnType Function08() const volatile& {
    value |= (1 << 7);
    return static_cast<ReturnType>(value);
  }
  ReturnType Function09() && {
    value |= (1 << 8);
    return static_cast<ReturnType>(value);
  }
  ReturnType Function10() const&& {
    value |= (1 << 9);
    return static_cast<ReturnType>(value);
  }
  ReturnType Function11() volatile&& {
    value |= (1 << 10);
    return static_cast<ReturnType>(value);
  }
  ReturnType Function12() const volatile&& {
    value |= (1 << 11);
    return static_cast<ReturnType>(value);
  }

  ReturnType Function13() noexcept {
    value |= (1 << 12);
    return static_cast<ReturnType>(value);
  }
  ReturnType Function14() const noexcept {
    value |= (1 << 13);
    return static_cast<ReturnType>(value);
  }
  ReturnType Function15() volatile noexcept {
    value |= (1 << 14);
    return static_cast<ReturnType>(value);
  }
  ReturnType Function16() const volatile noexcept {
    value |= (1 << 15);
    return static_cast<ReturnType>(value);
  }

  ReturnType Function17() & noexcept {
    value |= (1 << 16);
    return static_cast<ReturnType>(value);
  }
  ReturnType Function18() const& noexcept {
    value |= (1 << 17);
    return static_cast<ReturnType>(value);
  }
  ReturnType Function19() volatile& noexcept {
    value |= (1 << 18);
    return static_cast<ReturnType>(value);
  }
  ReturnType Function20() const volatile& noexcept {
    value |= (1 << 19);
    return static_cast<ReturnType>(value);
  }
  ReturnType Function21() && noexcept {
    value |= (1 << 20);
    return static_cast<ReturnType>(value);
  }
  ReturnType Function22() const&& noexcept {
    value |= (1 << 21);
    return static_cast<ReturnType>(value);
  }
  ReturnType Function23() volatile&& noexcept {
    value |= (1 << 22);
    return static_cast<ReturnType>(value);
  }
  ReturnType Function24() const volatile&& noexcept {
    value |= (1 << 23);
    return static_cast<ReturnType>(value);
  }

  mutable int value = 0;
};

TEST(MemberFunctionsExhaustionTest, ExhaustionTest) {
  MemberFunctions<void> obj;

  base::BindOnce(&MemberFunctions<void>::Function01, base::Unretained(&obj))
      .Run();
  base::BindOnce(&MemberFunctions<void>::Function03, base::Unretained(&obj))
      .Run();
  base::BindOnce(&MemberFunctions<void>::Function02, base::Unretained(&obj))
      .Run();
  base::BindOnce(&MemberFunctions<void>::Function04, base::Unretained(&obj))
      .Run();

  base::BindOnce(&MemberFunctions<void>::Function05, base::Unretained(&obj))
      .Run();
  base::BindOnce(&MemberFunctions<void>::Function06, base::Unretained(&obj))
      .Run();
  base::BindOnce(&MemberFunctions<void>::Function07, base::Unretained(&obj))
      .Run();
  base::BindOnce(&MemberFunctions<void>::Function08, base::Unretained(&obj))
      .Run();
  // Functions 09-12 are non-bindable to pointers since pointers cannot transfer
  // r-valueness

  base::BindOnce(&MemberFunctions<void>::Function13, base::Unretained(&obj))
      .Run();
  base::BindOnce(&MemberFunctions<void>::Function14, base::Unretained(&obj))
      .Run();
  base::BindOnce(&MemberFunctions<void>::Function15, base::Unretained(&obj))
      .Run();
  base::BindOnce(&MemberFunctions<void>::Function16, base::Unretained(&obj))
      .Run();

  base::BindOnce(&MemberFunctions<void>::Function17, base::Unretained(&obj))
      .Run();
  base::BindOnce(&MemberFunctions<void>::Function18, base::Unretained(&obj))
      .Run();
  base::BindOnce(&MemberFunctions<void>::Function19, base::Unretained(&obj))
      .Run();
  base::BindOnce(&MemberFunctions<void>::Function20, base::Unretained(&obj))
      .Run();
  // Functions 21-24 are non-bindable to pointers since pointers cannot transfer
  // r-valueness

  EXPECT_EQ(obj.value, 0b00001111'1111'00001111'1111);
}

class WeakClass {
 public:
  WeakClass() : value_(0u), weak_factory_(this) {}

  base::WeakPtr<WeakClass> GetWeakPtr() const {
    return weak_factory_.GetWeakPtr();
  }
  void InvalidateWeakPtrs() { weak_factory_.InvalidateWeakPtrs(); }

  size_t GetValue() { return value_; }
  void IncrementValue() { ++value_; }
  void IncrementBy(size_t increment_value) { value_ += increment_value; }
  size_t IncrementAndGetValue() { return ++value_; }

  void VerifyExpectation(size_t expected_value) {
    EXPECT_EQ(GetValue(), expected_value);
  }

 private:
  size_t value_;
  base::WeakPtrFactory<WeakClass> weak_factory_;
};

class WeakCallbackTest : public ::testing::Test {
 public:
  WeakCallbackTest()
      : sequence_id_setter_(
            base::detail::SequenceIdGenerator::GetNextSequenceId()) {}

 protected:
  // We need to emulate that we're on some sequence for DCHECKs to work
  // correctly.
  base::detail::ScopedSequenceIdSetter sequence_id_setter_;
  WeakClass weak_object_;
};

TEST_F(WeakCallbackTest, EmptyWeakOnceCallback) {
  base::WeakPtr<WeakClass> weak_object{nullptr};
  auto callback = base::BindOnce(&WeakClass::IncrementValue, weak_object);

  EXPECT_EQ(weak_object_.GetValue(), 0u);
  std::move(callback).Run();
  EXPECT_EQ(weak_object_.GetValue(), 0u);
}

TEST_F(WeakCallbackTest, InvalidatedWeakOnceCallback) {
  auto callback =
      base::BindOnce(&WeakClass::IncrementValue, weak_object_.GetWeakPtr());

  EXPECT_EQ(weak_object_.GetValue(), 0u);
  weak_object_.InvalidateWeakPtrs();
  std::move(callback).Run();
  EXPECT_EQ(weak_object_.GetValue(), 0u);
}

TEST_F(WeakCallbackTest, ValidWeakOnceCallback) {
  auto callback =
      base::BindOnce(&WeakClass::IncrementValue, weak_object_.GetWeakPtr());

  EXPECT_EQ(weak_object_.GetValue(), 0u);
  std::move(callback).Run();
  EXPECT_EQ(weak_object_.GetValue(), 1u);
}

TEST_F(WeakCallbackTest, ValidWeakOnceCallbackWithArg) {
  const auto kIncrementByValue = 5u;
  auto callback = base::BindOnce(&WeakClass::IncrementBy,
                                 weak_object_.GetWeakPtr(), kIncrementByValue);

  EXPECT_EQ(weak_object_.GetValue(), 0u);
  std::move(callback).Run();
  EXPECT_EQ(weak_object_.GetValue(), kIncrementByValue);
}

TEST_F(WeakCallbackTest, EmptyWeakRepeatingCallback) {
  base::WeakPtr<WeakClass> weak_object{nullptr};
  auto callback = base::BindRepeating(&WeakClass::IncrementValue, weak_object);

  EXPECT_EQ(weak_object_.GetValue(), 0u);
  callback.Run();
  EXPECT_EQ(weak_object_.GetValue(), 0u);
  callback.Run();
  EXPECT_EQ(weak_object_.GetValue(), 0u);
  callback.Run();
  EXPECT_EQ(weak_object_.GetValue(), 0u);
}

TEST_F(WeakCallbackTest, InvalidatedWeakRepeatingCallback) {
  auto callback = base::BindRepeating(&WeakClass::IncrementValue,
                                      weak_object_.GetWeakPtr());

  EXPECT_EQ(weak_object_.GetValue(), 0u);
  weak_object_.InvalidateWeakPtrs();
  callback.Run();
  EXPECT_EQ(weak_object_.GetValue(), 0u);
  callback.Run();
  EXPECT_EQ(weak_object_.GetValue(), 0u);
  callback.Run();
  EXPECT_EQ(weak_object_.GetValue(), 0u);
}

TEST_F(WeakCallbackTest, ValidThenInvalidatedWeakRepeatingCallback) {
  auto callback = base::BindRepeating(&WeakClass::IncrementValue,
                                      weak_object_.GetWeakPtr());

  EXPECT_EQ(weak_object_.GetValue(), 0u);
  callback.Run();
  EXPECT_EQ(weak_object_.GetValue(), 1u);
  callback.Run();
  EXPECT_EQ(weak_object_.GetValue(), 2u);
  callback.Run();
  EXPECT_EQ(weak_object_.GetValue(), 3u);

  weak_object_.InvalidateWeakPtrs();

  callback.Run();
  EXPECT_EQ(weak_object_.GetValue(), 3u);
  callback.Run();
  EXPECT_EQ(weak_object_.GetValue(), 3u);
}

TEST_F(WeakCallbackTest, ValidThenInvalidatedWeakRepeatingCallbackWithArg) {
  const auto kIncrementByValue = 5u;
  auto callback = base::BindRepeating(
      &WeakClass::IncrementBy, weak_object_.GetWeakPtr(), kIncrementByValue);

  EXPECT_EQ(weak_object_.GetValue(), 0u);
  callback.Run();
  EXPECT_EQ(weak_object_.GetValue(), 1u * kIncrementByValue);
  callback.Run();
  EXPECT_EQ(weak_object_.GetValue(), 2u * kIncrementByValue);
  callback.Run();
  EXPECT_EQ(weak_object_.GetValue(), 3u * kIncrementByValue);

  weak_object_.InvalidateWeakPtrs();

  callback.Run();
  EXPECT_EQ(weak_object_.GetValue(), 3u * kIncrementByValue);
  callback.Run();
  EXPECT_EQ(weak_object_.GetValue(), 3u * kIncrementByValue);
}

class ThreadedWeakCallbackTest : public WeakCallbackTest {
 public:
  void SetUp() override { thread_.Start(); }
  void TearDown() override {
    thread_.FlushForTesting();
    thread_.Join();
  }

  std::shared_ptr<base::SequencedTaskRunner> TaskRunner() {
    return thread_.TaskRunner();
  }

  void VerifyExpectation(size_t expected_value) {
    EXPECT_TRUE(TaskRunner()->RunsTasksInCurrentSequence());
    EXPECT_EQ(weak_object_.GetValue(), expected_value);
  }

 protected:
  base::Thread thread_;
};

TEST_F(ThreadedWeakCallbackTest, UseWeakPtrOnAnotherSequence) {
  auto weak_ptr = weak_object_.GetWeakPtr();
  TaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&WeakClass::VerifyExpectation, weak_ptr, 0u));

  // Increment twice
  TaskRunner()->PostTask(FROM_HERE,
                         base::BindOnce(&WeakClass::IncrementValue, weak_ptr));
  TaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&WeakClass::VerifyExpectation, weak_ptr, 1u));
  TaskRunner()->PostTask(FROM_HERE,
                         base::BindOnce(&WeakClass::IncrementValue, weak_ptr));
  TaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&WeakClass::VerifyExpectation, weak_ptr, 2u));

  // Invalidate
  TaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&WeakClass::InvalidateWeakPtrs, weak_ptr));

  // Increment twice with no effect
  TaskRunner()->PostTask(FROM_HERE,
                         base::BindOnce(&WeakClass::IncrementValue, weak_ptr));
  TaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&WeakClass::VerifyExpectation, weak_ptr, 2u));
  TaskRunner()->PostTask(FROM_HERE,
                         base::BindOnce(&WeakClass::IncrementValue, weak_ptr));
  TaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&WeakClass::VerifyExpectation, weak_ptr, 2u));
}

class IgnoreResultTest : public ::testing::Test {};

int SetGlobalIntArgAndReturnIt(int x) {
  SetGlobalIntArg(x);
  return x;
}

TEST_F(IgnoreResultTest, BindOnceIgnoreResult) {
  global_int_arg = -1;

  base::OnceCallback<void(int)> callback =
      base::BindOnce(base::IgnoreResult(&SetGlobalIntArgAndReturnIt));
  ASSERT_TRUE(callback);
  std::move(callback).Run(3);
  EXPECT_FALSE(callback);
  EXPECT_EQ(global_int_arg, 3);
}

TEST_F(IgnoreResultTest, BindOnceIgnoreResultWithArgument) {
  global_int_arg = -1;

  base::OnceCallback<void()> callback =
      base::BindOnce(base::IgnoreResult(&SetGlobalIntArgAndReturnIt), 7);
  ASSERT_TRUE(callback);
  std::move(callback).Run();
  EXPECT_FALSE(callback);
  EXPECT_EQ(global_int_arg, 7);
}

TEST_F(IgnoreResultTest, BindRepeatingIgnoreResult) {
  global_int_arg = -1;

  base::RepeatingCallback<void(int)> callback =
      base::BindRepeating(base::IgnoreResult(&SetGlobalIntArgAndReturnIt));
  ASSERT_TRUE(callback);
  callback.Run(3);
  EXPECT_TRUE(callback);
  EXPECT_EQ(global_int_arg, 3);
  callback.Run(7);
  EXPECT_TRUE(callback);
  EXPECT_EQ(global_int_arg, 7);
}

TEST_F(IgnoreResultTest, BindRepeatingIgnoreResultWithArgument) {
  global_int_arg = -1;

  base::RepeatingCallback<void()> callback =
      base::BindRepeating(base::IgnoreResult(&SetGlobalIntArgAndReturnIt), 9);
  ASSERT_TRUE(callback);
  callback.Run();
  EXPECT_TRUE(callback);
  EXPECT_EQ(global_int_arg, 9);
  global_int_arg = 11;
  callback.Run();
  EXPECT_TRUE(callback);
  EXPECT_EQ(global_int_arg, 9);
}

TEST(IgnoreResultExhaustionTest, MemberFunctions) {
  MemberFunctions<int> obj;

  base::BindOnce(base::IgnoreResult(&MemberFunctions<int>::Function01),
                 base::Unretained(&obj))
      .Run();
  base::BindOnce(base::IgnoreResult(&MemberFunctions<int>::Function02),
                 base::Unretained(&obj))
      .Run();
  base::BindOnce(base::IgnoreResult(&MemberFunctions<int>::Function03),
                 base::Unretained(&obj))
      .Run();
  base::BindOnce(base::IgnoreResult(&MemberFunctions<int>::Function04),
                 base::Unretained(&obj))
      .Run();

  base::BindOnce(base::IgnoreResult(&MemberFunctions<int>::Function05),
                 base::Unretained(&obj))
      .Run();
  base::BindOnce(base::IgnoreResult(&MemberFunctions<int>::Function06),
                 base::Unretained(&obj))
      .Run();
  base::BindOnce(base::IgnoreResult(&MemberFunctions<int>::Function07),
                 base::Unretained(&obj))
      .Run();
  base::BindOnce(base::IgnoreResult(&MemberFunctions<int>::Function08),
                 base::Unretained(&obj))
      .Run();
  // Functions 09-12 are non-bindable to pointers since pointers cannot transfer
  // r-valueness

  base::BindOnce(base::IgnoreResult(&MemberFunctions<int>::Function13),
                 base::Unretained(&obj))
      .Run();
  base::BindOnce(base::IgnoreResult(&MemberFunctions<int>::Function14),
                 base::Unretained(&obj))
      .Run();
  base::BindOnce(base::IgnoreResult(&MemberFunctions<int>::Function15),
                 base::Unretained(&obj))
      .Run();
  base::BindOnce(base::IgnoreResult(&MemberFunctions<int>::Function16),
                 base::Unretained(&obj))
      .Run();

  base::BindOnce(base::IgnoreResult(&MemberFunctions<int>::Function17),
                 base::Unretained(&obj))
      .Run();
  base::BindOnce(base::IgnoreResult(&MemberFunctions<int>::Function18),
                 base::Unretained(&obj))
      .Run();
  base::BindOnce(base::IgnoreResult(&MemberFunctions<int>::Function19),
                 base::Unretained(&obj))
      .Run();
  base::BindOnce(base::IgnoreResult(&MemberFunctions<int>::Function20),
                 base::Unretained(&obj))
      .Run();
  // Functions 21-24 are non-bindable to pointers since pointers cannot transfer
  // r-valueness

  EXPECT_EQ(obj.value, 0b00001111'1111'00001111'1111);
}

}  // namespace
