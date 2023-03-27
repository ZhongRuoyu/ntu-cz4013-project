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
