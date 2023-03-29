#ifndef DFIS_MESSAGES_FLIGHT_SEARCH_H_
#define DFIS_MESSAGES_FLIGHT_SEARCH_H_

#include <cstddef>
#include <optional>
#include <ostream>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include <srpc/types/integers.h>
#include <srpc/types/serialization.h>

#include "messages/flight.h"
#include "messages/message_type.h"

namespace dfis {

struct FlightSearchRequest {
  static constexpr MessageType kMessageType = MessageType::kFlightSearchRequest;
  srpc::u64 id;
  std::string source;
  std::string destination;
};

std::ostream &operator<<(std::ostream &os, const FlightSearchRequest &request);

struct FlightSearchResponse {
  static constexpr MessageType kMessageType =
      MessageType::kFlightSearchResponse;
  srpc::u64 id;
  srpc::i32 status_code;
  std::string message;
  std::vector<dfis::Flight> flights;
};

std::ostream &operator<<(std::ostream &os,
                         const FlightSearchResponse &response);

}  // namespace dfis

namespace srpc {

template <>
struct Marshal<dfis::FlightSearchRequest> {
  [[nodiscard]] std::vector<std::byte> operator()(
      const dfis::FlightSearchRequest &request) const;
};

template <>
struct Unmarshal<dfis::FlightSearchRequest> {
  [[nodiscard]] std::pair<i64, std::optional<dfis::FlightSearchRequest>>
  operator()(const std::span<const std::byte> &data) const;
};

template <>
struct Marshal<dfis::FlightSearchResponse> {
  [[nodiscard]] std::vector<std::byte> operator()(
      const dfis::FlightSearchResponse &response) const;
};

template <>
struct Unmarshal<dfis::FlightSearchResponse> {
  [[nodiscard]] std::pair<i64, std::optional<dfis::FlightSearchResponse>>
  operator()(const std::span<const std::byte> &data) const;
};

}  // namespace srpc

#endif  // DFIS_MESSAGES_FLIGHT_SEARCH_H_
