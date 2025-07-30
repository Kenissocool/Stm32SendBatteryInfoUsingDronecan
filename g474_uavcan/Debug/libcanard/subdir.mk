################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libcanard/canard.c 

OBJS += \
./libcanard/canard.o 

C_DEPS += \
./libcanard/canard.d 


# Each subdirectory must supply rules for building sources it contributes
libcanard/%.o libcanard/%.su libcanard/%.cyclo: ../libcanard/%.c libcanard/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I"/home/terapat/STM32CubeIDE/workspace_1.19.0/g474_uavcan/libcanard" -I"/home/terapat/STM32CubeIDE/workspace_1.19.0/g474_uavcan/dronecan/include" -I"/home/terapat/STM32CubeIDE/workspace_1.19.0/g474_uavcan/dronecan/src" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-libcanard

clean-libcanard:
	-$(RM) ./libcanard/canard.cyclo ./libcanard/canard.d ./libcanard/canard.o ./libcanard/canard.su

.PHONY: clean-libcanard

