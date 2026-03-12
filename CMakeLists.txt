cmake_minimum_required(VERSION 3.20)
project(shiritori_cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DPROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}")

find_package(Boost CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

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
add_subdirectory(tests)

target_link_libraries(server PRIVATE controller)
target_link_libraries(server PRIVATE service)
target_link_libraries(server PRIVATE repo)
