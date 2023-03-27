#ifndef DFIS_UTILS_TIME_H_
#define DFIS_UTILS_TIME_H_

#include <optional>
#include <string>

#include <srpc/types/integers.h>

namespace dfis {

inline constexpr const char *kISO8601Format = "%Y-%m-%dT%H:%M:%S%z";

[[nodiscard]] std::string FormatTimestamp(srpc::i64 timestamp,
                                          const char *format = kISO8601Format);

[[nodiscard]] std::optional<srpc::i64> ToTimestamp(const std::string &str,
                                                   const char *format);

}  // namespace dfis

#endif  // DFIS_UTILS_TIME_H_
