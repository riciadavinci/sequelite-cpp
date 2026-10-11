cmake_minimum_required(VERSION 3.20)

include(FetchContent)

set(FETCHCONTENT_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../deps" CACHE STRING "FetchContent Root" FORCE)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2 # Highly stable modern release tag
)

# Prevent GoogleTest from overriding your project-wide compiler variable maps
# For MSVC runtime library alignment on Windows
# Suppress installation of GTest targets when installing this project
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(sequelite_test_runner
    test_types.cpp
    test_database.cpp
    test_statement.cpp
)

target_link_libraries(sequelite_test_runner PRIVATE 
    sequelite::sequelite
    GTest::gtest_main
)

target_compile_features(sequelite_test_runner PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(sequelite_test_runner)


option(SEQUELITE_USE_ASAN "Enable Address and Undefined Behaviour Sanitizers" OFF)
option(SEQUELITE_USE_TSAN "Enable Thread Sanitizer" OFF)
option(SEQUELITE_ENABLE_CLANG_TIDY "Enable Clang-Tidy stataic analysis" OFF)

if (SEQUELITE_USE_ASAN AND SEQUELITE_USE_TSAN)
    message(FATAL_ERROR "SEQUELITE_USE_ASAN and SEQUELITE_USE_TSAN cannot be enabled at the same time!")
endif()

if(SEQUELITE_USE_ASAN)
    if(MSVC)
        target_compile_options(sequelite_test_runner PRIVATE /fsanitize=address)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|CLANG")
        target_compile_options(sequelite_test_runner PRIVATE 
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
            -g
        )
        target_link_options(sequelite_test_runner PRIVATE -fsanitize=address,undefined)
    endif()
endif()

if(SEQUELITE_USE_TSAN)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|CLANG")
        target_compile_options(sequelite_test_runner PRIVATE 
            -fsanitize=thread
            -fno-omit-frame-pointer
            -g
        )
        target_link_options(sequelite_test_runner PRIVATE -fsanitize=thread)
    else()
        message(WARNING "ThreadSanitizer (TSan) is only supported on GCC and Clang.")
    endif()
endif()

if(SEQUELITE_ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if(CLANG_TIDY_EXE)
        set_target_properties(sequelite_test_runner PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-warnings-as-errors=*"
        )
        message(STATUS "Clang-Tidy enabled for sequelite_test_runner")
    else()
        message(WARNING "SEQUELITE_ENABLE_CLANG_TIDY was requested, but clang-tidy executable was not found.")
    endif()
endif()
