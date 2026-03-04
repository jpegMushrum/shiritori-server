add_library(repo STATIC
    users_repo.cpp
    dictionary.cpp
    models/user.cpp
    models/game.cpp
    models/word.cpp
)

target_include_directories(repo PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)