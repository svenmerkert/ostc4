cmake_minimum_required(VERSION 3.20)
SET(PROJECTNAME OSTC4)

project("${PROJECTNAME}")


SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set (CMAKE_VERBOSE_MAKEFILE ON)
SET(ASM_OPTIONS "-x assembler-with-cpp")


SET(PROJECT_CPU1 "OSTC4_firmware")
SET(PROJECT_CPU2 "OSTC4_RTE")


ENABLE_LANGUAGE(C CXX ASM)

#
# Setup the processor definitions
#
SET(CPUFLAGS "-mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb -DDEBUG")
SET(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${CPUFLAGS} -D__SRAM_SIZE=0x8C000 -Wall  -fno-common  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin  -mapcs  -std=gnu99")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -MMD ${CPUFLAGS}  -std=c++11 -felide-constructors -fno-exceptions -fno-rtti -ffunction-sections  -fdata-sections" )
SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Os -MMD ${CPUFLAGS} -ffunction-sections  -fdata-sections" )
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--print-memory-usage -Wl,--gc-sections ${CPUFLAGS} -u _printf_float -fno-rtti -fno-exceptions -Xlinker -Map=output.map")


add_executable( ${PROJECT_CPU1} )

set_target_properties(
    ${PROJECT_CPU1}
    PROPERTIES 
        OUTPUT_NAME ${PROJECT_CPU1}		
		COMPILE_OPTIONS -DSTM32F429xx
		LINK_OPTIONS -T${CMAKE_CURRENT_SOURCE_DIR}/Common/CPU1-F429.ld
        SUFFIX ""
)

target_sources("OSTC4_firmware" PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Src/calc_crush.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Src/decom.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/Src/syscalls.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_can.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cec.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_crc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cryp.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cryp_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dac.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dac_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dcmi.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dcmi_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dfsdm.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma2d.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dsi.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_eth.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_fmpi2c.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_fmpi2c_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_hash.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_hash_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_hcd.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2s.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2s_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_irda.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_iwdg.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_lptim.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_ltdc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_ltdc_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_mmc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_nand.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_nor.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pccard.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_qspi.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rng.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rtc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rtc_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sai.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sai_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sd.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sdram.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_smartcard.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spdifrx.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sram.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_usart.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_wwdg.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_adc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_crc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_dac.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_dma.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_dma2d.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_exti.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_fmc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_fsmc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_gpio.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_i2c.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_lptim.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_pwr.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_rcc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_rng.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_rtc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_sdmmc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_spi.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_tim.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usart.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_utils.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/base.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/buehlmann.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/check_warning.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/crcmodel.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/data_central.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/data_exchange_main.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/demo.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/display.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/externCPU2bootloader.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/externLogbookFlash.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/firmwareJumpToApplication.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/gfx_colors.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/gfx_engine.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/gfx_fonts.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/logbook.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/logbook_miniLive.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/motion.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/ostc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/settings.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/show_logbook.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/simulation.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/startup_stm32f429xx.s
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/stm32f4xx_hal_msp_hw2.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/stm32f4xx_it.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/system_stm32f4xx_special_plus_256k.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/t3.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/t4_tetris.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/t5_gauge.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/t6_apnea.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/t7.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tCCR.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tComm.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tDebug.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/test_vpm.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/text_multilanguage.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tHome.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/timer.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tInfo.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tInfoCompass.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tInfoLog.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tInfoSensor.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tMenu.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tMenuCustom.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tMenuDeco.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tMenuDecoParameter.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tMenuEdit.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tMenuEditCustom.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tMenuEditDeco.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tMenuEditDecoParameter.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tMenuEditGasOC.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tMenuEditHardware.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tMenuEditPlanner.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tMenuEditSetpoint.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tMenuEditSystem.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tMenuEditXtra.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tMenuGas.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tMenuHardware.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tMenuPlanner.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tMenuSetpoint.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tMenuSystem.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/tMenuXtra.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/unit.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Src/vpm.c
)

target_include_directories("OSTC4_firmware" PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Discovery/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_DRIVER/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/CMSIS/Include)


