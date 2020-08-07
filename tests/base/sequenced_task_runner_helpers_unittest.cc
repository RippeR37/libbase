#include "base/sequenced_task_runner_helpers.h"

#include "gtest/gtest.h"

namespace {

TEST(SequenceIdGeneratorTest, CopiedValuesNotUnique) {
  const auto id1 = base::detail::SequenceIdGenerator::GetNextSequenceId();
  const auto id1_copy = id1;

  EXPECT_EQ(id1, id1_copy);

  const auto id2 = base::detail::SequenceIdGenerator::GetNextSequenceId();
  const auto id2_copy = id2;
  EXPECT_EQ(id2, id2_copy);

  EXPECT_NE(id1, id2);
  EXPECT_NE(id1, id2_copy);
  EXPECT_NE(id1_copy, id2);
  EXPECT_NE(id1_copy, id2_copy);
}

TEST(SequenceIdGeneratorTest, UniqueValues) {
  std::vector<base::SequenceId> sequences;

  for (size_t idx = 0; idx < 10; ++idx) {
    sequences.push_back(base::detail::SequenceIdGenerator::GetNextSequenceId());
  }

  for (auto current = sequences.begin(); current != sequences.end();
       ++current) {
    for (auto other = std::next(current); other != sequences.end(); ++other) {
      EXPECT_NE(*current, *other);
    }
  }
}

TEST(CurrentSequenceIdHelperTest, NotCurrentIfNotSet) {
  const auto id1 = base::detail::SequenceIdGenerator::GetNextSequenceId();
  EXPECT_FALSE(base::detail::CurrentSequenceIdHelper::IsCurrentSequence(id1));

  const auto id2 = base::detail::SequenceIdGenerator::GetNextSequenceId();
  EXPECT_FALSE(base::detail::CurrentSequenceIdHelper::IsCurrentSequence(id1));
  EXPECT_FALSE(base::detail::CurrentSequenceIdHelper::IsCurrentSequence(id2));

  const auto id3 = base::detail::SequenceIdGenerator::GetNextSequenceId();
  EXPECT_FALSE(base::detail::CurrentSequenceIdHelper::IsCurrentSequence(id1));
  EXPECT_FALSE(base::detail::CurrentSequenceIdHelper::IsCurrentSequence(id2));
  EXPECT_FALSE(base::detail::CurrentSequenceIdHelper::IsCurrentSequence(id3));
}

TEST(ScopedSequenceIdSetterTest, CurrentIdSet) {
  const auto id1 = base::detail::SequenceIdGenerator::GetNextSequenceId();
  const auto id2 = base::detail::SequenceIdGenerator::GetNextSequenceId();

  EXPECT_FALSE(base::detail::CurrentSequenceIdHelper::IsCurrentSequence(id1));
  EXPECT_FALSE(base::detail::CurrentSequenceIdHelper::IsCurrentSequence(id2));

  {
    base::detail::ScopedSequenceIdSetter setter_id1{id1};
    EXPECT_TRUE(base::detail::CurrentSequenceIdHelper::IsCurrentSequence(id1));
    EXPECT_FALSE(base::detail::CurrentSequenceIdHelper::IsCurrentSequence(id2));

    const auto id1_copy = id1;
    const auto id2_copy = id2;
    EXPECT_TRUE(
        base::detail::CurrentSequenceIdHelper::IsCurrentSequence(id1_copy));
    EXPECT_FALSE(
        base::detail::CurrentSequenceIdHelper::IsCurrentSequence(id2_copy));
  }

  EXPECT_FALSE(base::detail::CurrentSequenceIdHelper::IsCurrentSequence(id1));
  EXPECT_FALSE(base::detail::CurrentSequenceIdHelper::IsCurrentSequence(id2));

  {
    base::detail::ScopedSequenceIdSetter setter_id2{id2};
    EXPECT_FALSE(base::detail::CurrentSequenceIdHelper::IsCurrentSequence(id1));
    EXPECT_TRUE(base::detail::CurrentSequenceIdHelper::IsCurrentSequence(id2));

    const auto id1_copy = id1;
    const auto id2_copy = id2;
    EXPECT_FALSE(
        base::detail::CurrentSequenceIdHelper::IsCurrentSequence(id1_copy));
    EXPECT_TRUE(
        base::detail::CurrentSequenceIdHelper::IsCurrentSequence(id2_copy));
  }

  EXPECT_FALSE(base::detail::CurrentSequenceIdHelper::IsCurrentSequence(id1));
  EXPECT_FALSE(base::detail::CurrentSequenceIdHelper::IsCurrentSequence(id2));
}

}  // namespace
