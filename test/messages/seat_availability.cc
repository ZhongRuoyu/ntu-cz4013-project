#include "messages/seat_availability.h"

#include <gtest/gtest.h>
#include <srpc/types/serialization.h>

#include "utils/rand.h"

using namespace dfis;

TEST(Message, MarshalAndUnmarshalSeatAvailabilityMonitoringRequests) {
  SeatAvailabilityMonitoringRequest req1{
      .id = MakeMessageIdentifier(),
      .identifier = 4013,
      .monitor_interval_sec = 60,
  };
  auto data1 = srpc::Marshal<SeatAvailabilityMonitoringRequest>{}(req1);
  auto res1 = srpc::Unmarshal<SeatAvailabilityMonitoringRequest>{}(data1);
  ASSERT_TRUE(res1.second.has_value());
  // NOLINTBEGIN(bugprone-unchecked-optional-access)
  ASSERT_EQ(req1.id, res1.second->id);
  ASSERT_EQ(req1.identifier, res1.second->identifier);
  ASSERT_EQ(req1.monitor_interval_sec, res1.second->monitor_interval_sec);
  // NOLINTEND(bugprone-unchecked-optional-access)
}

TEST(Message, MarshalAndUnmarshalSeatAvailabilityMonitoringResponses) {
  SeatAvailabilityMonitoringResponse resp1{
      .id = MakeMessageIdentifier(),
      .status_code = 1,
      .message = "Flight not found",
      .identifier = 4012,
      .monitor_end = 0,
  };
  auto data1 = srpc::Marshal<SeatAvailabilityMonitoringResponse>{}(resp1);
  auto res1 = srpc::Unmarshal<SeatAvailabilityMonitoringResponse>{}(data1);
  ASSERT_TRUE(res1.second.has_value());
  // NOLINTBEGIN(bugprone-unchecked-optional-access)
  ASSERT_EQ(resp1.id, res1.second->id);
  ASSERT_EQ(resp1.status_code, res1.second->status_code);
  ASSERT_EQ(resp1.message, res1.second->message);
  ASSERT_EQ(resp1.identifier, res1.second->identifier);
  ASSERT_EQ(resp1.monitor_end, res1.second->monitor_end);
  // NOLINTEND(bugprone-unchecked-optional-access)

  SeatAvailabilityMonitoringResponse resp2{
      .id = MakeMessageIdentifier(),
      .status_code = 0,
      .message = {},
      .identifier = 4013,
      .monitor_end = 1675612800,
  };
  auto data2 = srpc::Marshal<dfis::SeatAvailabilityMonitoringResponse>{}(resp2);
  auto res2 =
      srpc::Unmarshal<dfis::SeatAvailabilityMonitoringResponse>{}(data2);
  ASSERT_TRUE(res2.second.has_value());
  // NOLINTBEGIN(bugprone-unchecked-optional-access)
  ASSERT_EQ(resp2.id, res2.second->id);
  ASSERT_EQ(resp2.status_code, res2.second->status_code);
  ASSERT_EQ(resp2.message, res2.second->message);
  ASSERT_EQ(resp2.identifier, res2.second->identifier);
  ASSERT_EQ(resp2.monitor_end, res2.second->monitor_end);
  // NOLINTEND(bugprone-unchecked-optional-access)
}

TEST(Message, MarshalAndUnmarshalSeatAvailabilityCallbackRequests) {
  SeatAvailabilityCallbackRequest req1{
      .id = MakeMessageIdentifier(),
      .identifier = 4013,
      .seat_availability = 39,
  };
  auto data1 = srpc::Marshal<SeatAvailabilityCallbackRequest>{}(req1);
  auto res1 = srpc::Unmarshal<SeatAvailabilityCallbackRequest>{}(data1);
  ASSERT_TRUE(res1.second.has_value());
  // NOLINTBEGIN(bugprone-unchecked-optional-access)
  ASSERT_EQ(req1.id, res1.second->id);
  ASSERT_EQ(req1.identifier, res1.second->identifier);
  ASSERT_EQ(req1.seat_availability, res1.second->seat_availability);
  // NOLINTEND(bugprone-unchecked-optional-access)
}

TEST(Message, MarshalAndUnmarshalSeatAvailabilityCallbackResponses) {
  SeatAvailabilityCallbackResponse resp1{
      .status_code = 0,
  };
  auto data1 = srpc::Marshal<SeatAvailabilityCallbackResponse>{}(resp1);
  auto res1 = srpc::Unmarshal<SeatAvailabilityCallbackResponse>{}(data1);
  ASSERT_TRUE(res1.second.has_value());
  // NOLINTBEGIN(bugprone-unchecked-optional-access)
  ASSERT_EQ(resp1.id, res1.second->id);
  ASSERT_EQ(resp1.status_code, res1.second->status_code);
  // NOLINTEND(bugprone-unchecked-optional-access)
}
