#include "utils/time.h"

#include <cstdlib>

#include <gtest/gtest.h>

using namespace dfis;

TEST(Utils, FormatTimestamp) {
  char env[] = "TZ=UTC";
  // NOLINTNEXTLINE(concurrency-mt-unsafe)
  putenv(env);
  // NOLINTNEXTLINE(concurrency-mt-unsafe)
  ASSERT_STREQ("UTC", getenv("TZ"));
  ASSERT_EQ("1970-01-01T00:00:00+0000", FormatTimestamp(0));
  ASSERT_EQ("02/05/23", FormatTimestamp(1675555200, "%D"));
  ASSERT_EQ("16:00", FormatTimestamp(1675526400, "%R"));
}

TEST(Utils, ToTimestamp) {
  char env[] = "TZ=UTC";
  // NOLINTNEXTLINE(concurrency-mt-unsafe)
  putenv(env);
  // NOLINTNEXTLINE(concurrency-mt-unsafe)
  ASSERT_STREQ("UTC", getenv("TZ"));

  auto ts1 = ToTimestamp("1970-01-01T00:00:00", "%Y-%m-%dT%H:%M:%S");
  ASSERT_TRUE(ts1.has_value());
  // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
  ASSERT_EQ(0, *ts1);

  auto ts2 = ToTimestamp("02/05/23", "%D");
  ASSERT_TRUE(ts2.has_value());
  // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
  ASSERT_EQ(1675555200, *ts2);
}
