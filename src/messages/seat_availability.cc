#include "messages/seat_availability.h"

#include <cstddef>
#include <optional>
#include <ostream>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include <srpc/types/integers.h>
#include <srpc/types/serialization.h>
#include <srpc/types/strings.h>
#include <srpc/types/vectors.h>

#include "messages/message_type.h"
#include "utils/time.h"

namespace dfis {

std::ostream &operator<<(std::ostream &os,
                         const SeatAvailabilityMonitoringRequest &request) {
  os << "[" << request.id << "] " << request.identifier << " ("
     << request.monitor_interval_sec << "s)";
  return os;
}

std::ostream &operator<<(std::ostream &os,
                         const SeatAvailabilityMonitoringResponse &response) {
  os << "[" << response.id << "] ";
  if (response.status_code != 0) {
    os << "Error: " << response.message;
  } else {
    os << "Monitoring " << response.identifier << " until "
       << FormatTimestamp(response.monitor_end);
  }
  return os;
}

std::ostream &operator<<(std::ostream &os,
                         const SeatAvailabilityCallbackRequest &request) {
  os << "[" << request.id << "] " << request.identifier << " ("
     << request.seat_availability << " seat(s) avail.)";
  return os;
}

std::ostream &operator<<(std::ostream &os,
                         const SeatAvailabilityCallbackResponse &response) {
  os << "[" << response.id << "] ";
  if (response.status_code != 0) {
    os << "Some error(s) occurred in the callback loop";
  } else {
    os << "Callback is completed successfully";
  }
  return os;
}

}  // namespace dfis

namespace srpc {

[[nodiscard]] std::vector<std::byte>
Marshal<dfis::SeatAvailabilityMonitoringRequest>::operator()(
    const dfis::SeatAvailabilityMonitoringRequest &request) const {
  std::vector<std::byte> data(sizeof(i32));

  Marshal<i32>{}(
      static_cast<i32>(dfis::SeatAvailabilityMonitoringRequest::kMessageType),
      std::span<std::byte, sizeof(i32)>{data.data(),
                                        data.data() + sizeof(i32)});

  auto id = Marshal<i64>{}(request.id);
  data.insert(data.end(), id.begin(), id.end());

  auto identifier = Marshal<i32>{}(request.identifier);
  data.insert(data.end(), identifier.begin(), identifier.end());

  auto monitor_interval_sec = Marshal<i32>{}(request.monitor_interval_sec);
  data.insert(data.end(), monitor_interval_sec.begin(),
              monitor_interval_sec.end());

  return data;
}

[[nodiscard]] std::pair<i64,
                        std::optional<dfis::SeatAvailabilityMonitoringRequest>>
Unmarshal<dfis::SeatAvailabilityMonitoringRequest>::operator()(
    const std::span<const std::byte> &data) const {
  if (data.size() < sizeof(i32)) {
    return {0, {}};
  }

  auto message_type = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data(), data.data() + sizeof(i32)});
  if (dfis::MessageType{message_type} !=
      dfis::SeatAvailabilityMonitoringRequest::kMessageType) {
    return {0, {}};
  }

  i64 p = sizeof(i32);

  if (p + sizeof(i64) > data.size()) {
    return {0, {}};
  }
  auto id = Unmarshal<i64>{}(std::span<const std::byte, sizeof(i64)>{
      data.data() + p, data.data() + p + sizeof(i64)});
  p += sizeof(i64);

  if (p + sizeof(i32) > data.size()) {
    return {0, {}};
  }
  auto identifier = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data() + p, data.data() + p + sizeof(i32)});
  p += sizeof(i32);

  if (p + sizeof(i32) > data.size()) {
    return {0, {}};
  }
  auto monitor_interval_sec =
      Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
          data.data() + p, data.data() + p + sizeof(i32)});
  p += sizeof(i32);

  return {p, dfis::SeatAvailabilityMonitoringRequest{
                 .id = id,
                 .identifier = identifier,
                 .monitor_interval_sec = monitor_interval_sec,
             }};
}

[[nodiscard]] std::vector<std::byte>
Marshal<dfis::SeatAvailabilityMonitoringResponse>::operator()(
    const dfis::SeatAvailabilityMonitoringResponse &response) const {
  std::vector<std::byte> data(sizeof(i32));

  Marshal<i32>{}(
      static_cast<i32>(dfis::SeatAvailabilityMonitoringResponse::kMessageType),
      std::span<std::byte, sizeof(i32)>{data.data(),
                                        data.data() + sizeof(i32)});

  auto id = Marshal<i64>{}(response.id);
  data.insert(data.end(), id.begin(), id.end());

  auto status_code = Marshal<i32>{}(response.status_code);
  data.insert(data.end(), status_code.begin(), status_code.end());

  auto message = Marshal<std::string>{}(response.message);
  data.insert(data.end(), message.begin(), message.end());

  auto identifier = Marshal<i32>{}(response.identifier);
  data.insert(data.end(), identifier.begin(), identifier.end());

  auto monitor_end = Marshal<i64>{}(response.monitor_end);
  data.insert(data.end(), monitor_end.begin(), monitor_end.end());

  return data;
}

[[nodiscard]] std::pair<i64,
                        std::optional<dfis::SeatAvailabilityMonitoringResponse>>
