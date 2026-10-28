#include <stdint.h>

#include CMSIS_device_header

#include "cdc_bridge.h"
#include "clk.h"
#include "gpio.h"
#include "usart.h"
#include "usb.h"
#include "watchdog.h"

#include "tusb.h"

/* Private user code ---------------------------------------------------------*/

void HardFault_Handler(void)
{
  NVIC_SystemReset();
}

/* Public user code ---------------------------------------------------------*/

int main(void)
{
  uint32_t blink_timer = 0;
  uint32_t button_debounce = 0;

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  watchdog_init();
  gpio_init();
  u2_init();
  cdc_bridge_init();
  usb_init();

  uint32_t tick = 0;
  uint32_t tick10ms = HAL_GetTick();
  while (1)
  {
    /* run fast */
    tick = HAL_GetTick();
    tud_task(); // tinyusb device task
    cdc_bridge_task();
    if ((tick - tick10ms) < 10)
    {
      continue;
    }
    tick10ms = tick;

    /* run every 10ms */
    cdc_bridge_tick();

    blink_timer++;
    if (blink_timer == 1)
    {
      led_run(true);
    }
    else if (blink_timer == 2)
    {
      led_run(false);
    }
    else if (blink_timer == 100)
    {
      blink_timer = 0;
      WDI();
    }

    /* USER button toggles target power */
    if (button_debounce == 14)
    {
      button_debounce++;
      cdc_bridge_power(!cdc_bridge_power_state());
    }
    else if (button())
    {
      if (button_debounce < 14)
      {
        button_debounce++;
      }
    }
    else
    {
      button_debounce = 0;
    }
  }
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  __disable_irq();
  led_run(true);
  while (1)
  {
  }
}
