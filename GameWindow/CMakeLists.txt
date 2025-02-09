cmake_minimum_required(VERSION 3.10)
project(GameWindow)


# Add ncurses library
find_package(Curses REQUIRED)

# Add executable
add_executable(GameWindow GameWindow.c
Window.c)

# Link ncurses
target_link_libraries(GameWindow ${CURSES_LIBRARIES} m shared signal_handlers logger)


