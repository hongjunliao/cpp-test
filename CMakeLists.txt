# author hongjun.liao <docici@126.com>
# date 2021/6/12

cmake_minimum_required(VERSION 2.8)

# cpp-test 
project(cpp-test)
add_subdirectory(deps/libhp) 
add_executable(cpp-test src/main.c
	src/algs/9787302356288/9787302356288.c
	src/algs/algorithms4th/1.5.c
	src/algs/algorithms4th/sort.c
	src/hp_poll.c
	src/algs/intro_to_algs3th/16.c
	src/boost/test_boost_asio.cc
	src/dbus/test_dbus.c
	src/boost/test_boost_asio.cc
)
target_compile_options(cpp-test PUBLIC
    -DHAVE_CONFIG_H -D_GNU_SOURCE -DCMAKE_EXPORT_COMPILE_COMMANDS
)

#boost
if(NOT CPP_TEST_NO_BOOST)
target_link_libraries(cpp-test )
endif()

# dbus
if(WITH_DBUS)
find_package(PkgConfig REQUIRED)
pkg_check_modules(dbus REQUIRED dbus-1)
target_link_libraries(cpp-test ${dbus_LIBRARIES})
target_include_directories(cpp-test PUBLIC ${dbus_INCLUDE_DIRS})
target_compile_options(cpp-test PUBLIC ${dbus_CFLAGS_OTHER})
endif()

target_include_directories(cpp-test PRIVATE . src/ deps/ deps/libhp/include/)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	target_compile_options(cpp-test PUBLIC  -O0 -g3  -Wall)
else()
	target_compile_options(cpp-test PUBLIC -D_CRT_SECURE_NO_WARNINGS  /MTd)
endif()
target_link_libraries(cpp-test libhp pthread)
