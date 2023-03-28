#include "utils/rand.h"

#include <set>
#include <vector>

#include <gtest/gtest.h>
#include <srpc/types/integers.h>

using namespace dfis;

TEST(Utils, MakeMessageIdentifier) {
  std::set<srpc::i64> identifiers;
  for (int i = 0; i < 100'000; ++i) {
    auto id = MakeMessageIdentifier();
    ASSERT_FALSE(identifiers.contains(id));
    identifiers.insert(id);
  }
}
