#ifndef DFIS_MESSAGES_SEAT_RESERVATION_H_
#define DFIS_MESSAGES_SEAT_RESERVATION_H_

#include <cstddef>
#include <optional>
#include <ostream>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include <srpc/types/integers.h>
#include <srpc/types/serialization.h>

#include "messages/message_type.h"

namespace dfis {

struct SeatReservationRequest {
  static constexpr MessageType kMessageType =
      MessageType::kSeatReservationRequest;
  srpc::i32 identifier;
  srpc::i32 seats;
};

std::ostream &operator<<(std::ostream &os,
                         const SeatReservationRequest &request);

struct SeatReservationResponse {
  static constexpr MessageType kMessageType =
      MessageType::kSeatReservationResponse;
  std::string message;
  srpc::i32 identifier;
  srpc::i32 seats;
};

std::ostream &operator<<(std::ostream &os,
                         const SeatReservationResponse &response);

}  // namespace dfis

namespace srpc {

template <>
struct Marshal<dfis::SeatReservationRequest> {
  [[nodiscard]] std::vector<std::byte> operator()(
      const dfis::SeatReservationRequest &request) const;
};

template <>
struct Unmarshal<dfis::SeatReservationRequest> {
  [[nodiscard]] std::pair<i64, std::optional<dfis::SeatReservationRequest>>
  operator()(const std::span<const std::byte> &data) const;
};

template <>
struct Marshal<dfis::SeatReservationResponse> {
  [[nodiscard]] std::vector<std::byte> operator()(
      const dfis::SeatReservationResponse &response) const;
};

template <>
struct Unmarshal<dfis::SeatReservationResponse> {
  [[nodiscard]] std::pair<i64, std::optional<dfis::SeatReservationResponse>>
  operator()(const std::span<const std::byte> &data) const;
};

}  // namespace srpc

#endif  // DFIS_MESSAGES_SEAT_RESERVATION_H_
