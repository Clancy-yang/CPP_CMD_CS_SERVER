# cmake_minimum_required(VERSION <specify CMake version here>)
project(CPP_CMD_CS_TEST_SERVER)

set(CMAKE_CXX_STANDARD 11)

add_executable(CPP_CMD_CS_TEST_SERVER
        main.cpp
        Management.h
        Management.cpp)
target_link_libraries(CPP_CMD_CS_TEST_SERVER
        pthread
)