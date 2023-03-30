#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include <srpc/network/datagram_client.h>
#include <srpc/network/datagram_server.h>
#include <srpc/network/tcp_ip.h>
#include <srpc/types/floats.h>
#include <srpc/types/integers.h>
#include <srpc/types/serialization.h>
#include <srpc/utils/result.h>

#include "messages/flight.h"
#include "messages/flight_info.h"
#include "messages/flight_search.h"
#include "messages/invocation_semantic.h"
#include "messages/seat_availability.h"
#include "messages/seat_reservation.h"
#include "utils/rand.h"
#include "utils/time.h"

using namespace dfis;

namespace {

std::unordered_map<srpc::i32, Flight> ReadFlightsFromFile(
    const std::string &filename) {
  std::unordered_map<srpc::i32, Flight> flights;
  std::ifstream in{filename};
  for (;;) {
    std::string line;
    if (!std::getline(in, line)) {
      break;
    }
    std::istringstream ss{line};
    Flight flight;
    ss >> flight.identifier;
    ss >> flight.source;
    ss >> flight.destination;
    ss >> flight.departure_time;
    ss >> flight.airfare;
    ss >> flight.seat_availability;
    std::clog << "Info: Read flight " << flight << std::endl;
    flights.emplace(flight.identifier, std::move(flight));
  }
  return flights;
}

std::ostream &operator<<(std::ostream &ostream,
                         const srpc::SocketAddress &addr) {
  switch (addr.protocol) {
    case srpc::kIPv4: return ostream << addr.address << ":" << addr.port;
    case srpc::kIPv6:
      return ostream << "[" << addr.address << "]:" << addr.port;
  }
  assert(false);
}

void RandomDelay(srpc::i64 from_ms = 0, srpc::i64 to_ms = 200) {
  static std::random_device rand;
  auto sleep_ms = std::chrono::milliseconds(
      std::uniform_int_distribution<std::chrono::milliseconds::rep>{
          from_ms, to_ms}(rand));
  std::clog << "Info: Simulating delay for " << sleep_ms.count() << "ms"
            << std::endl;
  std::this_thread::sleep_for(sleep_ms);
}

bool RandomLoss(srpc::f32 loss_prob = 0.1) {
  static std::random_device rand;
  return std::uniform_real_distribution<srpc::f32>{0.0, 1.0}(rand) < loss_prob;
}

void SendSeatAvailabilityCallbackRequest(const srpc::SocketAddress &to_addr,
                                         SeatAvailabilityCallbackRequest req) {
  auto client_res = srpc::DatagramClient::New(to_addr.address, to_addr.port);
  if (!client_res.OK()) {
    std::cerr
        << "Error: Unable to create client for seat availability callback to "
        << to_addr << ": " << client_res.Error() << std::endl;
    return;
  }

  auto client = std::move(client_res.Value());
  req.id = MakeMessageIdentifier();

  std::vector<std::byte> resp_bytes;
  constexpr int retry_times = 3;
  int attempt = 0;
  while (attempt <= retry_times) {
    auto resp_bytes_res = client->SendAndReceive(
        srpc::Marshal<SeatAvailabilityCallbackRequest>{}(req));
    if (!resp_bytes_res.OK()) {
      std::clog << "Error: Unable to receive response for seat availability "
                   "callback to "
                << to_addr << ": " << resp_bytes_res.Error() << std::endl;
      if (++attempt <= retry_times) {
        std::clog << "Info: Retrying; attempt " << attempt << std::endl;
      }
      continue;
    }
    resp_bytes = std::move(resp_bytes_res.Value());
    break;
  }
  if (attempt > retry_times) {
    std::cerr << "Error: Unable to receive response for seat availability "
                 "callback to "
              << to_addr << " after " << retry_times << " retries" << std::endl;
    return;
  }

  auto resp_res =
      srpc::Unmarshal<SeatAvailabilityCallbackResponse>{}(resp_bytes);
  if (!resp_res.second.has_value()) {
    std::cerr
        << "Error: Invalid response received for seat availability callback to "
        << to_addr << std::endl;
    return;
  }

  auto resp = *resp_res.second;
  if (resp.status_code != 0) {
    std::cerr
        << "Error: Received non-zero seat availability callback status code "
        << resp.status_code << " from " << to_addr << std::endl;
    return;
  }

  std::clog
      << "Info: Successfully received seat availability callback response from "
      << to_addr << std::endl;
}

std::optional<std::vector<std::byte>> Serve(
    InvocationSemantic semantic,
    const std::unordered_map<srpc::i32, Flight> &flights_input,
    const srpc::SocketAddress &from_addr,
    srpc::Result<std::vector<std::byte>> req_data_res) {
  struct Callback {
    srpc::SocketAddress to_addr;
    std::chrono::system_clock::time_point monitor_end;
  };

  struct Reservation {
    srpc::i32 identifier;
    srpc::i32 seats;
  };

  static bool initialised = false;
  static std::unordered_map<srpc::i32, Flight> flights;
  static std::unordered_map<srpc::i32, std::vector<Callback>> callbacks;
  static std::unordered_map<srpc::u64, Reservation> reservations;

  if (!initialised) {
    flights = flights_input;
    initialised = true;
  }

  if (!req_data_res.OK()) {
    std::cerr << "Error: Could not receive request from " << from_addr << ": "
              << req_data_res.Error() << std::endl;
    return {};
  }
  auto req_data = std::move(req_data_res.Value());

  {
    auto req_res = srpc::Unmarshal<FlightSearchRequest>{}(req_data);
    if (req_res.second.has_value()) {
      static std::unordered_map<
          srpc::u64, std::pair<FlightSearchRequest, FlightSearchResponse>>
          history;

      auto req = std::move(*req_res.second);
      std::clog << "Info: Received flight search request from " << from_addr
                << ": " << req << std::endl;

      auto req_lost = RandomLoss(0.1);
      auto res_lost = RandomLoss(0.2);
      if (req_lost) {
        std::clog << "Info: Request " << req.id << " is simulated to be lost"
                  << std::endl;
        return {};
      }
      RandomDelay();

      if (semantic == InvocationSemantic::kAtMostOnce &&
          history.contains(req.id)) {
        std::clog << "Info: " << req.id << " is a duplicate request"
                  << std::endl;
        auto res = history[req.id].second;
        if (res_lost) {
          std::clog << "Info: Response " << res.id << " is simulated to be lost"
                    << std::endl;
          return {};
        }
        RandomDelay();
        std::clog << "Info: Returning saved response " << res << std::endl;
        return srpc::Marshal<FlightSearchResponse>{}(res);
      }

      FlightSearchResponse res;
      std::vector<Flight> results;
      for (const auto &flight : flights) {
        if (flight.second.source == req.source &&
            flight.second.destination == req.destination) {
          results.emplace_back(flight.second);
        }
      }
      std::sort(results.begin(), results.end(),
                [](const auto &lhs, const auto &rhs) {
                  return lhs.identifier < rhs.identifier;
                });
      if (results.empty()) {
        res.id = req.id;
        res.status_code = 1;
        res.message = "Flights not found";
        res.flights = std::move(results);
      } else {
        res.id = req.id;
        res.status_code = 0;
        res.message = {};
        res.flights = std::move(results);
      }
      if (semantic == InvocationSemantic::kAtMostOnce) {
        history[req.id] = {req, res};
      }

      if (res_lost) {
        std::clog << "Info: Response " << res.id << " is simulated to be lost"
                  << std::endl;
        return {};
      }
      RandomDelay();

      std::clog << "Info: Sending flight search response to " << from_addr
                << ": " << res << std::endl;
      return srpc::Marshal<FlightSearchResponse>{}(res);
    }
  }

  {
    auto req_res = srpc::Unmarshal<FlightInfoRequest>{}(req_data);
    if (req_res.second.has_value()) {
      static std::unordered_map<
          srpc::u64, std::pair<FlightInfoRequest, FlightInfoResponse>>
          history;

      auto req = *req_res.second;
      std::clog << "Info: Received flight info request from " << from_addr
                << ": " << req << std::endl;

      auto req_lost = RandomLoss(0.1);
      auto res_lost = RandomLoss(0.2);
      if (req_lost) {
        std::clog << "Info: Request " << req.id << " is simulated to be lost"
                  << std::endl;
        return {};
      }
      RandomDelay();

      if (semantic == InvocationSemantic::kAtMostOnce &&
          history.contains(req.id)) {
        std::clog << "Info: " << req.id << " is a duplicate request"
                  << std::endl;
        auto res = history[req.id].second;
        if (res_lost) {
          std::clog << "Info: Response " << res.id << " is simulated to be lost"
                    << std::endl;
          return {};
        }
        RandomDelay();
        std::clog << "Info: Returning saved response " << res << std::endl;
        return srpc::Marshal<FlightInfoResponse>{}(res);
      }

      FlightInfoResponse res;
      if (!flights.contains(req.identifier)) {
        res.id = req.id;
        res.status_code = 1;
        res.message = "Flight not found";
        res.flight = {};
      } else {
        res.id = req.id;
        res.status_code = 0;
        res.message = {};
        res.flight = {flights[req.identifier]};
      }
      if (semantic == InvocationSemantic::kAtMostOnce) {
        history[req.id] = {req, res};
      }

      if (res_lost) {
        std::clog << "Info: Response " << res.id << " is simulated to be lost"
                  << std::endl;
        return {};
      }
      RandomDelay();

      std::clog << "Info: Sending flight info response to " << from_addr << ": "
                << res << std::endl;
      return srpc::Marshal<FlightInfoResponse>{}(res);
    }
  }

  {
    auto req_res = srpc::Unmarshal<SeatReservationRequest>{}(req_data);
    if (req_res.second.has_value()) {
      static std::unordered_map<
          srpc::u64, std::pair<SeatReservationRequest, SeatReservationResponse>>
          history;

      auto req = *req_res.second;
      std::clog << "Info: Received seat reservation request from " << from_addr
                << ": " << req << std::endl;

      auto req_lost = RandomLoss(0.1);
      auto res_lost = RandomLoss(0.2);
      if (req_lost) {
        std::clog << "Info: Request " << req.id << " is simulated to be lost"
                  << std::endl;
        return {};
      }
      RandomDelay();

      if (semantic == InvocationSemantic::kAtMostOnce &&
          history.contains(req.id)) {
        std::clog << "Info: " << req.id << " is a duplicate request"
                  << std::endl;
        auto res = history[req.id].second;
        if (res_lost) {
          std::clog << "Info: Response " << res.id << " is simulated to be lost"
                    << std::endl;
          return {};
        }
        RandomDelay();
        std::clog << "Info: Returning saved response " << res << std::endl;
        return srpc::Marshal<SeatReservationResponse>{}(res);
      }

      SeatReservationResponse res;
      if (!flights.contains(req.identifier)) {
        res.id = req.id;
        res.status_code = 1;
        res.message = "Flight not found";
        res.identifier = req.identifier;
        res.seats = 0;
      } else {
        auto &flight = flights[req.identifier];
        // Note: for simplicity, race condition is not handled.
        if (flight.seat_availability < req.seats) {
          res.id = req.id;
          res.status_code = 2;
          res.message = "No enough seats";
          res.identifier = req.identifier;
          res.seats = 0;
        } else {
          res.id = req.id;
          flight.seat_availability -= req.seats;
          std::clog << "Info: Flight " << flight.identifier << " now has "
                    << flight.seat_availability << " seat(s) left" << std::endl;
          res.status_code = 0;
          res.message = {};
          res.identifier = req.identifier;
          res.seats = req.seats;
          reservations.emplace(req.id, Reservation{
                                           .identifier = req.identifier,
                                           .seats = req.seats,
                                       });
          // Note: for simplicity, expired callbacks are not handled.
          auto now = std::chrono::system_clock::now();
          SeatAvailabilityCallbackRequest cb_req{
              .identifier = flight.identifier,
              .seat_availability = flight.seat_availability,
          };
          for (const auto &callback : callbacks[req.identifier]) {
            if (now < callback.monitor_end) {
              std::clog << "Info: Sending callback " << cb_req << " to "
                        << callback.to_addr << std::endl;
              std::thread{SendSeatAvailabilityCallbackRequest, callback.to_addr,
                          cb_req}
                  .detach();
            } else {
              std::clog << "Info: Callback to " << callback.to_addr
                        << " is expired" << std::endl;
            }
          }
        }
      }
      if (semantic == InvocationSemantic::kAtMostOnce) {
        history[req.id] = {req, res};
      }

      if (res_lost) {
        std::clog << "Info: Response " << res.id << " is simulated to be lost"
                  << std::endl;
        return {};
      }
      RandomDelay();

      std::clog << "Info: Sending seat reservation response to " << from_addr
                << ": " << res << std::endl;
      return srpc::Marshal<SeatReservationResponse>{}(res);
    }
  }

  {
    auto req_res =
        srpc::Unmarshal<SeatAvailabilityMonitoringRequest>{}(req_data);
    if (req_res.second.has_value()) {
      static std::unordered_map<srpc::u64,
                                std::pair<SeatAvailabilityMonitoringRequest,
                                          SeatAvailabilityMonitoringResponse>>
          history;

      auto req = *req_res.second;
      std::clog << "Info: Received seat availability monitoring request from "
                << from_addr << ": " << req << std::endl;

      auto req_lost = RandomLoss(0.1);
      auto res_lost = RandomLoss(0.2);
      if (req_lost) {
        std::clog << "Info: Request " << req.id << " is simulated to be lost"
                  << std::endl;
        return {};
      }
      RandomDelay();

      if (semantic == InvocationSemantic::kAtMostOnce &&
          history.contains(req.id)) {
        std::clog << "Info: " << req.id << " is a duplicate request"
                  << std::endl;
        auto res = history[req.id].second;
        if (res_lost) {
          std::clog << "Info: Response " << res.id << " is simulated to be lost"
                    << std::endl;
          return {};
        }
        RandomDelay();
        std::clog << "Info: Returning saved response " << res << std::endl;
        return srpc::Marshal<SeatAvailabilityMonitoringResponse>{}(res);
      }

      SeatAvailabilityMonitoringResponse res;
      if (!flights.contains(req.identifier)) {
        res.id = req.id;
        res.status_code = 1;
        res.message = "Flight not found";
        res.identifier = req.identifier;
        res.monitor_end = 0;
      } else {
        if (!callbacks.contains(req.identifier)) {
          callbacks[req.identifier] = {};
        }
        auto monitor_end = std::chrono::system_clock::now() +
                           std::chrono::seconds{req.monitor_interval_sec};
        auto monitor_end_ts = std::chrono::duration_cast<std::chrono::seconds>(
                                  monitor_end.time_since_epoch())
                                  .count();
        srpc::SocketAddress to_addr{
            .protocol = from_addr.protocol,
            .address = from_addr.address,
            .port = req.port,
        };
        std::clog << "Info: Monitoring seat availability of flight "
                  << req.identifier << " for " << to_addr << " until "
                  << FormatTimestamp(monitor_end_ts) << std::endl;
        callbacks[req.identifier].push_back(Callback{
            .to_addr = to_addr,
            .monitor_end = monitor_end,
        });
        res.id = req.id;
        res.status_code = 0;
        res.message = {};
        res.identifier = req.identifier;
        res.monitor_end = monitor_end_ts;
      }
      if (semantic == InvocationSemantic::kAtMostOnce) {
        history[req.id] = {req, res};
      }

      if (res_lost) {
        std::clog << "Info: Response " << res.id << " is simulated to be lost"
                  << std::endl;
        return {};
      }
      RandomDelay();

      std::clog << "Info: Sending seat availability monitoring response to "
                << from_addr << ": " << res << std::endl;
      return srpc::Marshal<SeatAvailabilityMonitoringResponse>{}(res);
    }
  }

  {
    auto req_res = srpc::Unmarshal<PriceRangeSearchRequest>{}(req_data);
    if (req_res.second.has_value()) {
      static std::unordered_map<srpc::u64, std::pair<PriceRangeSearchRequest,
                                                     PriceRangeSearchResponse>>
          history;

      auto req = *req_res.second;
      std::clog << "Info: Received price range search request from "
                << from_addr << ": " << req << std::endl;

      auto req_lost = RandomLoss(0.1);
      auto res_lost = RandomLoss(0.2);
      if (req_lost) {
        std::clog << "Info: Request " << req.id << " is simulated to be lost"
                  << std::endl;
        return {};
      }
      RandomDelay();

      if (semantic == InvocationSemantic::kAtMostOnce &&
          history.contains(req.id)) {
        std::clog << "Info: " << req.id << " is a duplicate request"
                  << std::endl;
        auto res = history[req.id].second;
        if (res_lost) {
          std::clog << "Info: Response " << res.id << " is simulated to be lost"
                    << std::endl;
          return {};
        }
        RandomDelay();
        std::clog << "Info: Returning saved response " << res << std::endl;
        return srpc::Marshal<PriceRangeSearchResponse>{}(res);
      }

      PriceRangeSearchResponse res;
      std::vector<Flight> results;
      for (const auto &flight : flights) {
        if (flight.second.airfare >= req.from &&
            flight.second.airfare <= req.to) {
          results.emplace_back(flight.second);
        }
      }
      std::sort(
          results.begin(), results.end(), [](const auto &lhs, const auto &rhs) {
            return lhs.airfare != rhs.airfare ? lhs.airfare < rhs.airfare
                                              : lhs.identifier < rhs.identifier;
          });
      if (results.empty()) {
        res.id = req.id;
        res.status_code = 1;
        res.message = "Flights not found";
        res.flights = std::move(results);
      } else {
        res.id = req.id;
        res.status_code = 0;
        res.message = {};
        res.flights = std::move(results);
      }
      if (semantic == InvocationSemantic::kAtMostOnce) {
        history[req.id] = {req, res};
      }

      if (res_lost) {
        std::clog << "Info: Response " << res.id << " is simulated to be lost"
                  << std::endl;
        return {};
      }
      RandomDelay();

      std::clog << "Info: Sending price range search response to " << from_addr
                << ": " << res << std::endl;
      return srpc::Marshal<PriceRangeSearchResponse>{}(res);
    }
  }

  {
    auto req_res =
        srpc::Unmarshal<SeatReservationCancellationRequest>{}(req_data);
    if (req_res.second.has_value()) {
      static std::unordered_map<srpc::u64,
                                std::pair<SeatReservationCancellationRequest,
                                          SeatReservationCancellationResponse>>
          history;

      auto req = *req_res.second;
      std::clog << "Info: Received seat reservation cancellation request from "
                << from_addr << ": " << req << std::endl;

      auto req_lost = RandomLoss(0.1);
      auto res_lost = RandomLoss(0.2);
      if (req_lost) {
        std::clog << "Info: Request " << req.id << " is simulated to be lost"
                  << std::endl;
        return {};
      }
      RandomDelay();

      if (semantic == InvocationSemantic::kAtMostOnce &&
          history.contains(req.id)) {
        std::clog << "Info: " << req.id << " is a duplicate request"
                  << std::endl;
        auto res = history[req.id].second;
        if (res_lost) {
          std::clog << "Info: Response " << res.id << " is simulated to be lost"
                    << std::endl;
          return {};
        }
        RandomDelay();
        std::clog << "Info: Returning saved response " << res << std::endl;
        return srpc::Marshal<SeatReservationCancellationResponse>{}(res);
      }

      SeatReservationCancellationResponse res;
      if (!reservations.contains(req.reservation_req_id)) {
        res.id = req.id;
        res.status_code = 1;
        res.message = "Reservation not found";
        res.identifier = 0;
        res.seats = 0;
      } else {
        auto &reservation = reservations[req.reservation_req_id];
        // Note: for simplicity, race condition is not handled.
        if (reservation.identifier != req.identifier) {
          res.id = req.id;
          res.status_code = 2;
          res.message = "Identifier mismatch";
          res.identifier = 0;
          res.seats = 0;
        } else {
          auto &flight = flights[req.identifier];
          if (req.seats > reservation.seats) {
            res.id = req.id;
            res.status_code = 3;
            res.message = "Too many seats to cancel";
            res.identifier = 0;
            res.seats = 0;
          } else {
            res.id = req.id;
            reservation.seats -= req.seats;
            std::clog << "Info: Reservation " << req.reservation_req_id
                      << " now has " << reservation.seats << " seat(s) left"
                      << std::endl;
            flight.seat_availability += req.seats;
            std::clog << "Info: Flight " << flight.identifier << " now has "
                      << flight.seat_availability << " seat(s) left"
                      << std::endl;
            res.status_code = 0;
            res.message = {};
            res.identifier = req.identifier;
            res.seats = req.seats;
            // Note: for simplicity, expired callbacks are not handled.
            auto now = std::chrono::system_clock::now();
            SeatAvailabilityCallbackRequest cb_req{
                .identifier = flight.identifier,
                .seat_availability = flight.seat_availability,
            };
            for (const auto &callback : callbacks[req.identifier]) {
              if (now < callback.monitor_end) {
                std::clog << "Info: Sending callback " << cb_req << " to "
                          << callback.to_addr << std::endl;
                std::thread{SendSeatAvailabilityCallbackRequest,
                            callback.to_addr, cb_req}
                    .detach();
              } else {
                std::clog << "Info: Callback to " << callback.to_addr
                          << " is expired" << std::endl;
              }
            }
          }
        }
      }
      if (semantic == InvocationSemantic::kAtMostOnce) {
        history[req.id] = {req, res};
      }

      if (res_lost) {
        std::clog << "Info: Response " << res.id << " is simulated to be lost"
                  << std::endl;
        return {};
      }
      RandomDelay();

      std::clog << "Info: Sending seat reservation cancellation response to "
                << from_addr << ": " << res << std::endl;
      return srpc::Marshal<SeatReservationCancellationResponse>{}(res);
    }
  }

  std::cerr << "Error: Could not determine type of request sent from "
            << from_addr << std::endl;
  return {};
}

}  // namespace

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0]
              << " (at-least-once | at-most-once) <port> <flights-input>"
              << std::endl;
    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    std::exit(EXIT_FAILURE);
  }

  InvocationSemantic semantic;
  auto port = std::atoi(argv[2]);
  std::string flights_input = argv[3];
  if (std::strcmp(argv[1], "at-least-once") == 0) {
    semantic = InvocationSemantic::kAtLeastOnce;
    std::clog << "Info: At-least-once semantic is used" << std::endl;
  } else if (std::strcmp(argv[1], "at-most-once") == 0) {
    semantic = InvocationSemantic::kAtMostOnce;
    std::clog << "Info: At-most-once semantic is used" << std::endl;
  } else {
    std::cerr << "Error: Invalid invocation semantic: " << argv[1] << std::endl;
    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    std::exit(EXIT_FAILURE);
  }

  auto flights = ReadFlightsFromFile(flights_input);

  auto server_res = srpc::DatagramServer::New(port);
  if (!server_res.OK()) {
    std::cerr << server_res.Error() << std::endl;
    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    std::exit(EXIT_FAILURE);
  }

  auto server = std::move(server_res.Value());
  std::clog << "Info: Server listening at port " << port << std::endl;
  server->Listen([semantic, flights](const auto &from_addr, auto req_data_res) {
    return Serve(semantic, flights, from_addr, req_data_res);
  });
}
