################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/xh_vircam/subdir.mk
-include src/win32/subdir.mk
-include src/vonmenw-pbft/libbyz/subdir.mk
-include src/vonmenw-pbft/bft-simple/subdir.mk
-include src/usb/subdir.mk
-include src/tests/subdir.mk
-include src/test_libwx/subdir.mk
-include src/springboot-sidecar/subdir.mk
-include src/spdlog/subdir.mk
-include src/sm/subdir.mk
-include src/sds/subdir.mk
-include src/protobuf_libev/subdir.mk
-include src/protobuf-c/subdir.mk
-include src/phxecho/logger_google/subdir.mk
-include src/phxecho/subdir.mk
-include src/paxos/subdir.mk
-include src/nghhtp2/subdir.mk
-include src/mysql/subdir.mk
-include src/mmpool/subdir.mk
-include src/mem_pool/subdir.mk
-include src/llvm/subdir.mk
-include src/liblog4cplus/subdir.mk
-include src/jzoffer/subdir.mk
-include src/hiredis/subdir.mk
-include src/cti/subdir.mk
-include src/chess/src/subdir.mk
-include src/bluez/subdir.mk
-include src/api_poll/subdir.mk
-include src/alsa_pcm/subdir.mk
-include src/algs/merge_sort_link_list/subdir.mk
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
all: libcpp-test.a

# Tool invocations
libcpp-test.a: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC Archiver'
	ar -r  "libcpp-test.a" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(ARCHIVES)$(C_UPPER_DEPS)$(CXX_DEPS)$(OBJS)$(CPP_DEPS)$(C_DEPS) libcpp-test.a
	-@echo ' '

.PHONY: all clean dependents

-include ../makefile.targets
