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
set(CMAKE_BUILD_TYPE Release CACHE STRING "Options are: None, Debug, Release, RelWithDebInfo, MinSizeRel." FORCE)
endif()
# -DCMAKE_EXPORT_COMPILE_COMMANDS is for lsp/clangd, see
# https://clangd.llvm.org/installation.html
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

###########################################################################################
# deps from deps/ 
# pkg_check_modules(openssl REQUIRED openssl)

# FIXME:
# cause pkg-config/hiredis has no redisAsyncContext::dataCleanup member, using deps/hiredis instead
# pkg_check_modules(hiredis REQUIRED hiredis)

# 为数字的表示不受LIBHP_WITH_XXX选项开关的控制
set(g_withs 1 1 1 1 1
	)
# 为.nullfilesub.h或.nullfilesrc.h的表示不要查找系统目录来定位库
#	.nullfilesub.h: 通过add_subdirectory()
#	.nullfilesrc.h: 通过file(GLOB)
set(g_hdrs .nullfilesub.h .nullfilesrc.h .nullfilesrc.h .nullfilesrc.h zlib.h
	)
#.src表示项目源目录,不需要查检deps/下相应目录是否存在
set(g_deps libhp .src phxpaxos protobuf-c zlib
	)
# 空格隔开的表示实际为数组
set(g_libs libhp "src/**/**/*.c src/**/*.c src/**/*.cpp src/**/*.cc src/* src/*.cc"
	"deps/phxpaxos/**/*.c"
	"deps/protobuf-c/**/*.c"
	z
	)
###########################################################################################

hp_cmake_find_deps(SRCS g_withs g_hdrs g_deps g_libs)
#build as library
add_executable(cpp-test ${SRCS})

foreach(dep IN LISTS g_deps  )
	message("cpp-test: dep='${dep}', ${dep}_INCLUDE_DIRS='${${dep}_INCLUDE_DIRS}', ${dep}_LIBRARIES='${${dep}_LIBRARIES}'")
	if(NOT (${dep} STREQUAL .src))
		target_include_directories(cpp-test PRIVATE ${${dep}_INCLUDE_DIRS} )
		target_link_libraries(cpp-test ${${dep}_LIBRARIES} ) 
	endif()
endforeach() 
target_include_directories(cpp-test PRIVATE "${PROJECT_BINARY_DIR}/" deps/libhp/include/ deps/ src/)

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


