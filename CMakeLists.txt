cmake_minimum_required(VERSION 3.10)
project(of_controller)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPCAP REQUIRED libpcap)

# Include directories
include_directories(${LIBPCAP_INCLUDE_DIRS})
include_directories(include)

# Define the executable
add_executable(of_controller 
    src/main.cpp
    src/of_controller.cpp
    src/of_switch.cpp
    src/of_message.cpp
)

# Link libraries
target_link_libraries(of_controller ${LIBPCAP_LIBRARIES})
target_link_libraries(of_controller pthread)
