cmake_minimum_required(VERSION 3.8)
project(ddg)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS
    "$ENV{CXXFLAGS} -rdynamic -O0 -fPIC -std=c++11 -Wall -Werror -Wno-unused-function -Wno-builtin-macro-redefined"
)
set(CMAKE_C_FLAGS
    "$ENV{CXXFLAGS} -rdynamic -O0 -fPIC -std=c++11 -Wall -Werror -Wno-unused-function -Wno-builtin-macro-redefined"
)

aux_source_directory(ddg LIB_SRC)

add_library(ddg SHARED ${LIB_SRC})

target_link_libraries(ddg yaml-cpp)

include_directories(./)

add_executable(test tests/test.cc)
add_dependencies(test ddg)
target_link_libraries(test ddg)

add_executable(test_config tests/test_config.cc)
add_dependencies(test_config ddg)
target_link_libraries(test_config ddg)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
