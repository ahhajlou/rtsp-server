if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "Fuzzing requires Clang (libFuzzer is a Clang feature)")
endif()

function(add_fuzz_target name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE rtsp_server_lib)
    target_compile_options(${name} PRIVATE -g -O1 -fsanitize=fuzzer,address,undefined)
    target_link_options(${name} PRIVATE -fsanitize=fuzzer,address,undefined)
endfunction()

add_fuzz_target(fuzz_rtsp_frame_parser fuzz_rtsp_frame_parser.cpp)
