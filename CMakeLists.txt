cmake_minimum_required(VERSION 3.10)
project(of_controller)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    # for my macbook, find libpcap by directly setting paths to the Homebrew installation
    # for some reason find_package doesn't work on my macbook
    set(LIBPCAP_ROOT "/opt/homebrew/Cellar/libpcap/1.10.5")
    set(LIBPCAP_LIBRARIES "${LIBPCAP_ROOT}/lib/libpcap.dylib")
    set(LIBPCAP_INCLUDE_DIRS "${LIBPCAP_ROOT}/include")

    # Confirm libpcap is found
    if(NOT EXISTS ${LIBPCAP_LIBRARIES})
        message(FATAL_ERROR "Cannot find libpcap library at ${LIBPCAP_LIBRARIES}")
    endif()

    if(NOT EXISTS ${LIBPCAP_INCLUDE_DIRS}/pcap.h)
        message(FATAL_ERROR "Cannot find pcap.h at ${LIBPCAP_INCLUDE_DIRS}")
    endif()

    message(STATUS "Using libpcap from Homebrew installation")
    set(FOUND_LIBPCAP TRUE)
else()
# for Linux and Windows, use find_package to find libpcap
    find_package(PCAP REQUIRED)
    set(FOUND_LIBPCAP ${PCAP_FOUND})
endif()

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

# Print status message to help with debugging
message(STATUS "LIBPCAP_LIBRARIES: ${LIBPCAP_LIBRARIES}")
message(STATUS "LIBPCAP_INCLUDE_DIRS: ${LIBPCAP_INCLUDE_DIRS}")
