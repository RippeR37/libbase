#include "base/memory/weak_ptr.h"

#include <memory>

#include "base/sequenced_task_runner_helpers.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

TEST(WeakPtrTest, Empty) {
  base::WeakPtr<bool> weak_ptr;
  EXPECT_FALSE(weak_ptr);
  EXPECT_FALSE(weak_ptr.MaybeValid());
  EXPECT_FALSE(weak_ptr.WasInvalidated());
  EXPECT_EQ(weak_ptr.Get(), nullptr);

  base::WeakPtr<bool> weak_ptr_copy = weak_ptr;
  EXPECT_FALSE(weak_ptr_copy);
  EXPECT_FALSE(weak_ptr_copy.MaybeValid());
  EXPECT_FALSE(weak_ptr_copy.WasInvalidated());
  EXPECT_EQ(weak_ptr_copy.Get(), nullptr);

  base::WeakPtr<bool> weak_ptr_move = std::move(weak_ptr);
  EXPECT_FALSE(weak_ptr_move);
  EXPECT_FALSE(weak_ptr_move.MaybeValid());
  EXPECT_FALSE(weak_ptr_move.WasInvalidated());
  EXPECT_EQ(weak_ptr_move.Get(), nullptr);
}

class WeakPtrFactoryOwner {
 public:
  void Call() { call_count_++; }
  size_t GetCallCount() const { return call_count_; }

 private:
  size_t call_count_{0u};

 public:
  // Must be last
  base::WeakPtrFactory<WeakPtrFactoryOwner> weak_factory{this};
};

class BaseWeakPtrFactoryTest : public ::testing::Test {
 public:
  BaseWeakPtrFactoryTest()
      : sequence_id_setter_(
            base::detail::SequenceIdGenerator::GetNextSequenceId()) {}

 protected:
  // We need to emulate that we're on some sequence for DCHECKs to work
  // correctly.
  base::detail::ScopedSequenceIdSetter sequence_id_setter_;
};

class WeakPtrFactoryTest : public BaseWeakPtrFactoryTest {
 public:
  void CreateWeakPtrFactoryOwner() {
    weak_class_ = std::make_unique<WeakPtrFactoryOwner>();
  }

  void DestroyWeakPtrFactoryOwner() { weak_class_.reset(); }

  base::WeakPtr<WeakPtrFactoryOwner> GetWeakPtr() const {
    return weak_class_->weak_factory.GetWeakPtr();
  }

  void InvalidateWeakPtrs() { weak_class_->weak_factory.InvalidateWeakPtrs(); }

  bool HasWeakPtrs() { return weak_class_->weak_factory.HasWeakPtrs(); }

 protected:
  std::unique_ptr<WeakPtrFactoryOwner> weak_class_;
};

TEST_F(WeakPtrFactoryTest, InvalidationFactoryWeakPtrByDestruction) {
  CreateWeakPtrFactoryOwner();

  EXPECT_FALSE(HasWeakPtrs());

  const auto weak_ptr = GetWeakPtr();
  EXPECT_TRUE(weak_ptr);
  EXPECT_TRUE(weak_ptr.MaybeValid());
  EXPECT_FALSE(weak_ptr.WasInvalidated());
  EXPECT_EQ(weak_ptr.Get(), weak_class_.get());
  EXPECT_EQ(&(*weak_ptr), weak_class_.get());
  EXPECT_TRUE(HasWeakPtrs());

  EXPECT_EQ(weak_class_->GetCallCount(), 0u);
  weak_ptr->Call();
  EXPECT_EQ(weak_class_->GetCallCount(), 1u);

  DestroyWeakPtrFactoryOwner();
  EXPECT_FALSE(weak_ptr);
  EXPECT_FALSE(weak_ptr.MaybeValid());
  EXPECT_TRUE(weak_ptr.WasInvalidated());
  EXPECT_EQ(weak_ptr.Get(), nullptr);
}

