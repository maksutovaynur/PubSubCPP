cmake_minimum_required(VERSION 3.10)
project(PubSubCPP)

set(CMAKE_CXX_STANDARD 17)


find_library(LIBRT rt)
find_library(LIBPTHREAD pthread)
add_executable(pubsub pubsub.cpp topic.hpp debug.hpp)
add_executable(rmtopic rmtopic.cpp topic.hpp debug.hpp)
add_executable(measure measure.cpp topic.hpp debug.hpp)
add_executable(test test.cpp)
add_executable(service service.cpp topic.hpp debug.hpp)
add_executable(rmservice rmservice.cpp topic.hpp debug.hpp)
target_link_libraries(pubsub ${LIBRT} ${LIBPTHREAD})
target_link_libraries(rmtopic ${LIBRT} ${LIBPTHREAD})
target_link_libraries(measure ${LIBRT} ${LIBPTHREAD})
target_link_libraries(rmservice ${LIBRT} ${LIBPTHREAD})
target_link_libraries(service ${LIBRT} ${LIBPTHREAD})