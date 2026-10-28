#include <stdbool.h>
#include <stdint.h>

#include "cdc_bridge.h"
#include "gpio.h"
#include "usart.h"

#include "tusb.h"

/*
 * USB CDC <-> USART2 bridge with modem lines driving target BSL pins
 *
 *   DTR asserted -> NRST_EXT low  (target held in reset)
 *   RTS asserted -> BOOT0_EXT high (target boots to system bootloader)
 *
 * Set *_INVERT to 1 to swap polarity of the line.
 */
#define DTR_INVERT 0
#define RTS_INVERT 0

static bool line_dtr = false;
static bool line_rts = false;
static bool power_on = false;

static bool tx_activity = false;
static bool rx_activity = false;

static void apply_lines(void)
{
  if (power_on)
  {
    target_reset(line_dtr ^ DTR_INVERT);
    target_boot0(line_rts ^ RTS_INVERT);
  }
  else
  { // do not back-power unpowered target through IO pins
    target_reset(false);
    target_boot0(false);
  }
}

void cdc_bridge_init(void)
{
  line_dtr = false;
  line_rts = false;
  cdc_bridge_power(true);
}

void cdc_bridge_power(bool on)
{
  power_on = on;
  if (!on)
  {
    apply_lines();
  }
  target_power(on);
  if (on)
  {
    apply_lines();
  }
}

bool cdc_bridge_power_state(void)
{
  return power_on;
}

/* run fast from main loop */
void cdc_bridge_task(void)
{
  uint8_t buf[CFG_TUD_CDC_RX_EPSIZE];
  uint32_t n;

  /* USB -> UART, stop reading CDC when UART buffer is full (USB NAK flow control) */
  n = u2_write_available();
  if (n > sizeof(buf))
  {
    n = sizeof(buf);
  }
  if (n && tud_cdc_available())
  {
    n = tud_cdc_read(buf, n);
    u2_write(buf, n);
    tx_activity = true;
  }

  /* UART -> USB */
  if (!tud_mounted())
  { // nobody listens, drop received data
    while (u2_read(buf, sizeof(buf)))
    {
    }
    return;
  }

  n = tud_cdc_write_available();
  if (n > sizeof(buf))
  {
    n = sizeof(buf);
  }
  if (n && u2_read_available())
  {
    n = u2_read(buf, n);
    tud_cdc_write(buf, n);
    tud_cdc_write_flush();
    rx_activity = true;
  }
}

/* run every 10ms from main loop */
void cdc_bridge_tick(void)
{
  led_usb(tud_mounted());
  led_tx(tx_activity);
  led_rx(rx_activity);
  tx_activity = false;
  rx_activity = false;
}

//--------------------------------------------------------------------+
// TinyUSB callbacks
//--------------------------------------------------------------------+

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  line_dtr = false;
  line_rts = false;
  apply_lines();
}

// Invoked when usb bus is suspended
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  line_dtr = false;
  line_rts = false;
  apply_lines();
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
  (void) itf;
  line_dtr = dtr;
  line_rts = rts;
  apply_lines();
}

// Invoked when line coding is change via SET_LINE_CODING
void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const *p_line_coding)
{
  (void) itf;
  u2_config(p_line_coding->bit_rate, p_line_coding->data_bits, p_line_coding->parity, p_line_coding->stop_bits);
}
