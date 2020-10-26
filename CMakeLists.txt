cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(NamedPipes VERSION 0.1)

# Define GNU standard installation directories
include(GNUInstallDirs)


# add the executable
add_executable(NamedPipesServer src/server.cpp)

# add the executable
add_executable(NamedPipesClient src/client.cpp)

add_custom_command(
        TARGET NamedPipesServer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/data/data.txt
                $<TARGET_FILE_DIR:NamedPipesServer>)