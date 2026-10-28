/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_ll_bus.h"
#include "stm32l0xx_ll_rcc.h"

#include "tusb.h"

void usb_init(void)
{
  tusb_rhport_init_t dev_init = {.role = TUSB_ROLE_DEVICE, .speed = TUSB_SPEED_AUTO};

  /* HSI48 enabled and trimmed by CRS in SystemClock_Config() */
  LL_RCC_SetUSBClockSource(LL_RCC_USB_CLKSOURCE_HSI48);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USB);

  tusb_init(0, &dev_init);
}

void USB_IRQHandler(void)
{
  tud_int_handler(0);
}
