################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/xh_vircam/subdir.mk
-include src/win32/subdir.mk
-include src/usb/subdir.mk
-include src/tests/subdir.mk
-include src/test_libwx/subdir.mk
-include src/springboot-sidecar/subdir.mk
-include src/nghhtp2/subdir.mk
-include src/mmpool/subdir.mk
-include src/mem_pool/subdir.mk
-include src/jzoffer/subdir.mk
-include src/cti/subdir.mk
-include src/chess/src/subdir.mk
-include src/bluez/subdir.mk
-include src/alsa_pcm/subdir.mk
-include src/algs/subdir.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: libcpp-test.so

# Tool invocations
libcpp-test.so: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++ -L../lib/ -shared -o "libcpp-test.so" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(LIBRARIES)$(CC_DEPS)$(C++_DEPS)$(C_UPPER_DEPS)$(CXX_DEPS)$(OBJS)$(CPP_DEPS)$(C_DEPS) libcpp-test.so
	-@echo ' '

post-build:
	-install libcpp-test.so ../bin/
	-@echo ' '

.PHONY: all clean dependents post-build

-include ../makefile.targets
