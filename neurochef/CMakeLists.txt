cmake_minimum_required(VERSION 3.5)
project(NeuroChef)

# Set C standard and compiler flags
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

# Add the executable
add_executable(neurochef main.c)

# Add custom target for running tests
add_custom_target(test
    COMMAND python -m pytest
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running tests with pytest"
)

# Add custom target for running the chatbot
add_custom_target(run
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/neurochef
    DEPENDS neurochef
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running NeuroChef chatbot"
)
