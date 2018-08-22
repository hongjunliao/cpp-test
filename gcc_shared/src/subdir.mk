################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/chksum.cpp \
../src/cpp_test.cpp \
../src/hexdump.cpp \
../src/string_util_url.cpp \
../src/termio_util.cpp \
../src/test_cpp11.cpp \
../src/xhsdk_select_server.cpp 

C_SRCS += \
../src/hp_epoll.c \
../src/hp_io.c \
../src/hp_log.c \
../src/hp_net.c \
../src/hp_sig.c \
../src/list_sort.c \
../src/net_util.c \
../src/string_dump.c \
../src/string_util.c \
../src/tcp_echo_cli.c \
../src/test_socketpair_fork.c \
../src/test_string_util.c \
../src/test_zlib_2.c \
../src/udp_echo_cli.c 

OBJS += \
./src/chksum.o \
./src/cpp_test.o \
./src/hexdump.o \
./src/hp_epoll.o \
./src/hp_io.o \
./src/hp_log.o \
./src/hp_net.o \
./src/hp_sig.o \
./src/list_sort.o \
./src/net_util.o \
./src/string_dump.o \
./src/string_util.o \
./src/string_util_url.o \
./src/tcp_echo_cli.o \
./src/termio_util.o \
./src/test_cpp11.o \
./src/test_socketpair_fork.o \
./src/test_string_util.o \
./src/test_zlib_2.o \
./src/udp_echo_cli.o \
./src/xhsdk_select_server.o 

CPP_DEPS += \
./src/chksum.d \
./src/cpp_test.d \
./src/hexdump.d \
./src/string_util_url.d \
./src/termio_util.d \
./src/test_cpp11.d \
./src/xhsdk_select_server.d 

C_DEPS += \
./src/hp_epoll.d \
./src/hp_io.d \
./src/hp_log.d \
./src/hp_net.d \
./src/hp_sig.d \
./src/list_sort.d \
./src/net_util.d \
./src/string_dump.d \
./src/string_util.d \
./src/tcp_echo_cli.d \
./src/test_socketpair_fork.d \
./src/test_string_util.d \
./src/test_zlib_2.d \
./src/udp_echo_cli.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -D_GNU_SOURCE -DWITH_LIBPROTOBUF_LIBEV=0 -I"/home/jun/sdb1/cpp-test/inc" -I"/home/jun/sdb1/cpp-test/src" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D_GNU_SOURCE -I"/home/jun/sdb1/cpp-test/inc" -I"/home/jun/sdb1/cpp-test/src" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


