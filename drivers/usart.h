#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/* parity */
#define U2_PARITY_NONE  0
#define U2_PARITY_ODD   1
#define U2_PARITY_EVEN  2

/* stop bits */
#define U2_STOPBITS_1   0
#define U2_STOPBITS_1_5 1
#define U2_STOPBITS_2   2

void u2_init(void);
void u2_config(uint32_t baud, uint8_t data_bits, uint8_t parity, uint8_t stop_bits);
uint32_t u2_write(const uint8_t *data, uint32_t len);
uint32_t u2_write_available(void);
uint32_t u2_read(uint8_t *data, uint32_t len);
uint32_t u2_read_available(void);

#ifdef __cplusplus
}
#endif
#endif /* __USART_H__ */
