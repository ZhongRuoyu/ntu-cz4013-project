#include "utils/time.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

namespace dfis {

std::string FormatTimestamp(srpc::i64 timestamp, const char *format) {
  std::time_t timepoint = std::chrono::system_clock::to_time_t(
      std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>{
          std::chrono::seconds(timestamp)});
  std::tm local_time;
  localtime_r(&timepoint, &local_time);
  std::ostringstream ss;
  ss << std::put_time(&local_time, format);
  return ss.str();
}

}  // namespace dfis
