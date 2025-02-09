# Add the executable for drone_dynamics
add_executable(DroneDynamicsManager 
	DroneDynamicsManager.c
	Dynamics_functions.c
)

target_include_directories(DroneDynamicsManager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(DroneDynamicsManager m shared signal_handlers logger)