TEST_F(WeakPtrFactoryTest, InvalidationFactoryWeakPtrByMethodCall) {
  CreateWeakPtrFactoryOwner();

  EXPECT_FALSE(HasWeakPtrs());

  const auto weak_ptr = GetWeakPtr();
  EXPECT_TRUE(weak_ptr);
  EXPECT_TRUE(weak_ptr.MaybeValid());
  EXPECT_FALSE(weak_ptr.WasInvalidated());
  EXPECT_EQ(weak_ptr.Get(), weak_class_.get());
  EXPECT_EQ(&(*weak_ptr), weak_class_.get());
  EXPECT_TRUE(HasWeakPtrs());

  EXPECT_EQ(weak_class_->GetCallCount(), 0u);
  weak_ptr->Call();
  EXPECT_EQ(weak_class_->GetCallCount(), 1u);

  InvalidateWeakPtrs();
  EXPECT_FALSE(HasWeakPtrs());

  const auto new_weak_ptr = GetWeakPtr();
  EXPECT_TRUE(HasWeakPtrs());

  EXPECT_FALSE(weak_ptr);
  EXPECT_FALSE(weak_ptr.MaybeValid());
  EXPECT_TRUE(weak_ptr.WasInvalidated());
  EXPECT_EQ(weak_ptr.Get(), nullptr);

  EXPECT_TRUE(new_weak_ptr);
  EXPECT_TRUE(new_weak_ptr.MaybeValid());
  EXPECT_FALSE(new_weak_ptr.WasInvalidated());
  EXPECT_EQ(new_weak_ptr.Get(), weak_class_.get());
  EXPECT_EQ(&(*new_weak_ptr), weak_class_.get());
}

TEST_F(WeakPtrFactoryTest, WeakPtrCopyBehavesTheSame) {
  CreateWeakPtrFactoryOwner();

  auto weak_ptr1 = GetWeakPtr();
  auto weak_ptr2 = weak_ptr1;

  EXPECT_TRUE(weak_ptr1);
  EXPECT_TRUE(weak_ptr2);
  EXPECT_EQ(weak_ptr1.Get(), weak_class_.get());
  EXPECT_EQ(weak_ptr2.Get(), weak_class_.get());

  weak_ptr1->Call();
  weak_ptr2->Call();
  EXPECT_EQ(weak_class_->GetCallCount(), 2u);

  DestroyWeakPtrFactoryOwner();

  EXPECT_FALSE(weak_ptr1);
  EXPECT_FALSE(weak_ptr2);
  EXPECT_EQ(weak_ptr1.Get(), nullptr);
  EXPECT_EQ(weak_ptr2.Get(), nullptr);
}

TEST_F(WeakPtrFactoryTest, Move) {
  CreateWeakPtrFactoryOwner();

  auto weak_ptr1 = GetWeakPtr();
  auto weak_ptr2 = GetWeakPtr();

  auto weak_ptr3 = std::move(weak_ptr1);
  base::WeakPtr<WeakPtrFactoryOwner> weak_ptr4;
  weak_ptr4 = std::move(weak_ptr2);

  EXPECT_FALSE(weak_ptr1);
  EXPECT_FALSE(weak_ptr2);
  EXPECT_TRUE(weak_ptr3);
  EXPECT_TRUE(weak_ptr4);
  EXPECT_EQ(weak_ptr1.Get(), nullptr);
  EXPECT_EQ(weak_ptr2.Get(), nullptr);
  EXPECT_EQ(weak_ptr3.Get(), weak_class_.get());
  EXPECT_EQ(weak_ptr4.Get(), weak_class_.get());
}

TEST_F(WeakPtrFactoryTest, NullptrComparison) {
  base::WeakPtr<WeakPtrFactoryOwner> weak_ptr1;
  EXPECT_EQ(weak_ptr1, nullptr);

  CreateWeakPtrFactoryOwner();
  base::WeakPtr<WeakPtrFactoryOwner> weak_ptr2 = GetWeakPtr();
  EXPECT_NE(weak_ptr2, nullptr);
  base::WeakPtr<WeakPtrFactoryOwner> weak_ptr3 = weak_ptr2;
  EXPECT_NE(weak_ptr3, nullptr);
  base::WeakPtr<WeakPtrFactoryOwner> weak_ptr4 = std::move(weak_ptr2);
  EXPECT_NE(weak_ptr3, nullptr);
  EXPECT_EQ(weak_ptr1, nullptr);

  InvalidateWeakPtrs();

  EXPECT_EQ(nullptr, weak_ptr1);
  EXPECT_EQ(nullptr, weak_ptr2);
  EXPECT_EQ(nullptr, weak_ptr3);
  EXPECT_EQ(nullptr, weak_ptr4);

  auto weak_ptr5 = GetWeakPtr();
  EXPECT_NE(nullptr, weak_ptr5);
}

class Base {
 public:
  virtual ~Base() = default;

  void Foo() {}
};

class Derived : public Base {
 public:
  void Bar() {}
};

class WeakPtrFactoryUpcastingTest : public BaseWeakPtrFactoryTest {};

