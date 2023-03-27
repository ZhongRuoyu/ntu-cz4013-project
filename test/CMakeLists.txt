cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

find_package(GTest REQUIRED)
add_executable(dfis_tests)
target_sources(dfis_tests PRIVATE
  messages/flight.cc
  messages/flight_info.cc
  messages/flight_search.cc
  messages/seat_reservation.cc
)
target_include_directories(dfis_tests PRIVATE ../src)
target_link_libraries(dfis_tests PRIVATE
  srpc
  GTest::gtest_main
)

list(TRANSFORM DFIS_SRCS PREPEND ../)
list(REMOVE_ITEM DFIS_SRCS ../src/main.cc)
target_sources(dfis_tests PRIVATE ${DFIS_SRCS})

include(GoogleTest)
gtest_discover_tests(dfis_tests)
