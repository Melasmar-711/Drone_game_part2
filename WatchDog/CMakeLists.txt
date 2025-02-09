
add_executable(WatchDog
    WatchDog.c
)


target_include_directories(WatchDog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link libraries if necessary
target_link_libraries(WatchDog m shared signal_handlers logger)

