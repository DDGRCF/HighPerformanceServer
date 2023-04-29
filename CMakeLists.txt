cmake_minimum_required(VERSION 3.8)
project(mysylar)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS
    "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Werror -Wno-unused-function -Wno-builtin-macro-redefined"
)

set(CMAKE_C_FLAGS
    "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Werror -Wno-unused-function -Wno-builtin-macro-redefined"
)

set(LIB_SRC sylar/log.cc)

add_library(sylar SHARED ${LIB_SRC})

include_directories(./)
add_executable(test tests/test.cc)
add_dependencies(test sylar)
target_link_libraries(test sylar)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
