#ifndef DFIS_MESSAGES_MESSAGE_TYPE_H_
#define DFIS_MESSAGES_MESSAGE_TYPE_H_

#include <srpc/types/integers.h>

namespace dfis {

enum class MessageType : srpc::i32 {
  kFlightSearchRequest = 1,
  kFlightSearchResponse = 2,
  kFlightInfoRequest = 3,
  kFlightInfoResponse = 4,
  kSeatReservationRequest = 5,
  kSeatReservationResponse = 6,
  kSeatAvailabilityMonitoringRequest = 7,
  kSeatAvailabilityMonitoringResponse = 8,
  kSeatAvailabilityCallbackRequest = 9,
  kSeatAvailabilityCallbackResponse = 10,
};

}  // namespace dfis

#endif  // DFIS_MESSAGES_MESSAGE_TYPE_H_
