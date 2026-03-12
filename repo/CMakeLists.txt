add_library(repo STATIC
    users_repo.cpp
    games_repo.cpp
    
    dicts/jisho.cpp
    
    models/user.cpp
    models/game.cpp
    models/word.cpp

    interfaces/idictionary.cpp
    interfaces/igames_repo.cpp
    interfaces/iusers_repo.cpp
)

target_include_directories(repo PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)

target_link_libraries(repo PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(repo PRIVATE CURL::libcurl)
target_link_libraries(repo PRIVATE unofficial::sqlite3::sqlite3)

