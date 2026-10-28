#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

void gpio_init(void);

/* inputs */
bool button(void);

/* outputs - LEDs */
void led_run(bool on);
void led_toggle(void);
void led_usb(bool on);
void led_tx(bool on);
void led_rx(bool on);

/* outputs - target control */
void target_power(bool on);
void target_reset(bool assert);
void target_boot0(bool high);

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */
