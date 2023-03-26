#ifndef DFIS_MESSAGES_FLIGHT_H_
#define DFIS_MESSAGES_FLIGHT_H_

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include <srpc/types/floats.h>
#include <srpc/types/integers.h>
#include <srpc/types/serialization.h>

namespace dfis {

struct Flight {
  srpc::i32 identifier;
  std::string source;
  std::string destination;
  srpc::i64 departure_time;
  srpc::f32 airfare;
  srpc::i32 seat_availability;
};

std::ostream &operator<<(std::ostream &os, const Flight &flight);

}  // namespace dfis

namespace srpc {

template <>
struct Marshal<dfis::Flight> {
  [[nodiscard]] std::vector<std::byte> operator()(
      const dfis::Flight &flight) const;
};

template <>
struct Unmarshal<dfis::Flight> {
  [[nodiscard]] std::pair<i64, std::optional<dfis::Flight>> operator()(
      const std::span<const std::byte> &data) const;
};

}  // namespace srpc

#endif  // DFIS_MESSAGES_FLIGHT_H_
