#include "messages/seat_reservation.h"

#include <gtest/gtest.h>
#include <srpc/types/serialization.h>

using namespace dfis;

TEST(Message, MarshalAndUnmarshalSeatReservationRequests) {
  SeatReservationRequest req1{
      .identifier = 4013,
      .seats = 3,
  };
  auto data1 = srpc::Marshal<SeatReservationRequest>{}(req1);
  auto res1 = srpc::Unmarshal<SeatReservationRequest>{}(data1);
  ASSERT_TRUE(res1.second.has_value());
  // NOLINTBEGIN(bugprone-unchecked-optional-access)
  ASSERT_EQ(req1.identifier, res1.second->identifier);
  ASSERT_EQ(req1.identifier, res1.second->identifier);
  // NOLINTEND(bugprone-unchecked-optional-access)
}

TEST(Message, MarshalAndUnmarshalSeatReservationResponses) {
  SeatReservationResponse resp1{
      .status_code = 1,
      .message = "Flight not found",
      .identifier = 4012,
      .seats = 0,
  };
  auto data1 = srpc::Marshal<SeatReservationResponse>{}(resp1);
  auto res1 = srpc::Unmarshal<SeatReservationResponse>{}(data1);
  ASSERT_TRUE(res1.second.has_value());
  // NOLINTBEGIN(bugprone-unchecked-optional-access)
  ASSERT_EQ(resp1.status_code, res1.second->status_code);
  ASSERT_EQ(resp1.message, res1.second->message);
  ASSERT_EQ(resp1.identifier, res1.second->identifier);
  ASSERT_EQ(resp1.seats, res1.second->seats);
  // NOLINTEND(bugprone-unchecked-optional-access)

  SeatReservationResponse resp2{
      .status_code = 0,
      .message = {},
      .identifier = 4013,
      .seats = 3,
  };
  auto data2 = srpc::Marshal<dfis::SeatReservationResponse>{}(resp2);
  auto res2 = srpc::Unmarshal<dfis::SeatReservationResponse>{}(data2);
  ASSERT_TRUE(res2.second.has_value());
  // NOLINTBEGIN(bugprone-unchecked-optional-access)
  ASSERT_EQ(resp2.status_code, res2.second->status_code);
  ASSERT_EQ(resp2.message, res2.second->message);
  ASSERT_EQ(resp2.identifier, res2.second->identifier);
  ASSERT_EQ(resp2.seats, res2.second->seats);
  // NOLINTEND(bugprone-unchecked-optional-access)
}
