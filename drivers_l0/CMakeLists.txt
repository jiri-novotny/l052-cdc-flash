set(SOURCE
   ${SOURCE}
   ${CMAKE_CURRENT_SOURCE_DIR}/../lib/cmsis-device-l0/Source/Templates/gcc/startup_stm32l052xx.s
   ${CMAKE_CURRENT_SOURCE_DIR}/../lib/cmsis-device-l0/Source/Templates/system_stm32l0xx.c

   ${CMAKE_CURRENT_SOURCE_DIR}/../lib/stm32l0xx-hal-driver/Src/stm32l0xx_hal_cortex.c
   ${CMAKE_CURRENT_SOURCE_DIR}/../lib/stm32l0xx-hal-driver/Src/stm32l0xx_hal.c
   ${CMAKE_CURRENT_SOURCE_DIR}/../lib/stm32l0xx-hal-driver/Src/stm32l0xx_ll_crs.c
   ${CMAKE_CURRENT_SOURCE_DIR}/../lib/stm32l0xx-hal-driver/Src/stm32l0xx_ll_gpio.c
   ${CMAKE_CURRENT_SOURCE_DIR}/../lib/stm32l0xx-hal-driver/Src/stm32l0xx_ll_rcc.c
   ${CMAKE_CURRENT_SOURCE_DIR}/../lib/stm32l0xx-hal-driver/Src/stm32l0xx_ll_usart.c
   ${CMAKE_CURRENT_SOURCE_DIR}/../lib/stm32l0xx-hal-driver/Src/stm32l0xx_ll_utils.c

   ${CMAKE_CURRENT_SOURCE_DIR}/clk.c
   ${CMAKE_CURRENT_SOURCE_DIR}/gpio.c
   ${CMAKE_CURRENT_SOURCE_DIR}/usart.c
   ${CMAKE_CURRENT_SOURCE_DIR}/usb.c
   ${CMAKE_CURRENT_SOURCE_DIR}/watchdog.c

   PARENT_SCOPE
)