TEST_F(WeakPtrFactoryUpcastingTest, UpcastingCopyCtor) {
  Derived derived;
  base::WeakPtrFactory<Derived> weak_factory{&derived};

  base::WeakPtr<Derived> weak_derived = weak_factory.GetWeakPtr();
  EXPECT_TRUE(weak_derived);
  EXPECT_EQ(weak_derived.Get(), &derived);

  base::WeakPtr<Base> weak_base{weak_derived};
  EXPECT_TRUE(weak_base);
  EXPECT_EQ(weak_base.Get(), static_cast<Base*>(&derived));
  EXPECT_TRUE(weak_derived);
  EXPECT_EQ(weak_derived.Get(), &derived);

  weak_factory.InvalidateWeakPtrs();
  EXPECT_FALSE(weak_derived);
  EXPECT_EQ(weak_derived.Get(), nullptr);
  EXPECT_FALSE(weak_base);
  EXPECT_EQ(weak_base.Get(), nullptr);
}

TEST_F(WeakPtrFactoryUpcastingTest, UpcastingMoveCtor) {
  Derived derived;
  base::WeakPtrFactory<Derived> weak_factory{&derived};

  base::WeakPtr<Derived> weak_derived = weak_factory.GetWeakPtr();
  EXPECT_TRUE(weak_derived);
  EXPECT_EQ(weak_derived.Get(), &derived);

  base::WeakPtr<Base> weak_base{std::move(weak_derived)};
  EXPECT_TRUE(weak_base);
  EXPECT_EQ(weak_base.Get(), static_cast<Base*>(&derived));
  EXPECT_FALSE(weak_derived);
  EXPECT_EQ(weak_derived.Get(), nullptr);

  weak_factory.InvalidateWeakPtrs();
  EXPECT_FALSE(weak_derived);
  EXPECT_EQ(weak_derived.Get(), nullptr);
  EXPECT_FALSE(weak_base);
  EXPECT_EQ(weak_base.Get(), nullptr);
}

TEST_F(WeakPtrFactoryUpcastingTest, UpcastingCopyAssign) {
  Base base;
  Derived derived;
  base::WeakPtrFactory<Base> weak_base_factory{&base};
  base::WeakPtrFactory<Derived> weak_derived_factory{&derived};

  base::WeakPtr<Derived> weak_derived = weak_derived_factory.GetWeakPtr();
  EXPECT_TRUE(weak_derived);
  EXPECT_EQ(weak_derived.Get(), &derived);

  base::WeakPtr<Base> weak_base = weak_base_factory.GetWeakPtr();
  weak_base = weak_derived;
  EXPECT_TRUE(weak_base);
  EXPECT_EQ(weak_base.Get(), static_cast<Base*>(&derived));
  EXPECT_TRUE(weak_derived);
  EXPECT_EQ(weak_derived.Get(), &derived);

  weak_base_factory.InvalidateWeakPtrs();
  EXPECT_TRUE(weak_base);
  EXPECT_EQ(weak_base.Get(), static_cast<Base*>(&derived));
  EXPECT_TRUE(weak_derived);
  EXPECT_EQ(weak_derived.Get(), &derived);

  weak_derived_factory.InvalidateWeakPtrs();
  EXPECT_FALSE(weak_derived);
  EXPECT_EQ(weak_derived.Get(), nullptr);
  EXPECT_FALSE(weak_base);
  EXPECT_EQ(weak_base.Get(), nullptr);
}

TEST_F(WeakPtrFactoryUpcastingTest, UpcastingMoveAssign) {
  Base base;
  Derived derived;
  base::WeakPtrFactory<Base> weak_base_factory{&base};
  base::WeakPtrFactory<Derived> weak_derived_factory{&derived};

  base::WeakPtr<Derived> weak_derived = weak_derived_factory.GetWeakPtr();
  EXPECT_TRUE(weak_derived);
  EXPECT_EQ(weak_derived.Get(), &derived);

  base::WeakPtr<Base> weak_base = weak_base_factory.GetWeakPtr();
  weak_base = std::move(weak_derived);
  EXPECT_TRUE(weak_base);
  EXPECT_EQ(weak_base.Get(), static_cast<Base*>(&derived));
  EXPECT_FALSE(weak_derived);
  EXPECT_EQ(weak_derived.Get(), nullptr);

  weak_base_factory.InvalidateWeakPtrs();
  EXPECT_TRUE(weak_base);
  EXPECT_EQ(weak_base.Get(), static_cast<Base*>(&derived));
  EXPECT_FALSE(weak_derived);
  EXPECT_EQ(weak_derived.Get(), nullptr);

  weak_derived_factory.InvalidateWeakPtrs();
  EXPECT_FALSE(weak_derived);
  EXPECT_EQ(weak_derived.Get(), nullptr);
  EXPECT_FALSE(weak_base);
  EXPECT_EQ(weak_base.Get(), nullptr);
}

