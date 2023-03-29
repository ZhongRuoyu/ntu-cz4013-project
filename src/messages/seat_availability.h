#ifndef DFIS_MESSAGES_SEAT_AVAILABILITY_H_
#define DFIS_MESSAGES_SEAT_AVAILABILITY_H_

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

struct SeatAvailabilityMonitoringRequest {
  static constexpr MessageType kMessageType =
      MessageType::kSeatAvailabilityMonitoringRequest;
  srpc::u64 id;
  srpc::i32 identifier;
  srpc::u16 port;
  srpc::i32 monitor_interval_sec;
};

std::ostream &operator<<(std::ostream &os,
                         const SeatAvailabilityMonitoringRequest &request);

struct SeatAvailabilityMonitoringResponse {
  static constexpr MessageType kMessageType =
      MessageType::kSeatAvailabilityMonitoringResponse;
  srpc::u64 id;
  srpc::i32 status_code;
  std::string message;
  srpc::i32 identifier;
  srpc::i64 monitor_end;
};

std::ostream &operator<<(std::ostream &os,
                         const SeatAvailabilityMonitoringResponse &response);

struct SeatAvailabilityCallbackRequest {
  static constexpr MessageType kMessageType =
      MessageType::kSeatAvailabilityCallbackRequest;
  srpc::u64 id;
  srpc::i32 identifier;
  srpc::i32 seat_availability;
};

std::ostream &operator<<(std::ostream &os,
                         const SeatAvailabilityCallbackRequest &request);

struct SeatAvailabilityCallbackResponse {
  static constexpr MessageType kMessageType =
      MessageType::kSeatAvailabilityCallbackResponse;
  srpc::u64 id;
  srpc::i32 status_code;
};

std::ostream &operator<<(std::ostream &os,
                         const SeatAvailabilityCallbackResponse &response);

}  // namespace dfis

namespace srpc {

template <>
struct Marshal<dfis::SeatAvailabilityMonitoringRequest> {
  [[nodiscard]] std::vector<std::byte> operator()(
      const dfis::SeatAvailabilityMonitoringRequest &request) const;
};

template <>
struct Unmarshal<dfis::SeatAvailabilityMonitoringRequest> {
  [[nodiscard]] std::pair<
      i64, std::optional<dfis::SeatAvailabilityMonitoringRequest>>
  operator()(const std::span<const std::byte> &data) const;
};

template <>
struct Marshal<dfis::SeatAvailabilityMonitoringResponse> {
  [[nodiscard]] std::vector<std::byte> operator()(
      const dfis::SeatAvailabilityMonitoringResponse &response) const;
};

template <>
struct Unmarshal<dfis::SeatAvailabilityMonitoringResponse> {
  [[nodiscard]] std::pair<
      i64, std::optional<dfis::SeatAvailabilityMonitoringResponse>>
  operator()(const std::span<const std::byte> &data) const;
};

template <>
struct Marshal<dfis::SeatAvailabilityCallbackRequest> {
  [[nodiscard]] std::vector<std::byte> operator()(
      const dfis::SeatAvailabilityCallbackRequest &request) const;
};

template <>
struct Unmarshal<dfis::SeatAvailabilityCallbackRequest> {
  [[nodiscard]] std::pair<i64,
                          std::optional<dfis::SeatAvailabilityCallbackRequest>>
  operator()(const std::span<const std::byte> &data) const;
};

template <>
struct Marshal<dfis::SeatAvailabilityCallbackResponse> {
  [[nodiscard]] std::vector<std::byte> operator()(
      const dfis::SeatAvailabilityCallbackResponse &response) const;
};

template <>
struct Unmarshal<dfis::SeatAvailabilityCallbackResponse> {
  [[nodiscard]] std::pair<i64,
                          std::optional<dfis::SeatAvailabilityCallbackResponse>>
  operator()(const std::span<const std::byte> &data) const;
};

}  // namespace srpc

#endif  // DFIS_MESSAGES_SEAT_AVAILABILITY_H_
