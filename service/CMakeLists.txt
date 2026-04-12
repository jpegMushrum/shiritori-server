add_library(service STATIC
    game_session.cpp
    game_fabric.cpp
    info_service.cpp
    mapper.cpp
    game_context.cpp
    player_score.cpp
    
    dto/user_info.cpp
    dto/game_info.cpp
    dto/word_info.cpp

    interfaces/iinfo_service.cpp
    interfaces/igame_session.cpp
    interfaces/igame_fabric.cpp
)

target_include_directories(service PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(service PUBLIC 
    repo 
    ${Boost_LIBRARIES}    
)