Unmarshal<dfis::SeatAvailabilityMonitoringResponse>::operator()(
    const std::span<const std::byte> &data) const {
  if (data.size() < sizeof(i32)) {
    return {0, {}};
  }

  auto message_type = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data(), data.data() + sizeof(i32)});
  if (dfis::MessageType{message_type} !=
      dfis::SeatAvailabilityMonitoringResponse::kMessageType) {
    return {0, {}};
  }

  i64 p = sizeof(i32);

  if (p + sizeof(i64) > data.size()) {
    return {0, {}};
  }
  auto id = Unmarshal<i64>{}(std::span<const std::byte, sizeof(i64)>{
      data.data() + p, data.data() + p + sizeof(i64)});
  p += sizeof(i64);

  if (p + sizeof(i32) > data.size()) {
    return {0, {}};
  }
  auto status_code = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data() + p, data.data() + p + sizeof(i32)});
  p += sizeof(i32);

  auto message_res = Unmarshal<std::string>{}(
      std::span<const std::byte>{data.data() + p, data.data() + data.size()});
  if (!message_res.second.has_value()) {
    return {0, {}};
  }
  auto message = std::move(*message_res.second);
  p += message_res.first;

  if (p + sizeof(i32) > data.size()) {
    return {0, {}};
  }
  auto identifier = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data() + p, data.data() + p + sizeof(i32)});
  p += sizeof(i32);

  if (p + sizeof(i64) > data.size()) {
    return {0, {}};
  }
  auto monitor_end = Unmarshal<i64>{}(std::span<const std::byte, sizeof(i64)>{
      data.data() + p, data.data() + p + sizeof(i64)});
  p += sizeof(i64);

  return {p, dfis::SeatAvailabilityMonitoringResponse{
                 .id = id,
                 .status_code = status_code,
                 .message = message,
                 .identifier = identifier,
                 .monitor_end = monitor_end,
             }};
}

[[nodiscard]] std::vector<std::byte>
Marshal<dfis::SeatAvailabilityCallbackRequest>::operator()(
    const dfis::SeatAvailabilityCallbackRequest &request) const {
  std::vector<std::byte> data(sizeof(i32));

  Marshal<i32>{}(
      static_cast<i32>(dfis::SeatAvailabilityCallbackRequest::kMessageType),
      std::span<std::byte, sizeof(i32)>{data.data(),
                                        data.data() + sizeof(i32)});

  auto id = Marshal<i64>{}(request.id);
  data.insert(data.end(), id.begin(), id.end());

  auto identifier = Marshal<i32>{}(request.identifier);
  data.insert(data.end(), identifier.begin(), identifier.end());

  auto seat_availability = Marshal<i32>{}(request.seat_availability);
  data.insert(data.end(), seat_availability.begin(), seat_availability.end());

  return data;
}

[[nodiscard]] std::pair<i64,
                        std::optional<dfis::SeatAvailabilityCallbackRequest>>
Unmarshal<dfis::SeatAvailabilityCallbackRequest>::operator()(
    const std::span<const std::byte> &data) const {
  if (data.size() < sizeof(i32)) {
    return {0, {}};
  }

  auto message_type = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data(), data.data() + sizeof(i32)});
  if (dfis::MessageType{message_type} !=
      dfis::SeatAvailabilityCallbackRequest::kMessageType) {
    return {0, {}};
  }

  i64 p = sizeof(i32);

  if (p + sizeof(i64) > data.size()) {
    return {0, {}};
  }
  auto id = Unmarshal<i64>{}(std::span<const std::byte, sizeof(i64)>{
      data.data() + p, data.data() + p + sizeof(i64)});
  p += sizeof(i64);

  if (p + sizeof(i32) > data.size()) {
    return {0, {}};
  }
  auto identifier = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data() + p, data.data() + p + sizeof(i32)});
  p += sizeof(i32);

  if (p + sizeof(i32) > data.size()) {
    return {0, {}};
  }
  auto seat_availability =
      Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
          data.data() + p, data.data() + p + sizeof(i32)});
  p += sizeof(i32);

  return {p, dfis::SeatAvailabilityCallbackRequest{
                 .id = id,
                 .identifier = identifier,
                 .seat_availability = seat_availability,
             }};
}

[[nodiscard]] std::vector<std::byte>
Marshal<dfis::SeatAvailabilityCallbackResponse>::operator()(
    const dfis::SeatAvailabilityCallbackResponse &response) const {
  std::vector<std::byte> data(sizeof(i32));

  Marshal<i32>{}(
      static_cast<i32>(dfis::SeatAvailabilityCallbackResponse::kMessageType),
      std::span<std::byte, sizeof(i32)>{data.data(),
                                        data.data() + sizeof(i32)});

  auto id = Marshal<i64>{}(response.id);
  data.insert(data.end(), id.begin(), id.end());

  auto status_code = Marshal<i32>{}(response.status_code);
  data.insert(data.end(), status_code.begin(), status_code.end());

  return data;
}

[[nodiscard]] std::pair<i64,
                        std::optional<dfis::SeatAvailabilityCallbackResponse>>
Unmarshal<dfis::SeatAvailabilityCallbackResponse>::operator()(
    const std::span<const std::byte> &data) const {
  if (data.size() < sizeof(i32)) {
    return {0, {}};
  }

  auto message_type = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data(), data.data() + sizeof(i32)});
  if (dfis::MessageType{message_type} !=
      dfis::SeatAvailabilityCallbackResponse::kMessageType) {
    return {0, {}};
  }

  i64 p = sizeof(i32);

  if (p + sizeof(i64) > data.size()) {
    return {0, {}};
  }
  auto id = Unmarshal<i64>{}(std::span<const std::byte, sizeof(i64)>{
      data.data() + p, data.data() + p + sizeof(i64)});
  p += sizeof(i64);

  if (p + sizeof(i32) > data.size()) {
    return {0, {}};
  }
  auto status_code = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data() + p, data.data() + p + sizeof(i32)});
  p += sizeof(i32);

  return {p, dfis::SeatAvailabilityCallbackResponse{
                 .id = id,
                 .status_code = status_code,
             }};
}

}  // namespace srpc
