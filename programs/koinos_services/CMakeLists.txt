file(GLOB HEADERS "include/*.hpp")

add_executable(koinos_services
               main.cpp
               ${HEADERS})
target_link_libraries(koinos_services PUBLIC Koinos::services Koinos::crypto Koinos::exception Koinos::proto Koinos::log Koinos::mq Koinos::git Boost::program_options yaml-cpp)
target_include_directories(koinos_services PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
install(TARGETS
   koinos_services
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
