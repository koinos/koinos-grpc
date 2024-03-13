if (NOT BUILD_EXAMPLES)
  return()
endif()

add_executable(koinos_grpc_client_example koinos_grpc_client_example.cpp)
target_link_libraries(
  koinos_grpc_client_example
    PRIVATE
      grpc
      Koinos::proto
      gRPC::grpc++
      gRPC::grpc++_reflection
      protobuf::libprotobuf)

koinos_add_format(TARGET koinos_grpc_client_example)

koinos_install(TARGETS koinos_grpc_client_example)

