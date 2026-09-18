################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../drivers/bsp/HC-SR04/stm32g4_hcsr04.c 

OBJS += \
./drivers/bsp/HC-SR04/stm32g4_hcsr04.o 

C_DEPS += \
./drivers/bsp/HC-SR04/stm32g4_hcsr04.d 


# Each subdirectory must supply rules for building sources it contributes
drivers/bsp/HC-SR04/%.o drivers/bsp/HC-SR04/%.su drivers/bsp/HC-SR04/%.cyclo: ../drivers/bsp/HC-SR04/%.c drivers/bsp/HC-SR04/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G431xx -c -I../core/Inc -I../drivers/stm32g4xx_hal/Inc -I../drivers/stm32g4xx_hal/Inc/Legacy -I../drivers/cmsis/Device/ST/STM32G4xx/Include -I../drivers/cmsis/Include -I../app -I../drivers/bsp -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-drivers-2f-bsp-2f-HC-2d-SR04

clean-drivers-2f-bsp-2f-HC-2d-SR04:
	-$(RM) ./drivers/bsp/HC-SR04/stm32g4_hcsr04.cyclo ./drivers/bsp/HC-SR04/stm32g4_hcsr04.d ./drivers/bsp/HC-SR04/stm32g4_hcsr04.o ./drivers/bsp/HC-SR04/stm32g4_hcsr04.su

.PHONY: clean-drivers-2f-bsp-2f-HC-2d-SR04

