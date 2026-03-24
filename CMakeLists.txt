cmake_minimum_required(VERSION 3.20)
project(shiritori_cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(/EHsc)

add_definitions(-DPROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}")
add_definitions(-DBOOST_DATE_TIME_NO_LIB)
add_definitions(-DBOOST_REGEX_NO_LIB)
add_definitions(-D_WIN32_WINNT=0x0601)

find_package(Boost CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

add_executable(server main.cpp)
add_subdirectory(network)
add_subdirectory(service)
add_subdirectory(repo)
add_subdirectory(controller)
add_subdirectory(tests)

target_link_libraries(server PRIVATE
    ${Boost_LIBRARIES}
    ws2_32
    network
    controller
    service
    repo
)
