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
../src/hp_sig.c \
../src/net_util.c \
../src/string_dump.c \
../src/string_util.c \
../src/tcp_echo_cli.c \
../src/test_string_util.c \
../src/udp_echo_cli.c 

OBJS += \
./src/chksum.o \
./src/cpp_test.o \
./src/hexdump.o \
./src/hp_sig.o \
./src/net_util.o \
./src/string_dump.o \
./src/string_util.o \
./src/string_util_url.o \
./src/tcp_echo_cli.o \
./src/termio_util.o \
./src/test_cpp11.o \
./src/test_string_util.o \
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
./src/hp_sig.d \
./src/net_util.d \
./src/string_dump.d \
./src/string_util.d \
./src/tcp_echo_cli.d \
./src/test_string_util.d \
./src/udp_echo_cli.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -D_GNU_SOURCE -I"/home/jun/ws/cpp-test/inc" -I../src/ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -std=c11 -D_GNU_SOURCE -I"/home/jun/ws/cpp-test/inc" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


