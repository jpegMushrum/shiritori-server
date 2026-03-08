cmake_minimum_required(VERSION 3.20)
project(shiritori_cpp)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Boost_ROOT "D:/Boost")
set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)

find_package(Boost CONFIG REQUIRED COMPONENTS system)
find_package(CURL CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
    message(STATUS "Boost found: ${Boost_VERSION}")
else()
    message(FATAL_ERROR "Boost not found")
endif()

add_executable(server main.cpp)
add_subdirectory(network)
add_subdirectory(service)
add_subdirectory(repo)
add_subdirectory(controller)

target_include_directories(server PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(server PRIVATE ${Boost_LIBRARIES} ws2_32)
target_link_libraries(server PRIVATE controller)
target_link_libraries(server PRIVATE service)
target_link_libraries(server PRIVATE repo)

# CURL::libcurl
# nlohmann_josn::nlohmann_json
