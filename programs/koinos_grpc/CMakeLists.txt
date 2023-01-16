file(GLOB HEADERS "include/*.hpp")

add_executable(koinos_grpc
               server.cpp
               ${HEADERS})
target_link_libraries(koinos_grpc PUBLIC Koinos::grpc Koinos::crypto Koinos::exception Koinos::proto Koinos::log Koinos::mq Koinos::git Boost::program_options yaml-cpp gRPC::grpc++ protobuf::libprotobuf)
target_include_directories(koinos_grpc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
install(TARGETS
   koinos_grpc
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)

add_executable(koinos_grpc_client
               client.cpp
               ${HEADERS})
target_link_libraries(koinos_grpc_client PUBLIC Koinos::grpc Koinos::crypto Koinos::exception Koinos::proto Koinos::log Koinos::mq Koinos::git Boost::program_options yaml-cpp gRPC::grpc++ protobuf::libprotobuf)
target_include_directories(koinos_grpc_client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
install(TARGETS
   koinos_grpc_client
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)

