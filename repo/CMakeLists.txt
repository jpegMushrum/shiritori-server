add_library(repo STATIC
    users_repo.cpp
    games_repo.cpp
    dicts/jisho.cpp
    models/user.cpp
    models/game.cpp
    models/word.cpp
)

target_include_directories(repo PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CURL_INCLUDE_DIRS}
)

target_link_libraries(repo PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(repo PRIVATE ${CURL_LIBRARIES})
