cmake_minimum_required(VERSION 3.8.2)
project(Grandma VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(httplib REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(tinyxml2 REQUIRED)

##
# Actual client library
##

file(GLOB omadm-client_SOURCES "client/src/*.cpp" "MO/src/*.cpp")
file(GLOB omadm-client_INTERFACE "interface/*.h")

add_library(omadm-client SHARED ${omadm-client_SOURCES})
set_target_properties(omadm-client PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(omadm-client PROPERTIES SOVERSION 0)

target_include_directories(omadm-client PRIVATE "client/include")
target_include_directories(omadm-client PRIVATE "MO/include")
target_include_directories(omadm-client PUBLIC "interface")

target_link_libraries(omadm-client PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(omadm-client PUBLIC OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(omadm-client PUBLIC tinyxml2)

target_compile_options(omadm-client PRIVATE -O2 -Werror -Wall -Wextra)

##
# Example application and example MO implementations
##

add_executable(dmclient "example/src/main.cpp")
target_include_directories(dmclient PRIVATE "client/include")
target_include_directories(dmclient PRIVATE "MO/include")
target_include_directories(dmclient PUBLIC "interface")
target_link_libraries(dmclient PRIVATE omadm-client)

##
# Test Armatures
##
add_executable(test_server "test/server.cpp")
target_link_libraries(test_server PRIVATE nlohmann_json::nlohmann_json pthread)
set_target_properties(test_server PROPERTIES CXX_STANDARD 14)
target_compile_options(test_server PRIVATE -Wall -Wextra)

