/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_ll_bus.h"
#include "stm32l0xx_ll_gpio.h"

#include "gpio.h"

/*
 * Pinout - LE BSL Fixture Main Board rev. A1 (STM32L052K8U6)
 *
 * PA2  UART_TX_EXT  USART2_TX -> target RX (CON1.6)   see usart.c
 * PA3  UART_RX_EXT  USART2_RX <- target TX (CON1.5)   see usart.c
 * PA4  NRST_EXT     target reset (CON1.2), open-drain, active low
 * PA5  BOOT0_EXT    target BOOT0 (CON1.1), push-pull, active high
 * PA7  LED4         red, active low
 * PA8  VOUT_EXT_EN  target 3V0_EXT regulator enable (IC2), active high
 * PA11 USB_DN
 * PA12 USB_DP
 * PA13 SWDIO
 * PA14 SWCLK
 * PB0  LED1         green, active low
 * PB1  LED2         green, active low
 * PB2  LED3         green, active low
 * PB8  USER         button SW3 to GND, active low
 */
#define LED_RUN_PORT  GPIOA
#define LED_RUN_PIN   LL_GPIO_PIN_7
#define LED_USB_PORT  GPIOB
#define LED_USB_PIN   LL_GPIO_PIN_0
#define LED_TX_PORT   GPIOB
#define LED_TX_PIN    LL_GPIO_PIN_1
#define LED_RX_PORT   GPIOB
#define LED_RX_PIN    LL_GPIO_PIN_2

#define BUTTON_PORT   GPIOB
#define BUTTON_PIN    LL_GPIO_PIN_8

#define NRST_PORT     GPIOA
#define NRST_PIN      LL_GPIO_PIN_4
#define BOOT0_PORT    GPIOA
#define BOOT0_PIN     LL_GPIO_PIN_5
#define VOUT_EN_PORT  GPIOA
#define VOUT_EN_PIN   LL_GPIO_PIN_8

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
void gpio_init(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Outputs - Default value: LEDs off, target powered off, reset released, BOOT0 low */
  LL_GPIO_SetOutputPin(GPIOA, LED_RUN_PIN | NRST_PIN);
  LL_GPIO_ResetOutputPin(GPIOA, BOOT0_PIN | VOUT_EN_PIN);
  LL_GPIO_SetOutputPin(GPIOB, LED_USB_PIN | LED_TX_PIN | LED_RX_PIN);

  /* Outputs - Push-Pull */
  GPIO_InitStruct.Pin = LED_RUN_PIN | BOOT0_PIN | VOUT_EN_PIN;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LED_USB_PIN | LED_TX_PIN | LED_RX_PIN;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Outputs - Open-Drain, target has its own NRST pull-up */
  GPIO_InitStruct.Pin = NRST_PIN;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Inputs */
  LL_GPIO_SetPinPull(BUTTON_PORT, BUTTON_PIN, LL_GPIO_PULL_UP);
  LL_GPIO_SetPinMode(BUTTON_PORT, BUTTON_PIN, LL_GPIO_MODE_INPUT);
}

/* inputs */
bool button(void)
{
  return (LL_GPIO_IsInputPinSet(BUTTON_PORT, BUTTON_PIN) == 0);
}

/* outputs - LEDs, all active low */
static void led_set(GPIO_TypeDef *port, uint32_t pin, bool on)
{
  if (on)
  {
    LL_GPIO_ResetOutputPin(port, pin);
  }
  else
  {
    LL_GPIO_SetOutputPin(port, pin);
  }
}

void led_run(bool on)
{
  led_set(LED_RUN_PORT, LED_RUN_PIN, on);
}

void led_toggle(void)
{
  LL_GPIO_TogglePin(LED_RUN_PORT, LED_RUN_PIN);
}

void led_usb(bool on)
{
  led_set(LED_USB_PORT, LED_USB_PIN, on);
}

void led_tx(bool on)
{
  led_set(LED_TX_PORT, LED_TX_PIN, on);
}

void led_rx(bool on)
{
  led_set(LED_RX_PORT, LED_RX_PIN, on);
}

/* outputs - target control */
void target_power(bool on)
{
  if (on)
  {
    LL_GPIO_SetOutputPin(VOUT_EN_PORT, VOUT_EN_PIN);
  }
  else
  {
    LL_GPIO_ResetOutputPin(VOUT_EN_PORT, VOUT_EN_PIN);
  }
}

void target_reset(bool assert)
{
  if (assert)
  {
    LL_GPIO_ResetOutputPin(NRST_PORT, NRST_PIN);
  }
  else
  {
    LL_GPIO_SetOutputPin(NRST_PORT, NRST_PIN);
  }
}

void target_boot0(bool high)
{
  if (high)
  {
    LL_GPIO_SetOutputPin(BOOT0_PORT, BOOT0_PIN);
  }
  else
  {
    LL_GPIO_ResetOutputPin(BOOT0_PORT, BOOT0_PIN);
  }
}
