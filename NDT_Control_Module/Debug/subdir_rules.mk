################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.1.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/Juan/Downloads/NDT_Control_Module2" --include_path="C:/ti/simplelink_msp432p4_sdk_2_30_00_14/source" --include_path="C:/ti/simplelink_msp432p4_sdk_2_30_00_14/kernel/nortos" --include_path="C:/ti/simplelink_msp432p4_sdk_2_30_00_14/kernel/nortos/posix" --include_path="C:/ti/simplelink_msp432p4_sdk_2_30_00_14/source/third_party/CMSIS/Include" --include_path="C:/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.1.LTS/include" --advice:power=none -g --define=__MSP432P401R__ --define=DeviceFamily_MSP432P401x --display_error_number --diag_wrap=off --diag_warning=225 --diag_warning=255 --gen_func_subsections=on --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


