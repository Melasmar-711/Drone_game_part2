

# Add ncurses library
find_package(Curses REQUIRED)

# Add executable
add_executable(KeyboardManager 
        KeyboardManager.c
        KeyBoard.c
        )


target_include_directories(KeyboardManager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link ncurses
target_link_libraries(KeyboardManager ${CURSES_LIBRARIES} shared signal_handlers logger)



