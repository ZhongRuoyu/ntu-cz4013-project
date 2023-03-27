cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(dfis)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

function(add_srpc)
  # Vendor srpc, and statically link to it.
  set(BUILD_SHARED_LIBS OFF)
  set(BUILD_TESTING OFF)
  add_subdirectory(srpc EXCLUDE_FROM_ALL)
endfunction()
add_srpc()

set(DFIS_SRCS
  src/messages/flight.cc
  src/messages/flight_info.cc
  src/messages/flight_search.cc
  src/messages/seat_availability.cc
  src/messages/seat_reservation.cc
  src/utils/time.cc
  src/main.cc
)
add_executable(dfis)
target_include_directories(dfis PRIVATE src)
target_sources(dfis PRIVATE ${DFIS_SRCS})
target_link_libraries(dfis PRIVATE srpc)

find_package(Threads REQUIRED)
target_link_libraries(dfis PUBLIC Threads::Threads)

include(GNUInstallDirs)
install(TARGETS dfis DESTINATION ${CMAKE_INSTALL_BINDIR})

option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()
