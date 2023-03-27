#include "messages/flight_info.h"

#include <gtest/gtest.h>
#include <srpc/types/serialization.h>

#include "messages/flight.h"

using namespace dfis;

TEST(Message, MarshalAndUnmarshalFlightInfoRequests) {
  FlightInfoRequest req1{
      .identifier = 4013,
  };
  auto data1 = srpc::Marshal<FlightInfoRequest>{}(req1);
  auto res1 = srpc::Unmarshal<FlightInfoRequest>{}(data1);
  ASSERT_TRUE(res1.second.has_value());
  // NOLINTBEGIN(bugprone-unchecked-optional-access)
  ASSERT_EQ(req1.identifier, res1.second->identifier);
  // NOLINTEND(bugprone-unchecked-optional-access)
}

TEST(Message, MarshalAndUnmarshalFlightInfoResponses) {
  auto assert_eq = [](auto expected, auto actual) {
    ASSERT_EQ(expected.size(), actual.size());
    for (std::size_t i = 0; i < expected.size(); ++i) {
      ASSERT_EQ(expected[i], actual[i]);
    }
  };

  FlightInfoResponse resp1{
      .status_code = 1,
      .message = "Flight not found",
      .flight = {},
  };
  auto data1 = srpc::Marshal<FlightInfoResponse>{}(resp1);
  auto res1 = srpc::Unmarshal<FlightInfoResponse>{}(data1);
  ASSERT_TRUE(res1.second.has_value());
  // NOLINTBEGIN(bugprone-unchecked-optional-access)
  ASSERT_EQ(resp1.status_code, res1.second->status_code);
  ASSERT_EQ(resp1.message, res1.second->message);
  assert_eq(resp1.flight, res1.second->flight);
  // NOLINTEND(bugprone-unchecked-optional-access)

  FlightInfoResponse resp2{
      .status_code = 0,
      .message = {},
      .flight = {Flight{
          .identifier = 4013,
          .source = "Guangzhou",
          .destination = "Singapore",
          .departure_time = 1675526400,
          .airfare = 314.15,
          .seat_availability = 42,
      }},
  };
  auto data2 = srpc::Marshal<dfis::FlightInfoResponse>{}(resp2);
  auto res2 = srpc::Unmarshal<dfis::FlightInfoResponse>{}(data2);
  ASSERT_TRUE(res2.second.has_value());
  // NOLINTBEGIN(bugprone-unchecked-optional-access)
  ASSERT_EQ(resp2.status_code, res2.second->status_code);
  ASSERT_EQ(resp2.message, res2.second->message);
  assert_eq(resp2.flight, res2.second->flight);
  // NOLINTEND(bugprone-unchecked-optional-access)
}