add_custom_command(TARGET ${PROJECT_CPU1} POST_BUILD COMMAND ${ARMTOOLPATH}/bin/arm-none-eabi-size $<TARGET_FILE:${PROJECT_CPU1}>
                                                    COMMAND ${ARMTOOLPATH}/bin/arm-none-eabi-objcopy -R .upper* -R.font_firmware_data -O binary -R .eeprom $<TARGET_FILE:${PROJECT_CPU1}> $<TARGET_FILE:${PROJECT_CPU1}>.bin
                                                    COMMAND ${ARMTOOLPATH}/bin/arm-none-eabi-objcopy -R .upper* -R.font_firmware_data -O ihex -R .eeprom $<TARGET_FILE:${PROJECT_CPU1}> $<TARGET_FILE:${PROJECT_CPU1}>.ihex
                                                    COMMAND ${ARMTOOLPATH}/bin/arm-none-eabi-objdump -x -d $<TARGET_FILE:${PROJECT_CPU1}> >$<TARGET_FILE:${PROJECT_CPU1}>.lst)



add_executable( ${PROJECT_CPU2} )

set_target_properties(
    ${PROJECT_CPU2}
    PROPERTIES 
        OUTPUT_NAME ${PROJECT_CPU2}		
		COMPILE_OPTIONS -DSTM32F411xE
		LINK_OPTIONS "-T${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/CPU2-RTE.ld"
        SUFFIX ""
)
target_link_libraries(${PROJECT_CPU2} m)


target_sources("OSTC4_RTE" PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Src/calc_crush.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Src/decom.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/Src/syscalls.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_can.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cec.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_crc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cryp.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cryp_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dac.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dac_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dcmi.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dcmi_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dfsdm.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma2d.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dsi.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_eth.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_fmpi2c.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_fmpi2c_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_hash.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_hash_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_hcd.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2s.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2s_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_irda.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_iwdg.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_lptim.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_ltdc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_ltdc_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_mmc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_nand.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_nor.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pccard.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_qspi.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rng.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rtc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rtc_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sai.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sai_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sd.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sdram.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_smartcard.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spdifrx.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sram.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_usart.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_wwdg.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_adc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_crc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_dac.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_dma.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_dma2d.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_exti.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_fmc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_fsmc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_gpio.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_i2c.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_lptim.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_pwr.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_rcc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_rng.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_rtc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_sdmmc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_spi.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_tim.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usart.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_utils.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Src/adc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Src/baseCPU2.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Src/batteryCharger.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Src/batteryGasGauge.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Src/compass.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Src/dma.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Src/externalInterface.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Src/i2c.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Src/pressure.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Src/rtc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Src/RTE_FlashAccess.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Src/scheduler.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Src/spi.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Src/startup_stm32f4xx.S
   ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Src/stm32f4xx_hal_msp_v3.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Src/stm32f4xx_it_v3.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Src/system_stm32f4xx.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Src/tm_stm32f4_otp.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Src/uart.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Src/uartProtocol_Co2.c
   ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Src/uartProtocol_O2.c
)

target_include_directories("OSTC4_RTE" PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Small_CPU/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/STM32F4xx_HAL_DRIVER/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Drivers/CMSIS/Include)


add_custom_command(TARGET ${PROJECT_CPU2} POST_BUILD COMMAND ${ARMTOOLPATH}/bin/arm-none-eabi-size $<TARGET_FILE:${PROJECT_CPU2}>
													COMMAND ${ARMTOOLPATH}/bin/arm-none-eabi-objcopy -R .upper* -R.font_firmware_data -O binary -R .eeprom $<TARGET_FILE:${PROJECT_CPU2}> $<TARGET_FILE:${PROJECT_CPU2}>.bin
                                                    COMMAND ${ARMTOOLPATH}/bin/arm-none-eabi-objcopy -R .upper* -R.font_firmware_data -O ihex -R .eeprom $<TARGET_FILE:${PROJECT_CPU2}> $<TARGET_FILE:${PROJECT_CPU2}>.ihex
                                                    COMMAND ${ARMTOOLPATH}/bin/arm-none-eabi-objdump -x -d $<TARGET_FILE:${PROJECT_CPU2}> >$<TARGET_FILE:${PROJECT_CPU2}>.lst)


