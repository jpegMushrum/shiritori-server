add_library(network STATIC
    server.cpp
    session.cpp
    router.cpp
)

target_include_directories(network PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(network PUBLIC
    controller
    service
    ${Boost_LIBRARIES}
    ws2_32
)

target_link_libraries(network PRIVATE 
    nlohmann_json::nlohmann_json
)
