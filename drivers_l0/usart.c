#include <stdbool.h>
#include <stdint.h>

#include "stm32l0xx_ll_bus.h"
#include "stm32l0xx_ll_cortex.h"
#include "stm32l0xx_ll_gpio.h"
#include "stm32l0xx_ll_rcc.h"
#include "stm32l0xx_ll_usart.h"

#include "usart.h"

/* USART2 on PA2 (TX) / PA3 (RX) - UART_TX_EXT / UART_RX_EXT to target */

/* must be power of 2 */
#define U2_BUF_TX_SIZE  256
#define U2_BUF_RX_SIZE  256

static uint8_t u2_tx_buffer[U2_BUF_TX_SIZE];
static uint8_t u2_rx_buffer[U2_BUF_RX_SIZE];

static volatile uint32_t u2_rx_wr_index = 0;
static volatile uint32_t u2_rx_rd_index = 0;

static volatile uint32_t u2_tx_wr_index = 0;
static volatile uint32_t u2_tx_rd_index = 0;

/* 0x7F for 7 data bits + parity, parity bit is received in MSB */
static volatile uint8_t u2_data_mask = 0xFF;

void u2_init(void)
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    LL_USART_InitTypeDef USART_InitStruct = {0};

    LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1);

    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_2 | LL_GPIO_PIN_3;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    USART_InitStruct.BaudRate = 115200;
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(USART2, &USART_InitStruct);
    /* do not stop RX on overrun, just drop the byte */
    LL_USART_DisableOverrunDetect(USART2);
    LL_USART_ConfigAsyncMode(USART2);
    LL_USART_Enable(USART2);
    LL_USART_EnableIT_RXNE(USART2);

    NVIC_SetPriority(USART2_IRQn, 1);
    NVIC_EnableIRQ(USART2_IRQn);
}

/* called on CDC SET_LINE_CODING, unsupported settings fall back to 8N1 */
void u2_config(uint32_t baud, uint8_t data_bits, uint8_t parity, uint8_t stop_bits)
{
    uint32_t clk = LL_RCC_GetUSARTClockFreq(LL_RCC_USART2_CLKSOURCE);
    uint32_t width;
    uint32_t par;
    uint32_t stop;

    /* BRR must be in range 16..0xFFFF with oversampling 16 */
    if (baud == 0)
    {
        baud = 115200;
    }
    else if (baud > clk / 16)
    {
        baud = clk / 16;
    }
    else if (baud < (clk / 0xFFFF) + 1)
    {
        baud = (clk / 0xFFFF) + 1;
    }

    switch (parity)
    {
        case U2_PARITY_ODD:
            par = LL_USART_PARITY_ODD;
            break;
        case U2_PARITY_EVEN:
            par = LL_USART_PARITY_EVEN;
            break;
        default:
            par = LL_USART_PARITY_NONE;
            break;
    }

    /* word length includes parity bit */
    if (data_bits == 7)
    {
        width = (par == LL_USART_PARITY_NONE) ? LL_USART_DATAWIDTH_7B : LL_USART_DATAWIDTH_8B;
        u2_data_mask = 0x7F;
    }
    else
    {
        width = (par == LL_USART_PARITY_NONE) ? LL_USART_DATAWIDTH_8B : LL_USART_DATAWIDTH_9B;
        u2_data_mask = 0xFF;
    }

    switch (stop_bits)
    {
        case U2_STOPBITS_1_5:
            stop = LL_USART_STOPBITS_1_5;
            break;
        case U2_STOPBITS_2:
            stop = LL_USART_STOPBITS_2;
            break;
        default:
            stop = LL_USART_STOPBITS_1;
            break;
    }

    LL_USART_Disable(USART2);
    LL_USART_ConfigCharacter(USART2, width, par, stop);
    LL_USART_SetBaudRate(USART2, clk, LL_USART_OVERSAMPLING_16, baud);
    LL_USART_Enable(USART2);
}

uint32_t u2_write_available(void)
{
    return U2_BUF_TX_SIZE - 1 - ((u2_tx_wr_index - u2_tx_rd_index) & (U2_BUF_TX_SIZE - 1));
}

uint32_t u2_write(const uint8_t *data, uint32_t len)
{
    uint32_t free = u2_write_available();
    uint32_t wr = u2_tx_wr_index;

    if (len > free)
    {
        len = free;
    }

    for (uint32_t i = 0; i < len; i++)
    {
        u2_tx_buffer[wr] = data[i];
        wr = (wr + 1) & (U2_BUF_TX_SIZE - 1);
    }
    u2_tx_wr_index = wr;

    if (len)
    {
        LL_USART_EnableIT_TXE(USART2);
    }

    return len;
}

uint32_t u2_read_available(void)
{
    return (u2_rx_wr_index - u2_rx_rd_index) & (U2_BUF_RX_SIZE - 1);
}

uint32_t u2_read(uint8_t *data, uint32_t len)
{
    uint32_t avail = u2_read_available();
    uint32_t rd = u2_rx_rd_index;

    if (len > avail)
    {
        len = avail;
    }

    for (uint32_t i = 0; i < len; i++)
    {
        data[i] = u2_rx_buffer[rd];
        rd = (rd + 1) & (U2_BUF_RX_SIZE - 1);
    }
    u2_rx_rd_index = rd;

    return len;
}

void USART2_IRQHandler(void)
{
    /* framing / parity / noise errors - drop flags, keep data */
    if (LL_USART_IsActiveFlag_FE(USART2))
    {
        LL_USART_ClearFlag_FE(USART2);
    }
    if (LL_USART_IsActiveFlag_PE(USART2))
    {
        LL_USART_ClearFlag_PE(USART2);
    }
    if (LL_USART_IsActiveFlag_NE(USART2))
    {
        LL_USART_ClearFlag_NE(USART2);
    }

    if (LL_USART_IsActiveFlag_RXNE(USART2))
    {
        uint8_t c = LL_USART_ReceiveData8(USART2) & u2_data_mask;
        uint32_t next = (u2_rx_wr_index + 1) & (U2_BUF_RX_SIZE - 1);

        /* drop on buffer full */
        if (next != u2_rx_rd_index)
        {
            u2_rx_buffer[u2_rx_wr_index] = c;
            u2_rx_wr_index = next;
        }
    }

    if (LL_USART_IsEnabledIT_TXE(USART2) && LL_USART_IsActiveFlag_TXE(USART2))
    {
        if (u2_tx_rd_index != u2_tx_wr_index)
        {
            LL_USART_TransmitData8(USART2, u2_tx_buffer[u2_tx_rd_index]);
            u2_tx_rd_index = (u2_tx_rd_index + 1) & (U2_BUF_TX_SIZE - 1);
        }
        else
        {
            LL_USART_DisableIT_TXE(USART2);
        }
    }
}
