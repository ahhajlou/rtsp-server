cmake_minimum_required(VERSION 3.20)

project(rtsp-server VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "14.0")
        message(FATAL_ERROR
            "GCC ${CMAKE_CXX_COMPILER_VERSION} is too old — this project needs "
            "GCC >= 14 for <generator> (std::generator, C++23). "
            "Install g++-14 or newer and pass -DCMAKE_CXX_COMPILER=g++-14.")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "17.0")
        message(FATAL_ERROR
            "Clang ${CMAKE_CXX_COMPILER_VERSION} is too old — this project needs "
            "Clang >= 17 with libc++ for <generator> support.")
    endif()
else()
    message(WARNING
        "Unrecognized compiler ${CMAKE_CXX_COMPILER_ID} — std::generator support unverified.")
endif()

include(FetchContent)

# ASIO

FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-38-2
)

FetchContent_MakeAvailable(asio)

add_library(asio INTERFACE)

target_include_directories(asio
  INTERFACE ${asio_SOURCE_DIR}/asio/include
)

target_compile_definitions(asio
  INTERFACE
    ASIO_STANDALONE
    ASIO_NO_DEPRECATED
)

target_link_libraries(asio
INTERFACE pthread
)

# Now set compiler warnings for OUR code only
# Create an interface library to hold our warning flags
add_library(project_warnings INTERFACE)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)

option(ENABLE_FUZZING "Build fuzz targets (requires Clang)" OFF)
if(ENABLE_FUZZING)
    add_subdirectory(fuzz)
endif()
