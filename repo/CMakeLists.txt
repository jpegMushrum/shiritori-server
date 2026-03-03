add_library(repo STATIC
    players_repo.cpp
    dictionary.cpp
)

target_include_directories(repo PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)