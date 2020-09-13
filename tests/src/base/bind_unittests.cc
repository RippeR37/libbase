#include "base/bind.h"

#include "base/sequenced_task_runner_helpers.h"
#include "base/threading/thread.h"

#include "gtest/gtest.h"

namespace {

int global_int_arg = -1;
double global_double_arg = -1.0;

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

TEST(RepeatingCallbackTest, FreeFunction) {
  global_int_arg = -1;

  base::RepeatingCallback<void(int)> cb(&SetGlobalIntArg);
  cb.Run(7);
  EXPECT_EQ(global_int_arg, 7);
}

TEST(RepeatingCallbackTest, SimpleLambda) {
  global_double_arg = -1.0;

  base::RepeatingCallback<void(double)> cb(
      [](double x) { global_double_arg = x; });
  cb.Run(2.71);
  EXPECT_EQ(global_double_arg, 2.71);
}

TEST(RepeatingCallbackTest, ClassMethod) {
  Counter counter;
  base::RepeatingCallback<void(int)> cb(&Counter::add, &counter);
  ASSERT_EQ(counter.sum, 0);
  cb.Run(37);
  EXPECT_EQ(counter.sum, 37);
  cb.Run(5);
  EXPECT_EQ(counter.sum, 42);
}

TEST(RepeatingCallbackTest, CopyRepeatingCallback) {
  Counter counter;
  base::RepeatingCallback<void(int)> cb_1(&Counter::add, &counter);
  base::RepeatingCallback<void(int)> cb_2(cb_1, std::make_tuple());
  base::RepeatingCallback<void(int)> cb_3(cb_1);

  ASSERT_EQ(counter.sum, 0);
  cb_1.Run(11);
  EXPECT_EQ(counter.sum, 11);
  cb_2.Run(-4);
  EXPECT_EQ(counter.sum, 7);
}

TEST(RepeatingCallbackTest, FreeFunctionWithArg) {
  global_int_arg = -1;

  base::RepeatingCallback<void()> cb{&SetGlobalIntArg, std::make_tuple(71)};
  cb.Run();
  EXPECT_EQ(global_int_arg, 71);
}

TEST(RepeatingCallbackTest, ClassMethodWithArg) {
  Counter counter;
  base::RepeatingCallback<void()> cb{&Counter::add, &counter,
                                     std::make_tuple(11)};
  ASSERT_EQ(counter.sum, 0);
  cb.Run();
  EXPECT_EQ(counter.sum, 11);
  cb.Run();
  EXPECT_EQ(counter.sum, 22);
  cb.Run();
  EXPECT_EQ(counter.sum, 33);
}

TEST(RepeatingCallbackTest, MultiArgFreeFunction) {
  base::RepeatingCallback<int(int, int)> cb{&add};
  EXPECT_EQ(cb.Run(0, 0), 0);
  EXPECT_EQ(cb.Run(2, 3), 5);
  EXPECT_EQ(cb.Run(-2, 1), -1);
}

TEST(RepeatingCallbackTest, MultiArgFreeFunctionWithSomeArgs) {
  base::RepeatingCallback<int(int)> cb{&add, std::make_tuple(1)};
  EXPECT_EQ(cb.Run(0), 1);
  EXPECT_EQ(cb.Run(1), 2);
  EXPECT_EQ(cb.Run(7), 8);
  EXPECT_EQ(cb.Run(-3), -2);
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
  auto cb = base::BindRepeating(&Counter::add, &counter);
  ASSERT_EQ(counter.sum, 0);
  cb.Run(7);
  EXPECT_EQ(counter.sum, 7);
  cb.Run(3);
  EXPECT_EQ(counter.sum, 10);
}

TEST(BindTest, ClassMethodWithArgs) {
  Counter counter;
  auto cb = base::BindRepeating(&Counter::add, &counter, 21);
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
  auto cb = base::BindOnce(&Counter::add, &counter);
  ASSERT_EQ(counter.sum, 0);
  std::move(cb).Run(7);
  EXPECT_EQ(counter.sum, 7);
  EXPECT_FALSE(cb);
}

TEST(BindOnceTest, ClassMethodWithArgs) {
  Counter counter;
  auto cb = base::BindOnce(&Counter::add, &counter, 21);
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

TEST(OnceCallbackTest, FreeFuncTakingUniquePtr) {
  global_int_arg = -1;

  auto cb = base::OnceCallback<void(std::unique_ptr<int>)>(
      &SetGlobalIntFromUniquePtrArg);
  std::move(cb).Run(std::make_unique<int>(3));
  EXPECT_EQ(global_int_arg, 3);
  EXPECT_TRUE(!cb);
}

TEST(OnceCallbackTest, LambdaTakingUniquePtr) {
  global_int_arg = -1;

  auto cb = base::OnceCallback<void(std::unique_ptr<int>)>(
      [](std::unique_ptr<int> x) { global_int_arg = *x; });
  std::move(cb).Run(std::make_unique<int>(7));
  EXPECT_EQ(global_int_arg, 7);
  EXPECT_TRUE(!cb);
}

TEST(OnceCallbackTest, FreeFuncTakingUniquePtrWithArg) {
  global_int_arg = -1;

  auto cb = base::OnceCallback<void()>(
      &SetGlobalIntFromUniquePtrArg, std::make_tuple(std::make_unique<int>(3)));
  std::move(cb).Run();
  EXPECT_EQ(global_int_arg, 3);
  EXPECT_TRUE(!cb);
}

TEST(OnceCallbackTest, AnotherOnceCallback) {
  global_int_arg = -1;

  auto cb_1 = base::OnceCallback<void(std::unique_ptr<int>)>(
      &SetGlobalIntFromUniquePtrArg);
  auto cb_2 = base::OnceCallback<void(std::unique_ptr<int>)>(std::move(cb_1));

  EXPECT_TRUE(!cb_1);
  std::move(cb_2).Run(std::make_unique<int>(-3));
  EXPECT_EQ(global_int_arg, -3);
}

TEST(OnceCallbackTest, AnotherOnceCallbackAlreadyWithArgs) {
  global_int_arg = -1;

  auto cb_1 = base::OnceCallback<void()>(
      &SetGlobalIntFromUniquePtrArg, std::make_tuple(std::make_unique<int>(3)));
  auto cb_2 = base::OnceCallback<void()>(std::move(cb_1));

  EXPECT_TRUE(!cb_1);
  std::move(cb_2).Run();
  EXPECT_EQ(global_int_arg, 3);
}

TEST(OnceCallbackTest, AnotherOnceCallbackWithArgs) {
  global_int_arg = -1;

  auto cb_1 = base::OnceCallback<void(std::unique_ptr<int>)>(
      &SetGlobalIntFromUniquePtrArg);
  auto cb_2 = base::OnceCallback<void()>(
      std::move(cb_1), std::make_tuple(std::make_unique<int>(5)));

  EXPECT_TRUE(!cb_1);
  std::move(cb_2).Run();
  EXPECT_EQ(global_int_arg, 5);
}

TEST(OnceCallbackTest, OnceCallbackFromRepeatingCallback) {
  global_int_arg = -1;

  auto cb_1 = base::RepeatingCallback<void(int)>(&SetGlobalIntArg);
  auto cb_2 = base::OnceCallback<void(int)>(cb_1);
  std::move(cb_2).Run(2);
  EXPECT_EQ(global_int_arg, 2);
  EXPECT_TRUE(!cb_2);
}

TEST(BindOnceCallbackTest, SimpleNoArgs) {
  global_int_arg = -1;

  auto cb = base::BindOnce(&SetGlobalIntFromUniquePtrArg);
  std::move(cb).Run(std::make_unique<int>(11));
  EXPECT_EQ(global_int_arg, 11);
  EXPECT_TRUE(!cb);
}

TEST(BindOnceCallbackTest, WithArg) {
  global_int_arg = -1;

  auto cb =
      base::BindOnce(&SetGlobalIntFromUniquePtrArg, std::make_unique<int>(25));
  std::move(cb).Run();
  EXPECT_EQ(global_int_arg, 25);
  EXPECT_TRUE(!cb);
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
  base::WeakPtr<WeakClass> weak_object = nullptr;
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
  base::WeakPtr<WeakClass> weak_object = nullptr;
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

}  // namespace
