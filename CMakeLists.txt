cmake_minimum_required(VERSION 3.10)
project(minieditor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

# Uncomment to add SDL2:
# find_package(SDL2 REQUIRED)
# target_include_directories(minieditor PRIVATE ${SDL2_INCLUDE_DIRS})
# target_link_libraries(minieditor PRIVATE ${SDL2_LIBRARIES})

add_compile_options(-Wall -Wextra -Wpedantic)

# Where to find headers
include_directories(${CMAKE_SOURCE_DIR}/include)

if(MINIEDITOR_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

if(MINIEDITOR_USE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
  if(CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
  endif()
endif()


file(GLOB_RECURSE ALL_SRC CONFIGURE_DEPENDS
  "${CMAKE_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_SOURCE_DIR}/src/*.c"
)

# exclude main.cpp from core library if present
set(MAIN_CPP "${CMAKE_SOURCE_DIR}/src/main.cpp")
list(FILTER ALL_SRC EXCLUDE REGEX ".*/main.cpp$")

# -----------------------
# Targets
# -----------------------
# core static library (editor logic)
add_library(core STATIC ${ALL_SRC})
target_include_directories(core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# main executable
add_executable(minieditor "${MAIN_CPP}")
target_link_libraries(minieditor PRIVATE core)

# -----------------------
# Tests (Catch2 via FetchContent)
# -----------------------
include(CTest)
if(MINIEDITOR_BUILD_TESTS)
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
  endif()
endif()

target_link_libraries(tests PRIVATE core Catch2::Catch2WithMain)

add_custom_target(format
    COMMAND clang-format -i ${ALL_SRC}
    COMMENT "Running clang-format on all source files"
)


# -----------------------
# Helpful messages
# -----------------------
message(STATUS "Project: ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build tests: ${MINIEDITOR_BUILD_TESTS}")
message(STATUS "Sanitizers enabled: ${MINIEDITOR_ENABLE_SANITIZERS}")
message(STATUS "Clang-tidy enabled: ${MINIEDITOR_USE_CLANG_TIDY}")
