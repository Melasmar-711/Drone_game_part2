
add_executable(Targets_Generator
    Targets_Generator.c
    Generator_functions.c
)


target_include_directories(Targets_Generator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link libraries if necessary
target_link_libraries(Targets_Generator m shared signal_handlers logger)

