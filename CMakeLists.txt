###########################################################################################
#
# This file is PART of cpp-test project
#
# author hongjun.liao <docici@126.com>
# date 2020/6

###########################################################################################

cmake_minimum_required(VERSION 2.8.12)

include(deps/libhp/cmakefiles/libhp.cmake) # hp_cmake_find_deps
# cpp-test
project(cpp-test)

###########################################################################################

if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Options are: None, Debug, Release, RelWithDebInfo, MinSizeRel." FORCE)
endif()
# -DCMAKE_EXPORT_COMPILE_COMMANDS is for lsp/clangd, see
# https://clangd.llvm.org/installation.html
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

###########################################################################################
# sources and deps
set(g_withs UV CURL 1 1 1 1 1
	)
set(g_hdrs uv.h curl/curl.h .nullfilesub.h .nullfilesrc.h .nullfilesrc.h google/protobuf-c/protobuf-c.h zlib.h
	)
set(g_incs .find_path .find_path "deps/libhp/include/ deps/libhp/deps"  "${PROJECT_BINARY_DIR}/ deps/ src/" .find_path .find_path .find_path)
set(g_deps uv curl libhp .src phxpaxos protobuf-c zlib
	)
set(g_libs uv curl libhp "src/**/**/*.c src/**/*.c src/**/*.cpp src/**/*.cc src/* src/*.cc"
	"deps/phxpaxos/**/*.c"
	protobuf-c
	z
	)
###########################################################################################
# for libhp
#  set(LIBHP_WITH_MQTT 1 CACHE INTERNAL "LIBHP_WITH_MQTT")
# fot cpp-test
set(CPP_TEST_WITH_UV 1 CACHE INTERNAL "CPP_TEST_WITH_UV")
set(CPP_TEST_WITH_CURL 1 CACHE INTERNAL "CPP_TEST_WITH_CURL")
hp_cmake_find_deps(SRCS CPP_TEST_WITH_ deps g_withs g_hdrs g_incs g_deps g_libs)
#build as library
add_executable(cpp-test ${SRCS})

foreach(dep IN LISTS g_deps  )
	message("cpp-test: dep='${dep}', ${dep}_INCLUDE_DIRS='${${dep}_INCLUDE_DIRS}', ${dep}_LIBRARIES='${${dep}_LIBRARIES}'")
	target_include_directories(cpp-test PRIVATE ${${dep}_INCLUDE_DIRS} )
	target_link_libraries(cpp-test ${${dep}_LIBRARIES} ) 
endforeach() 

target_compile_options(cpp-test PUBLIC
	-DHAVE_CONFIG_H
    -DCMAKE_EXPORT_COMPILE_COMMANDS
    # for redis
    -DDICT_BENCHMARK_MAIN
	# for mg
	-DMG_ENABLE_DIRECTORY_LISTING
    # for cJSON
    -DcJSON_FREE_PRINT
)
if(MSVC)
	target_compile_options(cpp-test PUBLIC -D_CRT_SECURE_NO_WARNINGS /MTd)
endif()

###########################################################################################
configure_file("config.h.in" "config.h")


