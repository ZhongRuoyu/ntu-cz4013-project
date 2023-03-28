#ifndef DFIS_MESSAGES_FLIGHT_INFO_H_
#define DFIS_MESSAGES_FLIGHT_INFO_H_

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

struct FlightInfoRequest {
  static constexpr MessageType kMessageType = MessageType::kFlightInfoRequest;
  srpc::i64 id;
  srpc::i32 identifier;
};

std::ostream &operator<<(std::ostream &os, const FlightInfoRequest &request);

struct FlightInfoResponse {
  static constexpr MessageType kMessageType = MessageType::kFlightInfoResponse;
  srpc::i64 id;
  srpc::i32 status_code;
  std::string message;
  std::vector<dfis::Flight> flight;
};

std::ostream &operator<<(std::ostream &os, const FlightInfoResponse &response);

}  // namespace dfis

namespace srpc {

template <>
struct Marshal<dfis::FlightInfoRequest> {
  [[nodiscard]] std::vector<std::byte> operator()(
      const dfis::FlightInfoRequest &request) const;
};

template <>
struct Unmarshal<dfis::FlightInfoRequest> {
  [[nodiscard]] std::pair<i64, std::optional<dfis::FlightInfoRequest>>
  operator()(const std::span<const std::byte> &data) const;
};

template <>
struct Marshal<dfis::FlightInfoResponse> {
  [[nodiscard]] std::vector<std::byte> operator()(
      const dfis::FlightInfoResponse &response) const;
};

template <>
struct Unmarshal<dfis::FlightInfoResponse> {
  [[nodiscard]] std::pair<i64, std::optional<dfis::FlightInfoResponse>>
  operator()(const std::span<const std::byte> &data) const;
};

}  // namespace srpc

#endif  // DFIS_MESSAGES_FLIGHT_INFO_H_