class MultiBase1 {
 public:
  virtual ~MultiBase1() = default;

  void Foo1() {}
  int n1 = 0;
};

class MultiBase2 {
 public:
  virtual ~MultiBase2() = default;

  void Foo2() {}
  int n2 = 0;
};

class MultiDerived : public MultiBase1, public MultiBase2 {
 public:
  void Bar() {}

  int n3 = 0;
};

class WeakPtrFactoryMultiInheritanceUpcastingTest
    : public BaseWeakPtrFactoryTest {};

TEST_F(WeakPtrFactoryMultiInheritanceUpcastingTest, UpcastingCopyCtor) {
  MultiDerived derived;
  base::WeakPtrFactory<MultiDerived> weak_factory{&derived};

  base::WeakPtr<MultiDerived> weak_derived = weak_factory.GetWeakPtr();
  EXPECT_TRUE(weak_derived);
  EXPECT_EQ(weak_derived.Get(), &derived);

  base::WeakPtr<MultiBase1> weak_base1{weak_derived};
  base::WeakPtr<MultiBase2> weak_base2{weak_derived};
  EXPECT_TRUE(weak_derived);
  EXPECT_EQ(weak_derived.Get(), &derived);
  EXPECT_TRUE(weak_base1);
  EXPECT_EQ(weak_base1.Get(), static_cast<MultiBase1*>(&derived));
  EXPECT_TRUE(weak_base2);
  EXPECT_EQ(weak_base2.Get(), static_cast<MultiBase2*>(&derived));

  weak_factory.InvalidateWeakPtrs();
  EXPECT_FALSE(weak_derived);
  EXPECT_EQ(weak_derived.Get(), nullptr);
  EXPECT_FALSE(weak_base1);
  EXPECT_EQ(weak_base1.Get(), nullptr);
  EXPECT_FALSE(weak_base2);
  EXPECT_EQ(weak_base2.Get(), nullptr);
}

TEST_F(WeakPtrFactoryMultiInheritanceUpcastingTest, UpcastingMoveCtor) {
  MultiDerived derived;
  base::WeakPtrFactory<MultiDerived> weak_factory{&derived};

  base::WeakPtr<MultiDerived> weak_derived1 = weak_factory.GetWeakPtr();
  base::WeakPtr<MultiDerived> weak_derived2 = weak_factory.GetWeakPtr();
  EXPECT_TRUE(weak_derived1);
  EXPECT_EQ(weak_derived1.Get(), &derived);
  EXPECT_TRUE(weak_derived2);
  EXPECT_EQ(weak_derived2.Get(), &derived);

  base::WeakPtr<MultiBase1> weak_base1{std::move(weak_derived1)};
  base::WeakPtr<MultiBase2> weak_base2{std::move(weak_derived2)};
  EXPECT_FALSE(weak_derived1);
  EXPECT_EQ(weak_derived1.Get(), nullptr);
  EXPECT_FALSE(weak_derived2);
  EXPECT_EQ(weak_derived2.Get(), nullptr);
  EXPECT_TRUE(weak_base1);
  EXPECT_EQ(weak_base1.Get(), static_cast<MultiBase1*>(&derived));
  EXPECT_TRUE(weak_base2);
  EXPECT_EQ(weak_base2.Get(), static_cast<MultiBase2*>(&derived));

  weak_factory.InvalidateWeakPtrs();
  EXPECT_FALSE(weak_derived1);
  EXPECT_EQ(weak_derived1.Get(), nullptr);
  EXPECT_FALSE(weak_derived2);
  EXPECT_EQ(weak_derived2.Get(), nullptr);
  EXPECT_FALSE(weak_base1);
  EXPECT_EQ(weak_base1.Get(), nullptr);
  EXPECT_FALSE(weak_base2);
  EXPECT_EQ(weak_base2.Get(), nullptr);
}

