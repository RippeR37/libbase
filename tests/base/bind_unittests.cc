#include "base/bind.h"

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

TEST(BindTest, FreeFundWithArgs) {
  global_int_arg = -1;

  auto cb = base::BindRepeating(&SetGlobalIntArg, 15);
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

TEST(AdvancedBindTest, AnotherCallbackToFreeFunction) {
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

}  // namespace
