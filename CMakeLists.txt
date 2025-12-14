cmake_minimum_required(VERSION 3.10)
project(minieditor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -----------------------
# Options
# -----------------------
option(MINIEDITOR_BUILD_TESTS "Build unit tests" OFF)
option(MINIEDITOR_ENABLE_SANITIZERS "Enable Address/Undefined sanitizers (only in Debug)" OFF)
option(MINIEDITOR_USE_CLANG_TIDY "Run clang-tidy during builds if available" OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------
# Global warnings and includes (prefer per-target but leave defaults)
# -----------------------
add_compile_options(-Wall -Wextra -Wpedantic)

# Where to find headers (kept for compatibility; targets also set includes)
include_directories(${CMAKE_SOURCE_DIR}/include)

# clang-tidy
if(MINIEDITOR_USE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
  if(CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
  else()
    message(WARNING "clang-tidy requested but not found.")
  endif()
endif()

# -----------------------
# Sources
# -----------------------
file(GLOB_RECURSE ALL_SRC CONFIGURE_DEPENDS
  "${CMAKE_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_SOURCE_DIR}/src/*.c"
  "${CMAKE_SOURCE_DIR}/include/*.h"
  "${CMAKE_SOURCE_DIR}/include/*.hpp"
)

# exclude main.cpp from core library if present
set(MAIN_CPP "${CMAKE_SOURCE_DIR}/src/main.cpp")
list(FILTER ALL_SRC EXCLUDE REGEX ".*/main.cpp$")

# -----------------------
# Core library
# -----------------------
add_library(core STATIC ${ALL_SRC})
target_include_directories(core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Compiler options specific to core
target_compile_features(core PUBLIC cxx_std_20)

if(MINIEDITOR_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(core PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options(core PRIVATE -fsanitize=address,undefined)
  message(STATUS "Sanitizers added to 'core' (Debug build).")
endif()

# -----------------------
# Executable
# -----------------------
add_executable(minieditor "${MAIN_CPP}")
target_link_libraries(minieditor PRIVATE core)

if(MINIEDITOR_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  # also apply sanitizer flags to the exe to ensure linking works
  target_compile_options(minieditor PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options(minieditor PRIVATE -fsanitize=address,undefined)
endif()

# -----------------------
# Tests
# -----------------------
include(CTest)
if(MINIEDITOR_BUILD_TESTS AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
  include(FetchContent)
  FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
  )
  FetchContent_MakeAvailable(catch2)

  file(GLOB_RECURSE TEST_SRCS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tests/*.cpp")
  if(TEST_SRCS)
    add_executable(tests ${TEST_SRCS})
    target_link_libraries(tests PRIVATE core Catch2::Catch2WithMain)
    add_test(NAME unit_tests COMMAND tests)
    set_tests_properties(unit_tests PROPERTIES LABELS "unit")
  else()
    message(WARNING "MINIEDITOR_BUILD_TESTS=ON but no tests found in tests/")
  endif()
endif()

# -----------------------
# Format target
# -----------------------
find_program(CLANG_FORMAT_EXE NAMES clang-format clang-format-14)
if(CLANG_FORMAT_EXE)
  # prefer to format only source/header files (expand glob at configure time)
  file(GLOB_RECURSE FORMAT_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.c" "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/include/*.h" "${CMAKE_SOURCE_DIR}/include/*.hpp"
  )
  add_custom_target(format
      COMMAND ${CLANG_FORMAT_EXE} -i ${FORMAT_FILES}
      COMMENT "Running clang-format on sources"
  )
else()
  add_custom_target(format
      COMMENT "clang-format not found; format target is a no-op"
  )
endif()

# -----------------------
# Helpful messages
# -----------------------
message(STATUS "Project: ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build tests: ${MINIEDITOR_BUILD_TESTS}")
message(STATUS "Sanitizers enabled: ${MINIEDITOR_ENABLE_SANITIZERS}")
message(STATUS "Clang-tidy enabled: ${MINIEDITOR_USE_CLANG_TIDY}")

