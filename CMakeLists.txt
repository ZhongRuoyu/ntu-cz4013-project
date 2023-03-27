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

set(DFIS_CORE_SRCS
  src/messages/flight.cc
  src/messages/flight_info.cc
  src/messages/flight_search.cc
  src/messages/seat_availability.cc
  src/messages/seat_reservation.cc
  src/utils/time.cc
)
add_library(dfis_core OBJECT ${DFIS_CORE_SRCS})
target_include_directories(dfis_core PUBLIC src)
target_link_libraries(dfis_core PUBLIC srpc)

set(DFIS_SERVER_SRCS
  src/server/main.cc
)
add_executable(dfis_server ${DFIS_SERVER_SRCS})
target_link_libraries(dfis_server PRIVATE dfis_core)

set(DFIS_CLIENT_SRCS
  src/client/main.cc
)
add_executable(dfis_client ${DFIS_CLIENT_SRCS})
target_link_libraries(dfis_client PRIVATE dfis_core)

include(GNUInstallDirs)
install(TARGETS dfis_server dfis_client DESTINATION ${CMAKE_INSTALL_BINDIR})

option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()
