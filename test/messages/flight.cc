#include "messages/flight.h"

#include <gtest/gtest.h>
#include <srpc/types/serialization.h>

using namespace dfis;

TEST(Message, MarshalAndUnmarshalFlights) {
  Flight flight1{
      .identifier = 4013,
      .source = "Guangzhou",
      .destination = "Singapore",
      .departure_time = 1675526400,
      .airfare = 314.15,
      .seat_availability = 42,
  };
  auto data1 = srpc::Marshal<Flight>{}(flight1);
  auto res1 = srpc::Unmarshal<Flight>{}(data1);
  ASSERT_TRUE(res1.second.has_value());
  // NOLINTBEGIN(bugprone-unchecked-optional-access)
  ASSERT_EQ(flight1.identifier, res1.second->identifier);
  ASSERT_EQ(flight1.source, res1.second->source);
  ASSERT_EQ(flight1.destination, res1.second->destination);
  ASSERT_EQ(flight1.departure_time, res1.second->departure_time);
  ASSERT_EQ(flight1.airfare, res1.second->airfare);
  ASSERT_EQ(flight1.seat_availability, res1.second->seat_availability);
  // NOLINTEND(bugprone-unchecked-optional-access)
}
