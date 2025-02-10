
cmake_minimum_required(VERSION 3.20)

project("generated_code")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find requirements
find_package(fastcdr REQUIRED)
find_package(fastdds 3 REQUIRED)


message(STATUS "Configuring Obstacles...")
add_library(Obstacles_lib ObstaclesTypeObjectSupport.cxx ObstaclesPubSubTypes.cxx)
target_link_libraries(Obstacles_lib fastcdr fastdds)

add_executable(Obstacles ObstaclesApplication.cxx ObstaclesPublisherApp.cxx ObstaclesSubscriberApp.cxx Obstaclesmain.cxx)
target_link_libraries(Obstacles fastcdr fastdds
        Obstacles_lib 
        )



