cmake_minimum_required(VERSION 3.10)
project(WebservTest)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

include(FetchContent)
FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

file(GLOB_RECURSE TEST_SOURCES "${CMAKE_SOURCE_DIR}/tests/*.cpp")

add_executable(webserv_test ${TEST_SOURCES})
target_include_directories(webserv_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(webserv_test gtest gtest_main)

gtest_discover_tests(webserv_test)
