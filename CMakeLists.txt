cmake_minimum_required(VERSION 3.10)
project(PubSubCPP)

set(CMAKE_CXX_STANDARD 17)


find_library(LIBRT rt)
find_library(LIBPTHREAD pthread)
add_executable(pubsub pubsub.cpp topic.hpp)
add_executable(rmtopic rmtopic.cpp topic.hpp)
target_link_libraries(pubsub ${LIBRT} ${LIBPTHREAD})
target_link_libraries(rmtopic ${LIBRT} ${LIBPTHREAD})