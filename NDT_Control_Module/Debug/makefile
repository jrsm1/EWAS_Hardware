################################################################################
# Automatically-generated file. Do not edit!
################################################################################

CG_TOOL_ROOT := /opt/ti/ccsv6/tools/compiler/arm_15.12.3.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./main.obj" \
"./system_msp432p401r.obj" \
"./boardConfigs/MSP_EXP432P401R.obj" \
"./ccs/startup_msp432p401r_ccs.obj" \
"./utils/cmd.obj" \
"./utils/cmdline.obj" \
"./utils/printf.obj" \
"../msp432p401r.cmd" \
$(GEN_CMDS__FLAG) \
-lti/display/lib/display.aem4f \
-lti/grlib/lib/ccs/m4f/grlib.a \
-lthird_party/spiffs/lib/ccs/m4f/spiffs.a \
-lti/drivers/lib/drivers_msp432p401x.aem4f \
-lthird_party/fatfs/lib/ccs/m4f/fatfs.a \
-lti/devices/msp432p4xx/driverlib/ccs/msp432p4xx_driverlib.lib \
-llibc.a \
-llib/nortos_msp432p401x.aem4f \

-include ../makefile.init

RM := rm -rf
RMDIR := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include boardConfigs/subdir_vars.mk
-include ccs/subdir_vars.mk
-include utils/subdir_vars.mk
-include subdir_rules.mk
-include boardConfigs/subdir_rules.mk
-include ccs/subdir_rules.mk
-include utils/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
NDT_Control_Module.out \

EXE_OUTPUTS__QUOTED += \
"NDT_Control_Module.out" \

BIN_OUTPUTS += \
NDT_Control_Module.hex \

BIN_OUTPUTS__QUOTED += \
"NDT_Control_Module.hex" \


# All Target
all: NDT_Control_Module.out

# Tool invocations
NDT_Control_Module.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: MSP432 Linker'
	"/opt/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --advice:power=none -g --define=__MSP432P401R__ --define=DeviceFamily_MSP432P401x --display_error_number --diag_warning=225 --diag_warning=255 --diag_wrap=off --gen_func_subsections=on -z -m"NDT_Control_Module.map" --stack_size=512 -i"/opt/ti/simplelink_msp432p4_sdk_2_30_00_14/source" -i"/opt/ti/simplelink_msp432p4_sdk_2_30_00_14/kernel/nortos" -i"/opt/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/lib" -i"/opt/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" --reread_libs --warn_sections --display_error_number --diag_wrap=off --xml_link_info="NDT_Control_Module_linkInfo.xml" --rom_model -o "NDT_Control_Module.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

NDT_Control_Module.hex: $(EXE_OUTPUTS)
	@echo 'Invoking: MSP432 Hex Utility'
	"/opt/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armhex"  -o "NDT_Control_Module.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXE_OUTPUTS__QUOTED)$(BIN_OUTPUTS__QUOTED)
	-$(RM) "main.d" "system_msp432p401r.d" "boardConfigs/MSP_EXP432P401R.d" "ccs/startup_msp432p401r_ccs.d" "utils/cmd.d" "utils/cmdline.d" "utils/printf.d" 
	-$(RM) "main.obj" "system_msp432p401r.obj" "boardConfigs/MSP_EXP432P401R.obj" "ccs/startup_msp432p401r_ccs.obj" "utils/cmd.obj" "utils/cmdline.obj" "utils/printf.obj" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

