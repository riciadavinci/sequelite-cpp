cmake_minimum_required(VERSION 3.20)
project(sequelite-cpp LANGUAGES C CXX)

option(SEQUELITE_USE_SYSTEM_SQLITE "Use system SQLite instead of the bundled amalgamation" OFF)

if(SEQUELITE_USE_SYSTEM_SQLITE)
    find_package(SQLite3 REQUIRED)
    set(SEQUELITE_SQLITE_TARGET SQLite::SQLite3)
else()
    include(FetchContent)
    set(FETCHCONTENT_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps" CACHE STRING "FetchContent Root" FORCE)
    FetchContent_Declare(sqlite3_amalgamation
        URL      https://sqlite.org/2026/sqlite-amalgamation-3540000.zip
        URL_HASH SHA3_256=7b670a62fdfbd672b75fef004cb703c8a3e87d3a5cc7d675b4a08337004a2d93
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(sqlite3_amalgamation)

    add_library(sqlite3_bundled STATIC
        ${sqlite3_amalgamation_SOURCE_DIR}/sqlite3.c)
    target_include_directories(sqlite3_bundled SYSTEM PUBLIC
        ${sqlite3_amalgamation_SOURCE_DIR})
    target_compile_definitions(sqlite3_bundled PRIVATE
        SQLITE_THREADSAFE=1)             # choose deliberately, see below
    find_package(Threads REQUIRED)
    target_link_libraries(sqlite3_bundled PUBLIC
        Threads::Threads ${CMAKE_DL_LIBS})
    set(SEQUELITE_SQLITE_TARGET sqlite3_bundled)
endif()

# Fallback to Release build if no CMAKE_BUILD_TYPE is explicitly specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

add_library(sequelite-cpp STATIC 
    src/database.cpp 
    # src/statement.cpp
    src/impl.hpp
    include/sequelite/sequelite.hpp
    include/sequelite/types.hpp
)
add_library(sequelite::sequelite ALIAS sequelite-cpp)
target_include_directories(sequelite-cpp PUBLIC include)
target_compile_features(sequelite-cpp PUBLIC cxx_std_20)
target_link_libraries(sequelite-cpp PRIVATE ${SEQUELITE_SQLITE_TARGET})

option(SEQUELITE_BUILD_TESTS "Build unit tests for sequelite-cpp" ON)

if (SEQUELITE_BUILD_TESTS AND (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME))
    include(CTest)
    add_subdirectory(tests)
endif()