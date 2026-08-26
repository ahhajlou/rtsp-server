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
    gtest
    gtest_main
    rtsp_server_lib
    project_warnings  # Apply warnings to test code only
)

# Add tests to CTest
include(GoogleTest)
gtest_discover_tests(rtsp_server_tests)
