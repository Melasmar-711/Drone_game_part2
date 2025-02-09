

    # Find the cJSON library
find_package(cJSON REQUIRED)


add_executable(BlackBoardServer
    BlackBoardServer.c
    server_functions.c
)


target_include_directories(BlackBoardServer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


# Link libraries if necessary
target_link_libraries(BlackBoardServer m shared signal_handlers logger cjson)

