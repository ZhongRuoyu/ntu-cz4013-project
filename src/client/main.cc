#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>

#include <srpc/network/datagram_client.h>
#include <srpc/network/datagram_server.h>
#include <srpc/network/tcp_ip.h>
#include <srpc/types/integers.h>
#include <srpc/types/serialization.h>

#include "messages/flight_info.h"
#include "messages/flight_search.h"
#include "messages/invocation_semantic.h"
#include "messages/seat_availability.h"
#include "messages/seat_reservation.h"
#include "utils/rand.h"

using namespace dfis;

[[noreturn]] static void Bye() {
  std::cout << "Bye-bye!" << std::endl;
  // NOLINTNEXTLINE(concurrency-mt-unsafe)
  std::exit(EXIT_SUCCESS);
}

template <typename T>
static T PromptForInput(const std::string &prompt,
                        const std::string &retry_prompt) {
  std::cout << prompt << std::flush;
  for (;;) {
    std::string line;
    if (!std::getline(std::cin, line)) {
      Bye();
    }
    std::istringstream ss{line};
    T value;
    if (ss >> value && (ss >> std::ws).eof()) {
      return value;
    }
    std::cout << retry_prompt << std::flush;
  }
}

template <typename Req, typename Res>
static std::optional<Res> SendAndReceive(const std::string &server_addr,
                                         srpc::u16 server_port, Req req) {
  auto client_res = srpc::DatagramClient::New(server_addr, server_port);
  if (!client_res.OK()) {
    std::cerr << "Error: Unable to create client: " << client_res.Error()
              << std::endl;
    return {};
  }

  auto client = std::move(client_res.Value());
  req.id = MakeMessageIdentifier();
  auto resp_bytes_res = client->SendAndReceive(srpc::Marshal<Req>{}(req));
  if (!resp_bytes_res.OK()) {
    std::cerr << "Error: Unable to receive response: " << resp_bytes_res.Error()
              << std::endl;
    return {};
  }
  if (!resp_bytes_res.Error().empty()) {
    std::cerr << "Info: " << resp_bytes_res.Error() << std::endl;
  }

  auto resp_bytes = std::move(resp_bytes_res.Value());
  auto resp_res = srpc::Unmarshal<Res>{}(resp_bytes);
  if (!resp_res.second.has_value()) {
    std::cerr << "Error: Failed to unmarshal response" << std::endl;
    return {};
  }

  auto resp = std::move(*resp_res.second);
  std::cout << "Received response:\n" << resp << std::endl;
  return resp;
}

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0]
              << " (at-least-once | at-most-once) <server-addr> <server-port>"
              << std::endl;
    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    std::exit(EXIT_FAILURE);
  }

  InvocationSemantic semantic;
  std::string server_addr = argv[2];
  auto server_port = static_cast<srpc::u16>(std::atoi(argv[3]));
  if (std::strcmp(argv[1], "at-least-once") == 0) {
    semantic = InvocationSemantic::kAtLeastOnce;
  } else if (std::strcmp(argv[1], "at-most-once") == 0) {
    semantic = InvocationSemantic::kAtMostOnce;
  } else {
    std::cerr << "Error: Invalid invocation semantic: " << argv[1] << std::endl;
    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    std::exit(EXIT_FAILURE);
  }

  bool first_launch = true;
  for (;;) {
    if (first_launch) {
      first_launch = false;
    } else {
      std::cout << std::endl;
    }
    std::cout << R"SEL(Selections available:
0. Exit
1. Flight search
2. Flight info
3. Seat reservation
4. Seat availability monitoring
Enter selection: )SEL"
              << std::flush;
    std::string line;
    if (!std::getline(std::cin, line) || line == "0") {
      Bye();
    }
    if (line == "1") {
      FlightSearchRequest req;
      req.source = PromptForInput<std::string>("Enter source: ",
                                               "Please enter a string: ");
      req.destination = PromptForInput<std::string>("Enter destination: ",
                                                    "Please enter a string: ");
      SendAndReceive<FlightSearchRequest, FlightSearchResponse>(
          server_addr, server_port, req);
      continue;
    }
    if (line == "2") {
      FlightInfoRequest req;
      req.identifier = PromptForInput<srpc::i32>("Enter identifier: ",
                                                 "Please enter an integer: ");
      SendAndReceive<FlightInfoRequest, FlightInfoResponse>(server_addr,
                                                            server_port, req);
      continue;
    }
    if (line == "3") {
      SeatReservationRequest req;
      req.identifier = PromptForInput<srpc::i32>("Enter identifier: ",
                                                 "Please enter an integer: ");
      req.seats = PromptForInput<srpc::i32>(
          "Enter number of seats to reserve: ", "Please enter an integer: ");
      SendAndReceive<SeatReservationRequest, SeatReservationResponse>(
          server_addr, server_port, req);
      continue;
    }
    if (line == "4") {
      SeatAvailabilityMonitoringRequest req;
      req.identifier = PromptForInput<srpc::i32>("Enter identifier: ",
                                                 "Please enter an integer: ");
      req.port = PromptForInput<srpc::u16>("Enter port number: ",
                                           "Please enter an integer: ");
      req.monitor_interval_sec = PromptForInput<srpc::i32>(
          "Enter monitor interval in seconds: ", "Please enter an integer: ");
      auto res = SendAndReceive<SeatAvailabilityMonitoringRequest,
                                SeatAvailabilityMonitoringResponse>(
          server_addr, server_port, req);
      if (!res.has_value()) {
        continue;
      }
      auto server_res = srpc::DatagramServer::New(req.port);
      if (!server_res.OK()) {
        std::cerr << "Failed to create server for callback listening: "
                  << server_res.Error() << std::endl;
        continue;
      }
      auto server = std::move(server_res.Value());
      std::thread{
          [semantic](auto server) {
            server->Listen([semantic](const srpc::SocketAddress &from_addr,
                                      std::vector<std::byte> req_data)
                               -> std::optional<std::vector<std::byte>> {
              auto req_res =
                  srpc::Unmarshal<SeatAvailabilityCallbackRequest>{}(req_data);
              if (!req_res.second.has_value()) {
                std::cerr
                    << "Error: Could not unmarshal seat availability callback"
                    << std::endl;
                return {};
              }

              static std::unordered_map<
                  srpc::i64, std::pair<SeatAvailabilityCallbackRequest,
                                       SeatAvailabilityCallbackResponse>>
                  history;

              auto req = *req_res.second;
              std::cout << "Received seat availability callback: " << req
                        << std::endl;

              if (semantic == InvocationSemantic::kAtMostOnce &&
                  history.contains(req.id)) {
                std::clog << "Info: " << req.id << " is a duplicate request."
                          << std::endl;
                auto res = history[req.id].second;
                std::clog << "Info: Returning saved response " << res
                          << std::endl;
                return srpc::Marshal<SeatAvailabilityCallbackResponse>{}(res);
              }

              SeatAvailabilityCallbackResponse res{
                  .id = req.id,
                  .status_code = 0,
              };
              if (semantic == InvocationSemantic::kAtMostOnce) {
                history[req.id] = {req, res};
              }
              return srpc::Marshal<SeatAvailabilityCallbackResponse>{}(res);
            });
          },
          std::move(server)}
          .detach();
      std::this_thread::sleep_until(
          std::chrono::system_clock::from_time_t(res->monitor_end));
      continue;
    }
    std::cerr << "Please enter a valid selection." << std::endl;
  }
}
