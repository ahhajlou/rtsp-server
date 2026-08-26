cmake_minimum_required(VERSION 3.20)

project(rtsp-server VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
