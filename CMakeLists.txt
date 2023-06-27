# author hongjun.liao <docici@126.com>
# date 2021/6/12
cmake_minimum_required(VERSION 3.0)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/config.h"
)
# cpp-test 
project(cpp-test)
add_subdirectory(deps/libhp) 
find_package(PkgConfig REQUIRED)

#WITH_LIBUV
if(WITH_LIBUV)
pkg_check_modules(libuv REQUIRED libuv)
file(GLOB SRCS ${SRCS} src/uv/uv.c )
endif()

#WITH_LIBWX
if(WITH_LIBWX)
find_package(wxWidgets REQUIRED COMPONENTS core base)
file(GLOB SRCS ${SRCS} src/wx/test_libwx.cpp )
include(${wxWidgets_USE_FILE})
endif()

#WITH_openssl
if(WITH_OPENSSL)
find_package(OpenSSL REQUIRED)
file(GLOB SRCS ${SRCS} src/openssl/bignum.cpp )
include_directories(${OPENSSL_INCLUDE_DIR})
endif()

add_executable(cpp-test ${SRCS} src/main.c
	src/algs/9787302356288/9787302356288.c
	src/algs/algorithms4th/1.5.c
	src/algs/algorithms4th/sort.c
	src/hp_poll.c
	src/algs/intro_to_algs3th/16.c
	src/boost/test_boost_asio.cc
	src/dbus/test_dbus.c
	src/boost/test_boost_asio.cc
)

if(WITH_LIBWX)
target_link_libraries(cpp-test ${wxWidgets_LIBRARIES})
endif()

if(WITH_LIBUV)
target_include_directories(cpp-test PUBLIC ${libuv_INCLUDE_DIRS})
target_link_libraries(cpp-test ${libuv_LIBRARIES})
endif()

if(WITH_OPENSSL)
target_include_directories(cpp-test PUBLIC ${openssl_INCLUDE_DIRS})
target_link_libraries(cpp-test ${openssl_LIBRARIES})
endif()

target_compile_options(cpp-test PUBLIC
    -DHAVE_CONFIG_H -D_GNU_SOURCE -DCMAKE_EXPORT_COMPILE_COMMANDS
)

#boost
if(NOT CPP_TEST_NO_BOOST)
endif()

# dbus
if(WITH_DBUS)
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
target_link_libraries(cpp-test libhp pthread crypto)
