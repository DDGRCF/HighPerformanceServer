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

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
  include_directories(${OPENSSL_INCLUDE_DIR})
endif()

aux_source_directory(${CMAKE_PROJECT_NAME} LIB_SRC)
aux_source_directory(${CMAKE_PROJECT_NAME}/utils LIB_SRC)
aux_source_directory(${CMAKE_PROJECT_NAME}/stream LIB_SRC)
aux_source_directory(${CMAKE_PROJECT_NAME}/http LIB_SRC)
aux_source_directory(${CMAKE_PROJECT_NAME}/http/servlets LIB_SRC)

add_library(${CMAKE_PROJECT_NAME} SHARED ${LIB_SRC})

target_link_libraries(${CMAKE_PROJECT_NAME} yaml-cpp dl ${OPENSSL_LIBRARIES}
                      jsoncpp)

include_directories(./)

aux_source_directory(tests TEST_SRC)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

foreach(filepath ${TEST_SRC})
  get_filename_component(filename ${filepath} NAME_WLE)
  add_executable(${filename} ${filepath})
  add_dependencies(${filename} ${CMAKE_PROJECT_NAME})
  target_link_libraries(${filename} ${CMAKE_PROJECT_NAME} dl)
endforeach()

aux_source_directory(examples EXAMPLE_SRC)

foreach(filepath ${EXAMPLE_SRC})
  get_filename_component(filename ${filepath} NAME_WLE)
  add_executable(${filename} ${filepath})
  add_dependencies(${filename} ${CMAKE_PROJECT_NAME})
  target_link_libraries(${filename} ${CMAKE_PROJECT_NAME} dl)
endforeach()
