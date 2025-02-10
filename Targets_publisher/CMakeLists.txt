
cmake_minimum_required(VERSION 3.20)

project("generated_code")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find requirements
find_package(fastcdr REQUIRED)
find_package(fastdds 3 REQUIRED)


message(STATUS "Configuring Targets...")
add_library(Targets_lib TargetsTypeObjectSupport.cxx TargetsPubSubTypes.cxx)
target_link_libraries(Targets_lib fastcdr fastdds)



add_executable(Targets TargetsApplication.cxx TargetsPublisherApp.cxx TargetsSubscriberApp.cxx Targetsmain.cxx)
target_link_libraries(Targets fastcdr fastdds
        Targets_lib shared
        )
        
target_include_directories(Targets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})



