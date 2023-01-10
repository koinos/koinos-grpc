file(GLOB HEADERS "include/koinos/services/*.hpp")
add_library(koinos_services_lib
            services.cpp
            ${HEADERS})
target_link_libraries(koinos_services_lib Koinos::exception Koinos::log Koinos::util gRPC::grpc)
target_include_directories(koinos_services_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(Koinos::services ALIAS koinos_services_lib)
