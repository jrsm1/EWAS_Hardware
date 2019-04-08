################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
boardConfigs/MSP_EXP432P401R.obj: ../boardConfigs/MSP_EXP432P401R.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"/opt/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="/home/jrsm/ccs_workspace/NDT_Control_Module" --include_path="/opt/ti/simplelink_msp432p4_sdk_2_30_00_14/source" --include_path="/opt/ti/simplelink_msp432p4_sdk_2_30_00_14/kernel/nortos" --include_path="/opt/ti/simplelink_msp432p4_sdk_2_30_00_14/kernel/nortos/posix" --include_path="/opt/ti/simplelink_msp432p4_sdk_2_30_00_14/source/third_party/CMSIS/Include" --include_path="/opt/ti/ccsv6/tools/compiler/arm_15.12.3.LTS/include" --advice:power=none -g --define=__MSP432P401R__ --define=DeviceFamily_MSP432P401x --display_error_number --diag_warning=225 --diag_warning=255 --diag_wrap=off --gen_func_subsections=on --preproc_with_compile --preproc_dependency="boardConfigs/MSP_EXP432P401R.d" --obj_directory="boardConfigs" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '


