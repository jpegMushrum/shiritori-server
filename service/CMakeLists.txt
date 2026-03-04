add_library(service STATIC
    game_session.cpp
    info_service.cpp
    dto/user_info.cpp
)

target_include_directories(service PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(service PUBLIC repo)