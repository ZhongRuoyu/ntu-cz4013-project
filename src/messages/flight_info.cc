#include "messages/flight_info.h"

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

#include "messages/flight.h"
#include "messages/message_type.h"

namespace dfis {

std::ostream &operator<<(std::ostream &os, const FlightInfoRequest &request) {
  os << request.identifier;
  return os;
}

std::ostream &operator<<(std::ostream &os, const FlightInfoResponse &response) {
  if (response.status_code != 0) {
    os << "Error: " << response.message;
  } else {
    for (const auto &flight : response.flight) {
      os << flight << "\n";
    }
  }
  return os;
}

}  // namespace dfis

namespace srpc {

[[nodiscard]] std::vector<std::byte>
Marshal<dfis::FlightInfoRequest>::operator()(
    const dfis::FlightInfoRequest &request) const {
  std::vector<std::byte> data(sizeof(i32));

  Marshal<i32>{}(static_cast<i32>(dfis::FlightInfoRequest::kMessageType),
                 std::span<std::byte, sizeof(i32)>{data.data(),
                                                   data.data() + sizeof(i32)});

  auto id = Marshal<i64>{}(request.id);
  data.insert(data.end(), id.begin(), id.end());

  auto identifier = Marshal<i32>{}(request.identifier);
  data.insert(data.end(), identifier.begin(), identifier.end());

  return data;
}

[[nodiscard]] std::pair<i64, std::optional<dfis::FlightInfoRequest>>
Unmarshal<dfis::FlightInfoRequest>::operator()(
    const std::span<const std::byte> &data) const {
  if (data.size() < sizeof(i32)) {
    return {0, {}};
  }

  auto message_type = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data(), data.data() + sizeof(i32)});
  if (dfis::MessageType{message_type} !=
      dfis::FlightInfoRequest::kMessageType) {
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

  return {p, dfis::FlightInfoRequest{
                 .id = id,
                 .identifier = identifier,
             }};
}

[[nodiscard]] std::vector<std::byte>
Marshal<dfis::FlightInfoResponse>::operator()(
    const dfis::FlightInfoResponse &response) const {
  std::vector<std::byte> data(sizeof(i32));

  Marshal<i32>{}(static_cast<i32>(dfis::FlightInfoResponse::kMessageType),
                 std::span<std::byte, sizeof(i32)>{data.data(),
                                                   data.data() + sizeof(i32)});

  auto id = Marshal<i64>{}(response.id);
  data.insert(data.end(), id.begin(), id.end());

  auto status_code = Marshal<i32>{}(response.status_code);
  data.insert(data.end(), status_code.begin(), status_code.end());

  auto message = Marshal<std::string>{}(response.message);
  data.insert(data.end(), message.begin(), message.end());

  auto flight = Marshal<std::vector<dfis::Flight>>{}(response.flight);
  data.insert(data.end(), flight.begin(), flight.end());

  return data;
}

[[nodiscard]] std::pair<i64, std::optional<dfis::FlightInfoResponse>>
Unmarshal<dfis::FlightInfoResponse>::operator()(
    const std::span<const std::byte> &data) const {
  if (data.size() < sizeof(i32)) {
    return {0, {}};
  }

  auto message_type = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data(), data.data() + sizeof(i32)});
  if (dfis::MessageType{message_type} !=
      dfis::FlightInfoResponse::kMessageType) {
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

  auto flight_res = Unmarshal<std::vector<dfis::Flight>>{}(
      std::span<const std::byte>{data.data() + p, data.data() + data.size()});
  if (!flight_res.second.has_value()) {
    return {0, {}};
  }
  auto flight = std::move(*flight_res.second);
  p += flight_res.first;

  return {p, dfis::FlightInfoResponse{
                 .id = id,
                 .status_code = status_code,
                 .message = message,
                 .flight = flight,
             }};
}

}  // namespace srpc
