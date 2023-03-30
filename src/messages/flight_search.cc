#include "messages/flight_search.h"

#include <cstddef>
#include <optional>
#include <ostream>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include <srpc/types/floats.h>
#include <srpc/types/integers.h>
#include <srpc/types/serialization.h>
#include <srpc/types/strings.h>
#include <srpc/types/vectors.h>

#include "messages/message_type.h"

namespace dfis {

std::ostream &operator<<(std::ostream &os, const FlightSearchRequest &request) {
  os << "[" << request.id << "] " << request.source << " -> "
     << request.destination;
  return os;
}

std::ostream &operator<<(std::ostream &os,
                         const FlightSearchResponse &response) {
  os << "[" << response.id << "] ";
  if (response.status_code != 0) {
    os << "Error: " << response.message;
  } else {
    os << "{";
    bool is_first = true;
    for (const auto &flight : response.flights) {
      if (is_first) {
        is_first = false;
      } else {
        os << ", ";
      }
      os << flight;
    }
    os << "}";
  }
  return os;
}

std::ostream &operator<<(std::ostream &os,
                         const PriceRangeSearchRequest &request) {
  os << "[" << request.id << "] $" << request.from << " to $" << request.to;
  return os;
}

std::ostream &operator<<(std::ostream &os,
                         const PriceRangeSearchResponse &response) {
  os << "[" << response.id << "] ";
  if (response.status_code != 0) {
    os << "Error: " << response.message;
  } else {
    os << "{";
    bool is_first = true;
    for (const auto &flight : response.flights) {
      if (is_first) {
        is_first = false;
      } else {
        os << ", ";
      }
      os << flight;
    }
    os << "}";
  }
  return os;
}

}  // namespace dfis

namespace srpc {

[[nodiscard]] std::vector<std::byte>
Marshal<dfis::FlightSearchRequest>::operator()(
    const dfis::FlightSearchRequest &request) const {
  std::vector<std::byte> data(sizeof(i32));

  Marshal<i32>{}(static_cast<i32>(dfis::FlightSearchRequest::kMessageType),
                 std::span<std::byte, sizeof(i32)>{data.data(),
                                                   data.data() + sizeof(i32)});

  auto id = Marshal<u64>{}(request.id);
  data.insert(data.end(), id.begin(), id.end());

  auto source = Marshal<std::string>{}(request.source);
  data.insert(data.end(), source.begin(), source.end());

  auto destination = Marshal<std::string>{}(request.destination);
  data.insert(data.end(), destination.begin(), destination.end());

  return data;
}

[[nodiscard]] std::pair<i64, std::optional<dfis::FlightSearchRequest>>
Unmarshal<dfis::FlightSearchRequest>::operator()(
    const std::span<const std::byte> &data) const {
  if (data.size() < sizeof(i32)) {
    return {0, {}};
  }

  auto message_type = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data(), data.data() + sizeof(i32)});
  if (dfis::MessageType{message_type} !=
      dfis::FlightSearchRequest::kMessageType) {
    return {0, {}};
  }

  i64 p = sizeof(i32);

  if (p + sizeof(u64) > data.size()) {
    return {0, {}};
  }
  auto id = Unmarshal<u64>{}(std::span<const std::byte, sizeof(u64)>{
      data.data() + p, data.data() + p + sizeof(u64)});
  p += sizeof(u64);

  auto source_res = Unmarshal<std::string>{}(
      std::span<const std::byte>{data.data() + p, data.data() + data.size()});
  if (!source_res.second.has_value()) {
    return {0, {}};
  }
  auto source = std::move(*source_res.second);
  p += source_res.first;

  auto destination_res = Unmarshal<std::string>{}(
      std::span<const std::byte>{data.data() + p, data.data() + data.size()});
  if (!destination_res.second.has_value()) {
    return {0, {}};
  }
  auto destination = std::move(*destination_res.second);
  p += destination_res.first;

  return {p, dfis::FlightSearchRequest{
                 .id = id,
                 .source = source,
                 .destination = destination,
             }};
}

[[nodiscard]] std::vector<std::byte>
Marshal<dfis::FlightSearchResponse>::operator()(
    const dfis::FlightSearchResponse &response) const {
  std::vector<std::byte> data(sizeof(i32));

  Marshal<i32>{}(static_cast<i32>(dfis::FlightSearchResponse::kMessageType),
                 std::span<std::byte, sizeof(i32)>{data.data(),
                                                   data.data() + sizeof(i32)});

  auto id = Marshal<u64>{}(response.id);
  data.insert(data.end(), id.begin(), id.end());

  auto status_code = Marshal<i32>{}(response.status_code);
  data.insert(data.end(), status_code.begin(), status_code.end());

  auto message = Marshal<std::string>{}(response.message);
  data.insert(data.end(), message.begin(), message.end());

  auto flights = Marshal<std::vector<i32>>{}(response.flights);
  data.insert(data.end(), flights.begin(), flights.end());

  return data;
}

