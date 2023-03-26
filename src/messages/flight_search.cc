#include "messages/flight_search.h"

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

std::ostream &operator<<(std::ostream &os, const FlightSearchRequest &request) {
  os << request.source << " -> " << request.destination;
  return os;
}

std::ostream &operator<<(std::ostream &os,
                         const FlightSearchResponse &response) {
  if (response.flights.empty()) {
    os << "Error: " << response.message << "\n";
  } else {
    for (const auto &flight : response.flights) {
      os << flight << "\n";
    }
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
      dfis::MessageType::kFlightSearchRequest) {
    return {0, {}};
  }

  i64 p = sizeof(i32);

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

  auto message = Marshal<std::string>{}(response.message);
  data.insert(data.end(), message.begin(), message.end());

  auto flights = Marshal<std::vector<dfis::Flight>>{}(response.flights);
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
      dfis::MessageType::kFlightSearchResponse) {
    return {0, {}};
  }

  i64 p = sizeof(i32);

  auto message_res = Unmarshal<std::string>{}(
      std::span<const std::byte>{data.data() + p, data.data() + data.size()});
  if (!message_res.second.has_value()) {
    return {0, {}};
  }
  auto message = std::move(*message_res.second);
  p += message_res.first;

  auto flights_res = Unmarshal<std::vector<dfis::Flight>>{}(
      std::span<const std::byte>{data.data() + p, data.data() + data.size()});
  if (!flights_res.second.has_value()) {
    return {0, {}};
  }
  auto flights = std::move(*flights_res.second);
  p += flights_res.first;

  return {p, dfis::FlightSearchResponse{
                 .message = message,
                 .flights = flights,
             }};
}

}  // namespace srpc