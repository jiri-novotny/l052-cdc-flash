# l052-cdc-flash
USB CDC to UART bridge for LE BSL Fixture Main Board (STM32L052K8U6) with TinyUSB.
DTR / RTS modem lines drive target NRST / BOOT0, so the target can be flashed
over its system bootloader e.g. with `stm32flash`.

## Pinout
Schematic: `Fixtura_MainBoard_SchematicVariants.PDF` (rev. A1)

| Pin  | Net          | Function                                       |
|------|--------------|------------------------------------------------|
| PA2  | UART_TX_EXT  | USART2 TX -> target RX (CON1.6)                |
| PA3  | UART_RX_EXT  | USART2 RX <- target TX (CON1.5)                |
| PA4  | NRST_EXT     | target reset (CON1.2), open-drain, CDC **DTR** |
| PA5  | BOOT0_EXT    | target BOOT0 (CON1.1), push-pull, CDC **RTS**  |
| PA7  | LED4 (red)   | heartbeat                                      |
| PA8  | VOUT_EXT_EN  | target 3V0_EXT supply (CON1.3) enable          |
| PA11 | USB_DN       | USB                                            |
| PA12 | USB_DP       | USB                                            |
| PA13 | SWDIO        | SWD (TP1)                                      |
| PA14 | SWCLK        | SWD (TP1)                                      |
| PB0  | LED1 (green) | USB mounted                                    |
| PB1  | LED2 (green) | USB -> UART activity                           |
| PB2  | LED3 (green) | UART -> USB activity                           |
| PB8  | USER (SW3)   | toggle target power                            |

No HSE crystal: SYSCLK 32 MHz from HSI16 + PLL, USB from HSI48 trimmed by CRS.

## Modem lines
| CDC line     | Target pin         |
|--------------|--------------------|
| DTR asserted | NRST_EXT low       |
| RTS asserted | BOOT0_EXT high     |

Polarity can be inverted in `app/cdc_bridge.c` (`DTR_INVERT`, `RTS_INVERT`).
Lines are released on USB unmount / suspend and while target power is off.
Line coding (baud rate, 7/8 data bits, none/odd/even parity, 1/1.5/2 stop bits)
is applied to USART2, so 8E1 for the STM32 bootloader works.

Note: Linux asserts DTR and RTS when the port is opened, which holds the target
in reset. For a plain terminal use e.g. `picocom --lower-dtr --lower-rts`.

## Flash target
```
stm32flash -b 115200 -i 'rts&dtr,-dtr,-rts:-rts&dtr,-dtr' -w target.bin -v /dev/ttyACM0
```

## Build
```
cmake -Bbuild/stm32l0 -GNinja -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_TOOLCHAIN_FILE=$STM32CLT_PATH/gcc-arm-none-eabi.cmake -Dstm32l0=1 .
ninja -Cbuild/stm32l0
STM32_Programmer_CLI -c port=SWD -v -d build/stm32l0/l052-cdc-flash.hex -rst
```
