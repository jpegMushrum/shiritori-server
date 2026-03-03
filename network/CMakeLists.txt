add_library(network STATIC
    server.cpp
    session.cpp
)

target_include_directories(network PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(network PUBLIC
    game
    ${Boost_LIBRARIES}
    ws2_32
)