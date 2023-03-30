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
  srpc::u64 id;
  srpc::i32 identifier;
  srpc::i32 seats;
};

std::ostream &operator<<(std::ostream &os,
                         const SeatReservationRequest &request);

struct SeatReservationResponse {
  static constexpr MessageType kMessageType =
      MessageType::kSeatReservationResponse;
  srpc::u64 id;
  srpc::i32 status_code;
  std::string message;
  srpc::i32 identifier;
  srpc::i32 seats;
};

std::ostream &operator<<(std::ostream &os,
                         const SeatReservationResponse &response);

struct SeatReservationCancellationRequest {
  static constexpr MessageType kMessageType =
      MessageType::kSeatReservationCancellationRequest;
  srpc::u64 id;
  srpc::u64 reservation_req_id;
  srpc::i32 identifier;
  srpc::i32 seats;
};

std::ostream &operator<<(std::ostream &os,
                         const SeatReservationCancellationRequest &request);

struct SeatReservationCancellationResponse {
  static constexpr MessageType kMessageType =
      MessageType::kSeatReservationCancellationResponse;
  srpc::u64 id;
  srpc::i32 status_code;
  std::string message;
  srpc::i32 identifier;
  srpc::i32 seats;
};

std::ostream &operator<<(std::ostream &os,
                         const SeatReservationCancellationResponse &response);

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

template <>
struct Marshal<dfis::SeatReservationCancellationRequest> {
  [[nodiscard]] std::vector<std::byte> operator()(
      const dfis::SeatReservationCancellationRequest &request) const;
};

template <>
struct Unmarshal<dfis::SeatReservationCancellationRequest> {
  [[nodiscard]] std::pair<
      i64, std::optional<dfis::SeatReservationCancellationRequest>>
  operator()(const std::span<const std::byte> &data) const;
};

template <>
struct Marshal<dfis::SeatReservationCancellationResponse> {
  [[nodiscard]] std::vector<std::byte> operator()(
      const dfis::SeatReservationCancellationResponse &response) const;
};

template <>
struct Unmarshal<dfis::SeatReservationCancellationResponse> {
  [[nodiscard]] std::pair<
      i64, std::optional<dfis::SeatReservationCancellationResponse>>
  operator()(const std::span<const std::byte> &data) const;
};

}  // namespace srpc

#endif  // DFIS_MESSAGES_SEAT_RESERVATION_H_
