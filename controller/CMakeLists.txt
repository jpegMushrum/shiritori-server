add_library(controller STATIC
    info_controller.cpp
    task_queue.cpp
    games_controller.cpp

    interfaces/iinfo_controller.cpp
    interfaces/igames_controller.cpp
    interfaces/itask_queue.cpp
)

target_include_directories(controller PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)

target_link_libraries(controller PUBLIC service)