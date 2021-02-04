################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := D:/AveryW/SchoolStuff/WPI/ccs10_1/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS

GEN_OPTS__FLAG := --cmd_file="D:/AveryW/SchoolStuff/WPI/ccs10_1/Workspace/tirtos_builds_cc13x2_cc26x2_release_ccs/Debug/configPkg/compiler.opt" 
GEN_CMDS__FLAG := -l"D:/AveryW/SchoolStuff/WPI/ccs10_1/Workspace/tirtos_builds_cc13x2_cc26x2_release_ccs/Debug/configPkg/linker.cmd" 

ORDERED_OBJS += \
"./bio_sensor.obj" \
"./bio_sensor_library_test.obj" \
"./syscfg/ti_devices_config.obj" \
"./syscfg/ti_drivers_config.obj" \
"./main_tirtos.obj" \
"../cc13x2_cc26x2_tirtos.cmd" \
-lti_utils_build_linker.cmd.genlibs \
$(GEN_CMDS__FLAG) \
-l"ti/devices/cc13x2_cc26x2/driverlib/bin/ccs/driverlib.lib" \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
Bio_Sensor_Library_Test.out \

EXE_OUTPUTS__QUOTED += \
"Bio_Sensor_Library_Test.out" \

BIN_OUTPUTS += \
Bio_Sensor_Library_Test.hex \

BIN_OUTPUTS__QUOTED += \
"Bio_Sensor_Library_Test.hex" \


# All Target
all: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "Bio_Sensor_Library_Test.out"

dependents:
	-cd D:/AveryW/SchoolStuff/WPI/ccs10_1/Workspace/tirtos_builds_cc13x2_cc26x2_release_ccs/Debug && $(MAKE) all

# Tool invocations
Bio_Sensor_Library_Test.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS) D:/AveryW/SchoolStuff/WPI/ccs10_1/Workspace/tirtos_builds_cc13x2_cc26x2_release_ccs/Debug/tirtos_builds_cc13x2_cc26x2_release_ccs.lib
	@echo 'Building target: "$@"'
	@echo 'Invoking: ARM Linker'
	"D:/AveryW/SchoolStuff/WPI/ccs10_1/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --define=DeviceFamily_CC26X2 -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on -z -m"Bio_Sensor_Library_Test.map" -i"C:/ti/simplelink_cc13x2_26x2_sdk_4_30_00_54/source" -i"C:/ti/simplelink_cc13x2_26x2_sdk_4_30_00_54/kernel/tirtos/packages" -i"D:/AveryW/SchoolStuff/WPI/ccs10_1/Workspace/Bio_Sensor_Library_Test/Debug/syscfg/" -i"D:/AveryW/SchoolStuff/WPI/ccs10_1/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/lib" --diag_wrap=off --display_error_number --warn_sections --xml_link_info="Bio_Sensor_Library_Test_linkInfo.xml" --rom_model -o "Bio_Sensor_Library_Test.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

Bio_Sensor_Library_Test.hex: $(EXE_OUTPUTS)
	@echo 'Building files: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo 'Invoking: ARM Hex Utility'
	"D:/AveryW/SchoolStuff/WPI/ccs10_1/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/bin/armhex"  -o "Bio_Sensor_Library_Test.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(GEN_MISC_FILES__QUOTED)$(BIN_OUTPUTS__QUOTED)$(GEN_FILES__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "bio_sensor.obj" "bio_sensor_library_test.obj" "syscfg\ti_devices_config.obj" "syscfg\ti_drivers_config.obj" "main_tirtos.obj" 
	-$(RM) "bio_sensor.d" "bio_sensor_library_test.d" "syscfg\ti_devices_config.d" "syscfg\ti_drivers_config.d" "main_tirtos.d" 
	-$(RMDIR) $(GEN_MISC_DIRS__QUOTED)
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
D:/AveryW/SchoolStuff/WPI/ccs10_1/Workspace/tirtos_builds_cc13x2_cc26x2_release_ccs/Debug/tirtos_builds_cc13x2_cc26x2_release_ccs.lib:

-include ../makefile.targets

