add_library(grpc
  koinos/services/grpc.cpp

  koinos/services/grpc.hpp)

target_link_libraries(
  grpc
    PUBLIC
      Koinos::exception
      Koinos::log
      Koinos::mq
      Koinos::util
      gRPC::grpc)

target_include_directories(
  grpc
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

koinos_add_format(TARGET grpc)

#add_executable(koinos_grpc_client_demo koinos_grpc_client_demo.cpp)
#target_link_libraries(
#  koinos_grpc_client_demo
#    PRIVATE
#      grpc
#      Koinos::git
#      Koinos::proto
#      gRPC::grpc++
#      gRPC::grpc++_reflection
#      protobuf::libprotobuf)
#
#koinos_add_format(TARGET koinos_grpc_client_demo)
#
add_executable(koinos_grpc koinos_grpc.cpp)
target_link_libraries(
 koinos_grpc
   PRIVATE
     grpc
     Boost::program_options
     gRPC::grpc++
     Koinos::git
     protobuf::libprotobuf
     yaml-cpp)

target_include_directories(
  koinos_grpc
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

koinos_add_format(TARGET koinos_grpc)

koinos_install(TARGETS koinos_grpc)