[[nodiscard]] std::pair<i64, std::optional<dfis::FlightSearchResponse>>
Unmarshal<dfis::FlightSearchResponse>::operator()(
    const std::span<const std::byte> &data) const {
  if (data.size() < sizeof(i32)) {
    return {0, {}};
  }

  auto message_type = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data(), data.data() + sizeof(i32)});
  if (dfis::MessageType{message_type} !=
      dfis::FlightSearchResponse::kMessageType) {
    return {0, {}};
  }

  i64 p = sizeof(i32);

  if (p + sizeof(u64) > data.size()) {
    return {0, {}};
  }
  auto id = Unmarshal<u64>{}(std::span<const std::byte, sizeof(u64)>{
      data.data() + p, data.data() + p + sizeof(u64)});
  p += sizeof(u64);

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

  auto flights_res = Unmarshal<std::vector<i32>>{}(
      std::span<const std::byte>{data.data() + p, data.data() + data.size()});
  if (!flights_res.second.has_value()) {
    return {0, {}};
  }
  auto flights = std::move(*flights_res.second);
  p += flights_res.first;

  return {p, dfis::FlightSearchResponse{
                 .id = id,
                 .status_code = status_code,
                 .message = message,
                 .flights = flights,
             }};
}

[[nodiscard]] std::vector<std::byte>
Marshal<dfis::PriceRangeSearchRequest>::operator()(
    const dfis::PriceRangeSearchRequest &request) const {
  std::vector<std::byte> data(sizeof(i32));

  Marshal<i32>{}(static_cast<i32>(dfis::PriceRangeSearchRequest::kMessageType),
                 std::span<std::byte, sizeof(i32)>{data.data(),
                                                   data.data() + sizeof(i32)});

  auto id = Marshal<u64>{}(request.id);
  data.insert(data.end(), id.begin(), id.end());

  auto from = Marshal<f32>{}(request.from);
  data.insert(data.end(), from.begin(), from.end());

  auto to = Marshal<f32>{}(request.to);
  data.insert(data.end(), to.begin(), to.end());

  return data;
}

[[nodiscard]] std::pair<i64, std::optional<dfis::PriceRangeSearchRequest>>
Unmarshal<dfis::PriceRangeSearchRequest>::operator()(
    const std::span<const std::byte> &data) const {
  if (data.size() < sizeof(i32)) {
    return {0, {}};
  }

  auto message_type = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data(), data.data() + sizeof(i32)});
  if (dfis::MessageType{message_type} !=
      dfis::PriceRangeSearchRequest::kMessageType) {
    return {0, {}};
  }

  i64 p = sizeof(i32);

  if (p + sizeof(u64) > data.size()) {
    return {0, {}};
  }
  auto id = Unmarshal<u64>{}(std::span<const std::byte, sizeof(u64)>{
      data.data() + p, data.data() + p + sizeof(u64)});
  p += sizeof(u64);

  if (p + sizeof(f32) > data.size()) {
    return {0, {}};
  }
  auto from = Unmarshal<f32>{}(std::span<const std::byte, sizeof(f32)>{
      data.data() + p, data.data() + p + sizeof(f32)});
  p += sizeof(f32);

  if (p + sizeof(f32) > data.size()) {
    return {0, {}};
  }
  auto to = Unmarshal<f32>{}(std::span<const std::byte, sizeof(f32)>{
      data.data() + p, data.data() + p + sizeof(f32)});
  p += sizeof(f32);

  return {p, dfis::PriceRangeSearchRequest{
                 .id = id,
                 .from = from,
                 .to = to,
             }};
}

[[nodiscard]] std::vector<std::byte>
Marshal<dfis::PriceRangeSearchResponse>::operator()(
    const dfis::PriceRangeSearchResponse &response) const {
  std::vector<std::byte> data(sizeof(i32));

  Marshal<i32>{}(static_cast<i32>(dfis::PriceRangeSearchResponse::kMessageType),
                 std::span<std::byte, sizeof(i32)>{data.data(),
                                                   data.data() + sizeof(i32)});

  auto id = Marshal<u64>{}(response.id);
  data.insert(data.end(), id.begin(), id.end());

  auto status_code = Marshal<i32>{}(response.status_code);
  data.insert(data.end(), status_code.begin(), status_code.end());

  auto message = Marshal<std::string>{}(response.message);
  data.insert(data.end(), message.begin(), message.end());

  auto flights = Marshal<std::vector<i32>>{}(response.flights);
  data.insert(data.end(), flights.begin(), flights.end());

  return data;
}

[[nodiscard]] std::pair<i64, std::optional<dfis::PriceRangeSearchResponse>>
Unmarshal<dfis::PriceRangeSearchResponse>::operator()(
    const std::span<const std::byte> &data) const {
  if (data.size() < sizeof(i32)) {
    return {0, {}};
  }

  auto message_type = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data(), data.data() + sizeof(i32)});
  if (dfis::MessageType{message_type} !=
      dfis::PriceRangeSearchResponse::kMessageType) {
    return {0, {}};
  }

  i64 p = sizeof(i32);

  if (p + sizeof(u64) > data.size()) {
    return {0, {}};
  }
  auto id = Unmarshal<u64>{}(std::span<const std::byte, sizeof(u64)>{
      data.data() + p, data.data() + p + sizeof(u64)});
  p += sizeof(u64);

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

  auto flights_res = Unmarshal<std::vector<i32>>{}(
      std::span<const std::byte>{data.data() + p, data.data() + data.size()});
  if (!flights_res.second.has_value()) {
    return {0, {}};
  }
  auto flights = std::move(*flights_res.second);
  p += flights_res.first;

  return {p, dfis::PriceRangeSearchResponse{
                 .id = id,
                 .status_code = status_code,
                 .message = message,
                 .flights = flights,
             }};
}

}  // namespace srpc
