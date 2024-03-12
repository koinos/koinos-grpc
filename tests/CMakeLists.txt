if (NOT BUILD_TESTING)
  return()
endif()

include(CTest)

koinos_add_test(
  grpc_tests
    SOURCES
      main.cpp
      grpc_tests.cpp)

target_link_libraries(
  grpc_tests
    PRIVATE
      grpc
      Koinos::crypto
      Koinos::exception
      Koinos::log
      Koinos::proto
      Koinos::util
      gRPC::grpc)

target_include_directories(
  grpc_tests
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>)

koinos_add_format(TARGET grpc_tests)

koinos_coverage(
  EXECUTABLE
    grpc_tests
  EXCLUDE
    "tests/*")
