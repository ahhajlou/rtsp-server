include(FetchContent)

# GTest
# Fetch Google Test (before setting warning flags)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.18.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Create test executable
add_executable(rtsp_server_tests
    rtsp_frame_parser_test.cpp
)

# Include the src directory so tests can find headers
target_include_directories(rtsp_server_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Link against gtest, gtest_main, and your library
# Note: We apply project_warnings to tests but NOT to gtest itself
target_link_libraries(rtsp_server_tests
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    rtsp_server_lib
    project_warnings  # Apply warnings to test code only
)

# Add tests to CTest
include(GoogleTest)
gtest_discover_tests(rtsp_server_tests)
