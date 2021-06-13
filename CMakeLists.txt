# author hongjun.liao <docici@126.com>
# date 2021/6/12

cmake_minimum_required(VERSION 2.8)

# cpp-test 
project(cpp-test)
add_executable(cpp-test src/main.c
	src/algs/9787302356288.c
)
target_compile_options(cpp-test PUBLIC
    -DHAVE_CONFIG_H -D_GNU_SOURCE -DCMAKE_EXPORT_COMPILE_COMMANDS
)
if(NOT WIN32)
	target_compile_options(cpp-test PUBLIC  -O0 -g3  -Wall)
else()
	target_compile_options(cpp-test PUBLIC -D_CRT_SECURE_NO_WARNINGS  /MTd)
endif()
