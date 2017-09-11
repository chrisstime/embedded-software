################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/SPI.c \
../Sources/analog.c \
../Sources/main.c \
../Sources/median.c 

OBJS += \
./Sources/SPI.o \
./Sources/analog.o \
./Sources/main.o \
./Sources/median.o 

C_DEPS += \
./Sources/SPI.d \
./Sources/analog.d \
./Sources/main.d \
./Sources/median.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/%.o: ../Sources/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"C:\Users\11986282\Desktop\Lab4withLab3Lib\Library" -I"C:/Users/11986282/Desktop/Lab4withLab3Lib/Static_Code/IO_Map" -I"C:/Users/11986282/Desktop/Lab4withLab3Lib/Sources" -I"C:/Users/11986282/Desktop/Lab4withLab3Lib/Generated_Code" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


