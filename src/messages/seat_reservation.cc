#include "messages/seat_reservation.h"

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

#include "messages/message_type.h"

namespace dfis {

std::ostream &operator<<(std::ostream &os,
                         const SeatReservationRequest &request) {
  os << request.identifier << " (" << request.seats << ")";
  return os;
}

std::ostream &operator<<(std::ostream &os,
                         const SeatReservationResponse &response) {
  if (response.seats == 0) {
    os << "Error: " << response.message;
  } else {
    os << response.identifier << ": " << response.seats << " seat(s) reserved";
  }
  return os;
}

}  // namespace dfis

namespace srpc {

[[nodiscard]] std::vector<std::byte>
Marshal<dfis::SeatReservationRequest>::operator()(
    const dfis::SeatReservationRequest &request) const {
  std::vector<std::byte> data(sizeof(i32));

  Marshal<i32>{}(static_cast<i32>(dfis::SeatReservationRequest::kMessageType),
                 std::span<std::byte, sizeof(i32)>{data.data(),
                                                   data.data() + sizeof(i32)});

  auto identifier = Marshal<i32>{}(request.identifier);
  data.insert(data.end(), identifier.begin(), identifier.end());

  auto seats = Marshal<i32>{}(request.seats);
  data.insert(data.end(), seats.begin(), seats.end());

  return data;
}

[[nodiscard]] std::pair<i64, std::optional<dfis::SeatReservationRequest>>
Unmarshal<dfis::SeatReservationRequest>::operator()(
    const std::span<const std::byte> &data) const {
  if (data.size() < sizeof(i32)) {
    return {0, {}};
  }

  auto message_type = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data(), data.data() + sizeof(i32)});
  if (dfis::MessageType{message_type} !=
      dfis::SeatReservationRequest::kMessageType) {
    return {0, {}};
  }

  i64 p = sizeof(i32);

  if (p + sizeof(i32) > data.size()) {
    return {0, {}};
  }
  auto identifier = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data() + p, data.data() + p + sizeof(i32)});
  p += sizeof(i32);

  if (p + sizeof(i32) > data.size()) {
    return {0, {}};
  }
  auto seats = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data() + p, data.data() + p + sizeof(i32)});
  p += sizeof(i32);

  return {p, dfis::SeatReservationRequest{
                 .identifier = identifier,
                 .seats = seats,
             }};
}

[[nodiscard]] std::vector<std::byte>
Marshal<dfis::SeatReservationResponse>::operator()(
    const dfis::SeatReservationResponse &response) const {
  std::vector<std::byte> data(sizeof(i32));

  Marshal<i32>{}(static_cast<i32>(dfis::SeatReservationResponse::kMessageType),
                 std::span<std::byte, sizeof(i32)>{data.data(),
                                                   data.data() + sizeof(i32)});

  auto message = Marshal<std::string>{}(response.message);
  data.insert(data.end(), message.begin(), message.end());

  auto identifier = Marshal<i32>{}(response.identifier);
  data.insert(data.end(), identifier.begin(), identifier.end());

  auto seats = Marshal<i32>{}(response.seats);
  data.insert(data.end(), seats.begin(), seats.end());

  return data;
}

[[nodiscard]] std::pair<i64, std::optional<dfis::SeatReservationResponse>>
Unmarshal<dfis::SeatReservationResponse>::operator()(
    const std::span<const std::byte> &data) const {
  if (data.size() < sizeof(i32)) {
    return {0, {}};
  }

  auto message_type = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data(), data.data() + sizeof(i32)});
  if (dfis::MessageType{message_type} !=
      dfis::SeatReservationResponse::kMessageType) {
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

  if (p + sizeof(i32) > data.size()) {
    return {0, {}};
  }
  auto identifier = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data() + p, data.data() + p + sizeof(i32)});
  p += sizeof(i32);

  if (p + sizeof(i32) > data.size()) {
    return {0, {}};
  }
  auto seats = Unmarshal<i32>{}(std::span<const std::byte, sizeof(i32)>{
      data.data() + p, data.data() + p + sizeof(i32)});
  p += sizeof(i32);

  return {p, dfis::SeatReservationResponse{
                 .message = message,
                 .identifier = identifier,
                 .seats = seats,
             }};
}

}  // namespace srpc
