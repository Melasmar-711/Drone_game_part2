cmake_minimum_required(VERSION 3.10)

# Project name and version
project(DroneSimulation VERSION 1.0)



include_directories(${CMAKE_CURRENT_SOURCE_DIR}/shared)

# Add the common library
add_library(shared common_files/shared.c)
add_library(signal_handlers common_files/sig_handle.c)
add_library(logger common_files/logger.c)


find_package(cJSON REQUIRED)


# Include directories for the shared library
target_include_directories(shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/common_files)
target_include_directories(signal_handlers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/common_files)
target_include_directories(logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/common_files)



target_link_libraries(shared  cjson)

# Add subdirectories for each module
add_subdirectory(KeyboardManager)
add_subdirectory(Targets_Generator)
add_subdirectory(Obstacle_Generator)
add_subdirectory(GameWindow)
add_subdirectory(DroneDynamicsManager)
add_subdirectory(BlackBoardServer)
add_subdirectory(WatchDog)
add_subdirectory(Targets_publisher_subscriber)
add_subdirectory(Obstacles_publisher_subscriber)
