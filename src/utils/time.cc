#include "utils/time.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>

namespace dfis {

std::string FormatTimestamp(srpc::i64 timestamp, const char *format) {
  std::time_t timepoint = std::chrono::system_clock::to_time_t(
      std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>{
          std::chrono::seconds(timestamp)});
  std::tm local_time{};
  localtime_r(&timepoint, &local_time);
  std::ostringstream ss;
  ss << std::put_time(&local_time, format);
  return ss.str();
}

std::optional<srpc::i64> ToTimestamp(const std::string &str,
                                     const char *format) {
  std::tm local_time{};
  std::istringstream ss{str};
  ss >> std::get_time(&local_time, format);
  if (ss.fail()) {
    return {};
  }
  return std::chrono::duration_cast<std::chrono::seconds>(
             std::chrono::system_clock::from_time_t(std::mktime(&local_time))
                 .time_since_epoch())
      .count();
}

}  // namespace dfis
