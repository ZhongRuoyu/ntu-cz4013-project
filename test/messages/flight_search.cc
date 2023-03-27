#include "messages/flight_search.h"

#include <gtest/gtest.h>
#include <srpc/types/serialization.h>

#include "messages/flight.h"

using namespace dfis;

TEST(Message, MarshalAndUnmarshalFlightSearchRequests) {
  FlightSearchRequest req1{
      .source = "Guangzhou",
      .destination = "Singapore",
  };
  auto data1 = srpc::Marshal<FlightSearchRequest>{}(req1);
  auto res1 = srpc::Unmarshal<FlightSearchRequest>{}(data1);
  ASSERT_TRUE(res1.second.has_value());
  // NOLINTBEGIN(bugprone-unchecked-optional-access)
  ASSERT_EQ(req1.source, res1.second->source);
  ASSERT_EQ(req1.destination, res1.second->destination);
  // NOLINTEND(bugprone-unchecked-optional-access)
}

TEST(Message, MarshalAndUnmarshalFlightSearchResponses) {
  auto assert_eq = [](auto expected, auto actual) {
    ASSERT_EQ(expected.size(), actual.size());
    for (std::size_t i = 0; i < expected.size(); ++i) {
      ASSERT_EQ(expected[i], actual[i]);
    }
  };

  FlightSearchResponse resp1{
      .status_code = 1,
      .message = "Flights not found",
      .flights = {},
  };
  auto data1 = srpc::Marshal<FlightSearchResponse>{}(resp1);
  auto res1 = srpc::Unmarshal<FlightSearchResponse>{}(data1);
  ASSERT_TRUE(res1.second.has_value());
  // NOLINTBEGIN(bugprone-unchecked-optional-access)
  ASSERT_EQ(resp1.status_code, res1.second->status_code);
  ASSERT_EQ(resp1.message, res1.second->message);
  assert_eq(resp1.flights, res1.second->flights);
  // NOLINTEND(bugprone-unchecked-optional-access)

  FlightSearchResponse resp2{
      .status_code = 0,
      .message = {},
      .flights = {Flight{
                      .identifier = 4013,
                      .source = "Guangzhou",
                      .destination = "Singapore",
                      .departure_time = 1675526400,
                      .airfare = 314.15,
                      .seat_availability = 42,
                  },
                  Flight{
                      .identifier = 4014,
                      .source = "Singapore",
                      .destination = "Brussels",
                      .departure_time = 1675612800,
                      .airfare = 926.53,
                      .seat_availability = 84,
                  }},
  };
  auto data2 = srpc::Marshal<dfis::FlightSearchResponse>{}(resp2);
  auto res2 = srpc::Unmarshal<dfis::FlightSearchResponse>{}(data2);
  ASSERT_TRUE(res2.second.has_value());
  // NOLINTBEGIN(bugprone-unchecked-optional-access)
  ASSERT_EQ(resp2.status_code, res2.second->status_code);
  ASSERT_EQ(resp2.message, res2.second->message);
  assert_eq(resp2.flights, res2.second->flights);
  // NOLINTEND(bugprone-unchecked-optional-access)
}
