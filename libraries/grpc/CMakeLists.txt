file(GLOB HEADERS "include/koinos/grpc/*.hpp")
add_library(koinos_grpc_lib OBJECT
            grpc.cpp
            ${HEADERS})
target_link_libraries(koinos_grpc_lib Koinos::exception Koinos::log Koinos::util gRPC::grpc)
target_include_directories(koinos_grpc_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(Koinos::grpc ALIAS koinos_grpc_lib)
