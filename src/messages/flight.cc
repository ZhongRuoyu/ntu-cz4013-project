#include "messages/flight.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <optional>
#include <span>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <srpc/types/floats.h>
#include <srpc/types/integers.h>
#include <srpc/types/serialization.h>
#include <srpc/types/strings.h>

#include "utils/time.h"

namespace dfis {

bool Flight::operator==(const Flight &other) const {
  return this->identifier == other.identifier && this->source == other.source &&
         this->destination == other.destination &&
         this->departure_time == other.departure_time &&
         this->airfare == other.airfare &&
         this->seat_availability == other.seat_availability;
}

std::ostream &operator<<(std::ostream &os, const Flight &flight) {
  os << flight.identifier << " (" << flight.source << " -> "
     << flight.destination << ") @ "
     << dfis::FormatTimestamp(flight.departure_time) << " $" << flight.airfare
     << " (" << flight.seat_availability << " seat(s) avail.)";
  return os;
}

}  // namespace dfis

namespace srpc {

std::vector<std::byte> Marshal<dfis::Flight>::operator()(
    const dfis::Flight &flight) const {
  std::vector<std::byte> data;

  auto identifier = Marshal<i32>{}(flight.identifier);
  data.insert(data.end(), identifier.begin(), identifier.end());

  auto source = Marshal<std::string>{}(flight.source);
  data.insert(data.end(), source.begin(), source.end());

  auto destination = Marshal<std::string>{}(flight.destination);
  data.insert(data.end(), destination.begin(), destination.end());

  auto departure_time = Marshal<i64>{}(flight.departure_time);
  data.insert(data.end(), departure_time.begin(), departure_time.end());

  auto airfare = Marshal<f32>{}(flight.airfare);
  data.insert(data.end(), airfare.begin(), airfare.end());

  auto seat_availability = Marshal<i32>{}(flight.seat_availability);
  data.insert(data.end(), seat_availability.begin(), seat_availability.end());

  return data;
}

std::pair<i64, std::optional<dfis::Flight>> Unmarshal<dfis::Flight>::operator()(
    const std::span<const std::byte> &data) const {
  i64 p = 0;

  if (p + sizeof(i32) > data.size()) {
    return {0, {}};
  }
  auto identifier = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data() + p, data.data() + p + sizeof(i32)});
  p += sizeof(i32);

  auto source_res = Unmarshal<std::string>{}(
      std::span<const std::byte>{data.data() + p, data.data() + data.size()});
  if (!source_res.second.has_value()) {
    return {0, {}};
  }
  p += source_res.first;
  std::string source = std::move(*source_res.second);

  auto destination_res = Unmarshal<std::string>{}(
      std::span<const std::byte>{data.data() + p, data.data() + data.size()});
  if (!destination_res.second.has_value()) {
    return {0, {}};
  }
  p += destination_res.first;
  std::string destination = std::move(*destination_res.second);

  if (p + sizeof(i64) > data.size()) {
    return {0, {}};
  }
  auto departure_time =
      Unmarshal<i64>{}(std::span<const std::byte, sizeof(i64)>{
          data.data() + p, data.data() + p + sizeof(i64)});
  p += sizeof(i64);

  if (p + sizeof(f32) > data.size()) {
    return {0, {}};
  }
  auto airfare = Unmarshal<f32>{}(std::span<const std::byte, sizeof(f32)>{
      data.data() + p, data.data() + p + sizeof(f32)});
  p += sizeof(f32);

  if (p + sizeof(i32) > data.size()) {
    return {0, {}};
  }
  auto seat_availability =
      Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
          data.data() + p, data.data() + p + sizeof(i32)});
  p += sizeof(i32);

  return {p, dfis::Flight{
                 .identifier = identifier,
                 .source = source,
                 .destination = destination,
                 .departure_time = departure_time,
                 .airfare = airfare,
                 .seat_availability = seat_availability,
             }};
}

}  // namespace srpc