TEST_F(WeakPtrFactoryMultiInheritanceUpcastingTest, UpcastingCopyAssign) {
  MultiBase1 base1;
  MultiBase2 base2;
  MultiDerived derived;
  base::WeakPtrFactory<MultiBase1> weak_base1_factory{&base1};
  base::WeakPtrFactory<MultiBase2> weak_base2_factory{&base2};
  base::WeakPtrFactory<MultiDerived> weak_derived_factory{&derived};

  base::WeakPtr<MultiDerived> weak_derived = weak_derived_factory.GetWeakPtr();
  EXPECT_TRUE(weak_derived);
  EXPECT_EQ(weak_derived.Get(), &derived);

  base::WeakPtr<MultiBase1> weak_base1 = weak_base1_factory.GetWeakPtr();
  base::WeakPtr<MultiBase2> weak_base2 = weak_base2_factory.GetWeakPtr();
  weak_base1 = weak_derived;
  weak_base2 = weak_derived;
  EXPECT_TRUE(weak_derived);
  EXPECT_EQ(weak_derived.Get(), &derived);
  EXPECT_TRUE(weak_base1);
  EXPECT_EQ(weak_base1.Get(), static_cast<MultiBase1*>(&derived));
  EXPECT_TRUE(weak_base2);
  EXPECT_EQ(weak_base2.Get(), static_cast<MultiBase2*>(&derived));

  weak_base1_factory.InvalidateWeakPtrs();
  weak_base2_factory.InvalidateWeakPtrs();
  EXPECT_TRUE(weak_derived);
  EXPECT_EQ(weak_derived.Get(), &derived);
  EXPECT_TRUE(weak_base1);
  EXPECT_EQ(weak_base1.Get(), static_cast<MultiBase1*>(&derived));
  EXPECT_TRUE(weak_base2);
  EXPECT_EQ(weak_base2.Get(), static_cast<MultiBase2*>(&derived));

  weak_derived_factory.InvalidateWeakPtrs();
  EXPECT_FALSE(weak_derived);
  EXPECT_EQ(weak_derived.Get(), nullptr);
  EXPECT_FALSE(weak_base1);
  EXPECT_EQ(weak_base1.Get(), nullptr);
  EXPECT_FALSE(weak_base2);
  EXPECT_EQ(weak_base2.Get(), nullptr);
}

TEST_F(WeakPtrFactoryMultiInheritanceUpcastingTest, UpcastingMoveAssign) {
  MultiBase1 base1;
  MultiBase2 base2;
  MultiDerived derived;
  base::WeakPtrFactory<MultiBase1> weak_base1_factory{&base1};
  base::WeakPtrFactory<MultiBase2> weak_base2_factory{&base2};
  base::WeakPtrFactory<MultiDerived> weak_derived_factory{&derived};

  base::WeakPtr<MultiDerived> weak_derived1 = weak_derived_factory.GetWeakPtr();
  base::WeakPtr<MultiDerived> weak_derived2 = weak_derived_factory.GetWeakPtr();
  EXPECT_TRUE(weak_derived1);
  EXPECT_EQ(weak_derived1.Get(), &derived);
  EXPECT_TRUE(weak_derived2);
  EXPECT_EQ(weak_derived2.Get(), &derived);

  base::WeakPtr<MultiBase1> weak_base1 = weak_base1_factory.GetWeakPtr();
  base::WeakPtr<MultiBase2> weak_base2 = weak_base2_factory.GetWeakPtr();
  weak_base1 = std::move(weak_derived1);
  weak_base2 = std::move(weak_derived2);
  EXPECT_FALSE(weak_derived1);
  EXPECT_EQ(weak_derived1.Get(), nullptr);
  EXPECT_FALSE(weak_derived2);
  EXPECT_EQ(weak_derived2.Get(), nullptr);
  EXPECT_TRUE(weak_base1);
  EXPECT_EQ(weak_base1.Get(), static_cast<MultiBase1*>(&derived));
  EXPECT_TRUE(weak_base2);
  EXPECT_EQ(weak_base2.Get(), static_cast<MultiBase2*>(&derived));

  weak_base1_factory.InvalidateWeakPtrs();
  weak_base2_factory.InvalidateWeakPtrs();
  EXPECT_FALSE(weak_derived1);
  EXPECT_EQ(weak_derived1.Get(), nullptr);
  EXPECT_FALSE(weak_derived2);
  EXPECT_EQ(weak_derived2.Get(), nullptr);
  EXPECT_TRUE(weak_base1);
  EXPECT_EQ(weak_base1.Get(), static_cast<MultiBase1*>(&derived));
  EXPECT_TRUE(weak_base2);
  EXPECT_EQ(weak_base2.Get(), static_cast<MultiBase2*>(&derived));

  weak_derived_factory.InvalidateWeakPtrs();
  EXPECT_FALSE(weak_derived1);
  EXPECT_EQ(weak_derived1.Get(), nullptr);
  EXPECT_FALSE(weak_derived2);
  EXPECT_EQ(weak_derived2.Get(), nullptr);
  EXPECT_FALSE(weak_base1);
  EXPECT_EQ(weak_base1.Get(), nullptr);
  EXPECT_FALSE(weak_base2);
  EXPECT_EQ(weak_base2.Get(), nullptr);
}

}  // namespace
