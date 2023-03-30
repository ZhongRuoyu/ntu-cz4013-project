#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <optional>
#include <ostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>

#include <srpc/network/datagram_client.h>
#include <srpc/network/datagram_server.h>
#include <srpc/network/tcp_ip.h>
#include <srpc/types/floats.h>
#include <srpc/types/integers.h>
#include <srpc/types/serialization.h>
#include <srpc/utils/result.h>

#include "messages/flight_info.h"
#include "messages/flight_search.h"
#include "messages/invocation_semantic.h"
#include "messages/seat_availability.h"
#include "messages/seat_reservation.h"
#include "utils/rand.h"

using namespace dfis;

namespace {

[[noreturn]] void Bye() {
  std::cout << "Bye-bye!" << std::endl;
  // NOLINTNEXTLINE(concurrency-mt-unsafe)
  std::exit(EXIT_SUCCESS);
}

template <typename T>
T PromptForInput(const std::string &prompt, const std::string &retry_prompt) {
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

template <typename Req, typename Res>
std::optional<Res> SendAndReceive(std::unique_ptr<srpc::DatagramClient> &client,
                                  const std::string &server_addr,
                                  srpc::u16 server_port, Req req) {
  req.id = MakeMessageIdentifier();

  std::vector<std::byte> resp_bytes;
  constexpr int retry_times = 3;
  int attempt = 0;
  while (attempt <= retry_times) {
    auto resp_bytes_res = client->SendAndReceive(srpc::Marshal<Req>{}(req));
    if (!resp_bytes_res.OK()) {
      std::clog << "Info: Unable to receive response: "
                << resp_bytes_res.Error() << std::endl;
      if (++attempt <= retry_times) {
        std::clog << "Info: Retrying; attempt " << attempt << std::endl;
      }
      continue;
    }
    resp_bytes = std::move(resp_bytes_res.Value());
    break;
  }
  if (attempt > retry_times) {
    std::cerr << "Error: Unable to receive response after " << retry_times
              << " retries" << std::endl;
    return {};
  }

  auto resp_res = srpc::Unmarshal<Res>{}(resp_bytes);
  if (!resp_res.second.has_value()) {
    std::cerr << "Error: Failed to unmarshal response" << std::endl;
    return {};
  }

  auto resp = std::move(*resp_res.second);
  std::cout << "Received response:\n" << resp << std::endl;
  return resp;
}

std::optional<std::vector<std::byte>> ServeSeatAvailabilityCallbacks(
    InvocationSemantic semantic, const srpc::SocketAddress &from_addr,
    srpc::Result<std::vector<std::byte>> req_data_res) {
  if (!req_data_res.OK()) {
    std::cerr << "Error: Could not receive seat availability callback from "
              << from_addr << ": " << req_data_res.Error() << std::endl;
    return {};
  }

  auto req_data = std::move(req_data_res.Value());
  auto req_res = srpc::Unmarshal<SeatAvailabilityCallbackRequest>{}(req_data);
  if (!req_res.second.has_value()) {
    std::cerr << "Error: Could not unmarshal seat availability callback"
              << std::endl;
    return {};
  }

  static std::unordered_map<srpc::u64,
                            std::pair<SeatAvailabilityCallbackRequest,
                                      SeatAvailabilityCallbackResponse>>
      history;

  auto req = *req_res.second;
  std::cout << "Received seat availability callback: " << req << std::endl;

  auto req_lost = RandomLoss(0.1);
  auto res_lost = RandomLoss(0.2);
  if (req_lost) {
    std::clog << "Info: Callback request " << req.id
              << " is simulated to be lost" << std::endl;
    return {};
  }
  RandomDelay();

  if (semantic == InvocationSemantic::kAtMostOnce && history.contains(req.id)) {
    std::clog << "Info: " << req.id << " is a duplicate request" << std::endl;
    auto res = history[req.id].second;
    if (res_lost) {
      std::clog << "Info: Callback response " << res.id
                << " is simulated to be lost" << std::endl;
      return {};
    }
    RandomDelay();
    std::clog << "Info: Returning saved response " << res << std::endl;
    return srpc::Marshal<SeatAvailabilityCallbackResponse>{}(res);
  }

  SeatAvailabilityCallbackResponse res{
      .id = req.id,
      .status_code = 0,
  };
  if (semantic == InvocationSemantic::kAtMostOnce) {
    history[req.id] = {req, res};
  }

  if (res_lost) {
    std::clog << "Info: Callback response " << res.id
              << " is simulated to be lost" << std::endl;
    return {};
  }
  RandomDelay();

  std::clog << "Info: Sending callback response to " << from_addr << ": " << res
            << std::endl;
  return srpc::Marshal<SeatAvailabilityCallbackResponse>{}(res);
}

}  // namespace

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
    std::clog << "Info: At-least-once semantic is used" << std::endl;
  } else if (std::strcmp(argv[1], "at-most-once") == 0) {
    semantic = InvocationSemantic::kAtMostOnce;
    std::clog << "Info: At-most-once semantic is used" << std::endl;
  } else {
    std::cerr << "Error: Invalid invocation semantic: " << argv[1] << std::endl;
    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    std::exit(EXIT_FAILURE);
  }

  auto client_res = srpc::DatagramClient::New(server_addr, server_port);
  if (!client_res.OK()) {
    std::cerr << "Error: Unable to create client: " << client_res.Error()
              << std::endl;
    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    std::exit(EXIT_FAILURE);
  }
  auto client = std::move(client_res.Value());

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
5. Price range search
6. Seat reservation cancellation
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
          client, server_addr, server_port, req);
      continue;
    }
    if (line == "2") {
      FlightInfoRequest req;
      req.identifier = PromptForInput<srpc::i32>("Enter identifier: ",
                                                 "Please enter an integer: ");
      SendAndReceive<FlightInfoRequest, FlightInfoResponse>(client, server_addr,
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
          client, server_addr, server_port, req);
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
          client, server_addr, server_port, req);
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
      std::thread{[semantic](auto server) {
                    server->Listen(
                        [semantic](const auto &from_addr, auto req_data_res) {
                          return ServeSeatAvailabilityCallbacks(
                              semantic, from_addr, req_data_res);
                        });
                  },
                  std::move(server)}
          .detach();
      std::this_thread::sleep_until(
          std::chrono::system_clock::from_time_t(res->monitor_end));
      continue;
    }
    if (line == "5") {
      PriceRangeSearchRequest req;
      req.from = PromptForInput<srpc::f32>("Enter lower bound of price range: ",
                                           "Please enter a number: ");
      req.to = PromptForInput<srpc::f32>("Enter upper bound of price range: ",
                                         "Please enter a number: ");
      SendAndReceive<PriceRangeSearchRequest, PriceRangeSearchResponse>(
          client, server_addr, server_port, req);
      continue;
    }
    if (line == "6") {
      SeatReservationCancellationRequest req;
      req.reservation_req_id =
          PromptForInput<srpc::u64>("Enter reservation request identifier: ",
                                    "Please enter a non-negative integer: ");
      req.identifier = PromptForInput<srpc::i32>("Enter flight identifier: ",
                                                 "Please enter an integer: ");
      req.seats = PromptForInput<srpc::i32>("Enter number of seats to cancel: ",
                                            "Please enter an integer: ");
      SendAndReceive<SeatReservationCancellationRequest,
                     SeatReservationCancellationResponse>(client, server_addr,
                                                          server_port, req);
      continue;
    }

    std::cerr << "Please enter a valid selection." << std::endl;
  }
}
