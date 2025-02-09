
add_executable(Obstacle_Generator
    Obstacle_Generator.c
    Generator_functions.c
)


target_include_directories(Obstacle_Generator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link libraries if necessary
target_link_libraries(Obstacle_Generator m shared signal_handlers logger)

