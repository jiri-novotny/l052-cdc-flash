#include "stm32l0xx_hal.h"

#include "stm32l0xx_ll_bus.h"
#include "stm32l0xx_ll_crs.h"
#include "stm32l0xx_ll_pwr.h"
#include "stm32l0xx_ll_rcc.h"
#include "stm32l0xx_ll_system.h"
#include "stm32l0xx_ll_utils.h"

extern void Error_Handler(void);

/**
 * @brief System Clock Configuration
 *        No HSE on board: SYSCLK = HSI16 * 4 / 2 = 32 MHz,
 *        USB clock = HSI48 trimmed by CRS from USB SOF.
 * @retval None
 */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
  while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1)
  {
  }

  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  while (LL_PWR_IsActiveFlag_VOS() != 0)
  {
  }

  LL_RCC_HSI_Enable();

  /* Wait till HSI is ready */
  while (LL_RCC_HSI_IsReady() != 1)
  {
  }
  LL_RCC_HSI_SetCalibTrimming(16);

  /* HSI48 needs VREFINT buffer enabled */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_SYSCFG_VREFINT_EnableHSI48();
  LL_RCC_HSI48_Enable();

  /* Wait till HSI48 is ready */
  while (LL_RCC_HSI48_IsReady() != 1)
  {
  }

  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLL_MUL_4, LL_RCC_PLL_DIV_2);
  LL_RCC_PLL_Enable();

  /* Wait till PLL is ready */
  while (LL_RCC_PLL_IsReady() != 1)
  {
  }

  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

  /* Wait till System clock is ready */
  while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  }

  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_SetSystemCoreClock(32000000);

  /* Update the time base */
  if (HAL_InitTick(TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }

  /* HSI48 automatic trimming from USB SOF (1 kHz) */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_CRS);
  LL_CRS_ConfigSynchronization(LL_CRS_HSI48CALIBRATION_DEFAULT, LL_CRS_ERRORLIMIT_DEFAULT,
                               LL_CRS_RELOADVALUE_DEFAULT,
                               LL_CRS_SYNC_DIV_1 | LL_CRS_SYNC_SOURCE_USB | LL_CRS_SYNC_POLARITY_RISING);
  LL_CRS_EnableAutoTrimming();
  LL_CRS_EnableFreqErrorCounter();

  /* GPIO Ports Clock Enable */
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA | LL_IOP_GRP1_PERIPH_GPIOB | LL_IOP_GRP1_PERIPH_GPIOC);
}

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void)
{
  HAL_IncTick();
}
