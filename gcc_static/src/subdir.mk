################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/chksum.cpp \
../src/hexdump.cpp \
../src/hp_sig.cpp \
../src/plcdn_cpp_test.cpp \
../src/string_util_url.cpp \
../src/tcp_echo_cli.cpp \
../src/termio_util.cpp \
../src/test_cpp11.cpp \
../src/test_string_util.cpp \
../src/xhsdk_select_server.cpp 

C_SRCS += \
../src/net_util.c \
../src/string_dump.c \
../src/string_util.c 

OBJS += \
./src/chksum.o \
./src/hexdump.o \
./src/hp_sig.o \
./src/net_util.o \
./src/plcdn_cpp_test.o \
./src/string_dump.o \
./src/string_util.o \
./src/string_util_url.o \
./src/tcp_echo_cli.o \
./src/termio_util.o \
./src/test_cpp11.o \
./src/test_string_util.o \
./src/xhsdk_select_server.o 

CPP_DEPS += \
./src/chksum.d \
./src/hexdump.d \
./src/hp_sig.d \
./src/plcdn_cpp_test.d \
./src/string_util_url.d \
./src/tcp_echo_cli.d \
./src/termio_util.d \
./src/test_cpp11.d \
./src/test_string_util.d \
./src/xhsdk_select_server.d 

C_DEPS += \
./src/net_util.d \
./src/string_dump.d \
./src/string_util.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -D_GNU_SOURCE -I"/home/jun/ws/cpp-test/inc" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -std=c11 -I"/home/jun/ws/cpp-test/inc" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


