cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

find_package(GTest REQUIRED)
add_executable(dfis_tests)
target_sources(dfis_tests PRIVATE
  messages/flight.cc
  messages/flight_info.cc
  messages/flight_search.cc
  messages/seat_availability.cc
  messages/seat_reservation.cc
  utils/rand.cc
  utils/time.cc
)
target_link_libraries(dfis_tests PRIVATE
  dfis_core
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(dfis_tests)
