cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Piper VERSION 0.0.1 DESCRIPTION "Support class to simplify using NamedPipes" LANGUAGES CXX)

# Use 'Release' by default for build
set(DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Define GNU standard installation directories
include(GNUInstallDirs)
include_directories(include)

# Define source files
set(SRC_FILES
        src/piper.cpp
        src/imagepiper.cpp
        src/image2string.cpp
)

# Define server source files
set(SERVER_SRC_FILES
    ${SRC_FILES}
    src/server.cpp
)

# Define server source files
set(CLIENT_SRC_FILES
    ${SRC_FILES}
    src/client.cpp
)

# Define OpenCV library
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# add the executable
add_executable(${PROJECT_NAME}Server ${SERVER_SRC_FILES})
target_link_libraries(${PROJECT_NAME}Server ${OpenCV_LIBS})

# add the executable
add_executable(${PROJECT_NAME}Client ${CLIENT_SRC_FILES})
target_link_libraries(${PROJECT_NAME}Client ${OpenCV_LIBS})

if(CMAKE_BUILD_TYPE MATCHES Release)
# Define OpenCV dlls (will be copied to install folder)
set(OpenCV_DLLS
        ${OpenCV_DIR}/x64/vc15/bin/opencv_world440.dll
        ${OpenCV_DIR}/x64/vc15/bin/opencv_videoio_ffmpeg440_64.dll
)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
# Define OpenCV dlls (will be copied to install folder)
set(OpenCV_DLLS
        ${OpenCV_DIR}/x64/vc15/bin/opencv_world440d.dll
        ${OpenCV_DIR}/x64/vc15/bin/opencv_videoio_ffmpeg440_64.dll
)
endif()

add_custom_command(
        TARGET ${PROJECT_NAME}Server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/data/data.txt
                $<TARGET_FILE_DIR:${PROJECT_NAME}Server>)

# copy dlls to build folder
foreach( dll ${OpenCV_DLLS})
        add_custom_command(
                TARGET ${PROJECT_NAME}Server POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${dll}
                $<TARGET_FILE_DIR:${PROJECT_NAME}Server>)
endforeach( dll )

# Install app in install directory
install(TARGETS ${PROJECT_NAME}Server DESTINATION .)
# Install app in install directory
install(TARGETS ${PROJECT_NAME}Client DESTINATION .)
# Install OpenCV dlls in install directory
install(FILES ${OpenCV_DLLS} DESTINATION .)
# Install test data in install directory
install(FILES ${CMAKE_SOURCE_DIR}/data/data.txt DESTINATION .)