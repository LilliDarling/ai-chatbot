cmake_minimum_required(VERSION 3.10)
project(NeuroChef)

# Set C standard and compiler flags
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

# Source files
set(SOURCES
    main.c
    recipe_utils.c
)

# Add the executable
add_executable(neurochef ${SOURCES})

# Copy meal_data.json to build directory
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/meal_data.json
               ${CMAKE_CURRENT_BINARY_DIR}/meal_data.json COPYONLY)

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
