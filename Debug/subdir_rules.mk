################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"D:/AveryW/SchoolStuff/WPI/ccs10_1/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="D:/AveryW/SchoolStuff/WPI/ccs10_1/Workspace/Bio_Sensor_Library_Test" --include_path="D:/AveryW/SchoolStuff/WPI/ccs10_1/Workspace/Bio_Sensor_Library_Test/Debug" --include_path="C:/ti/simplelink_cc13x2_26x2_sdk_4_30_00_54/source" --include_path="C:/ti/simplelink_cc13x2_26x2_sdk_4_30_00_54/source/ti/posix/ccs" --include_path="D:/AveryW/SchoolStuff/WPI/ccs10_1/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/include" --define=DeviceFamily_CC26X2 -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" --include_path="D:/AveryW/SchoolStuff/WPI/ccs10_1/Workspace/Bio_Sensor_Library_Test/Debug/syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-1283259935: ../bio_sensor_library_test.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"D:/AveryW/SchoolStuff/WPI/ccs10_1/ccs/utils/sysconfig_1.6.0/sysconfig_cli.bat" -s "C:/ti/simplelink_cc13x2_26x2_sdk_4_30_00_54/.metadata/product.json" -o "syscfg" --compiler ccs "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/ti_devices_config.c: build-1283259935 ../bio_sensor_library_test.syscfg
syscfg/ti_drivers_config.c: build-1283259935
syscfg/ti_drivers_config.h: build-1283259935
syscfg/ti_utils_build_linker.cmd.genlibs: build-1283259935
syscfg/syscfg_c.rov.xs: build-1283259935
syscfg/ti_utils_runtime_model.gv: build-1283259935
syscfg/ti_utils_runtime_Makefile: build-1283259935
syscfg/: build-1283259935

syscfg/%.obj: ./syscfg/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"D:/AveryW/SchoolStuff/WPI/ccs10_1/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="D:/AveryW/SchoolStuff/WPI/ccs10_1/Workspace/Bio_Sensor_Library_Test" --include_path="D:/AveryW/SchoolStuff/WPI/ccs10_1/Workspace/Bio_Sensor_Library_Test/Debug" --include_path="C:/ti/simplelink_cc13x2_26x2_sdk_4_30_00_54/source" --include_path="C:/ti/simplelink_cc13x2_26x2_sdk_4_30_00_54/source/ti/posix/ccs" --include_path="D:/AveryW/SchoolStuff/WPI/ccs10_1/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/include" --define=DeviceFamily_CC26X2 -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="syscfg/$(basename $(<F)).d_raw" --include_path="D:/AveryW/SchoolStuff/WPI/ccs10_1/Workspace/Bio_Sensor_Library_Test/Debug/syscfg" --obj_directory="syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


