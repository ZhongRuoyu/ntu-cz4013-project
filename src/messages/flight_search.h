#ifndef DFIS_MESSAGES_FLIGHT_SEARCH_H_
#define DFIS_MESSAGES_FLIGHT_SEARCH_H_

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
  std::vector<srpc::i32> flights;
};

std::ostream &operator<<(std::ostream &os,
                         const FlightSearchResponse &response);

struct PriceRangeSearchRequest {
  static constexpr MessageType kMessageType = MessageType::kFlightSearchRequest;
  srpc::u64 id;
  srpc::f32 from;
  srpc::f32 to;
};

std::ostream &operator<<(std::ostream &os,
                         const PriceRangeSearchRequest &request);

struct PriceRangeSearchResponse {
  static constexpr MessageType kMessageType =
      MessageType::kFlightSearchResponse;
  srpc::u64 id;
  srpc::i32 status_code;
  std::string message;
  std::vector<srpc::i32> flights;
};

std::ostream &operator<<(std::ostream &os,
                         const PriceRangeSearchResponse &response);

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

template <>
struct Marshal<dfis::PriceRangeSearchRequest> {
  [[nodiscard]] std::vector<std::byte> operator()(
      const dfis::PriceRangeSearchRequest &request) const;
};

template <>
struct Unmarshal<dfis::PriceRangeSearchRequest> {
  [[nodiscard]] std::pair<i64, std::optional<dfis::PriceRangeSearchRequest>>
  operator()(const std::span<const std::byte> &data) const;
};

template <>
struct Marshal<dfis::PriceRangeSearchResponse> {
  [[nodiscard]] std::vector<std::byte> operator()(
      const dfis::PriceRangeSearchResponse &response) const;
};

template <>
struct Unmarshal<dfis::PriceRangeSearchResponse> {
  [[nodiscard]] std::pair<i64, std::optional<dfis::PriceRangeSearchResponse>>
  operator()(const std::span<const std::byte> &data) const;
};

}  // namespace srpc

#endif  // DFIS_MESSAGES_FLIGHT_SEARCH_H_
