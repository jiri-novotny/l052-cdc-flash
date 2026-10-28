#include "stm32l0xx_ll_iwdg.h"

#include "watchdog.h"

/* LSI ~37 kHz / 256 -> ~145 ticks per second */
#define IWDG_TICKS_PER_SEC 145

void watchdog_init(void)
{
  LL_IWDG_Enable(IWDG);
  watchdog_reinit(10);
}

void watchdog_reinit(uint16_t time)
{
  time = IWDG_TICKS_PER_SEC * time;
  if (time == 0)
  {
    time = 1;
  }
  else if (time > 0xfff)
  {
    time = 0xfff;
  }

  LL_IWDG_EnableWriteAccess(IWDG);
  LL_IWDG_SetPrescaler(IWDG, LL_IWDG_PRESCALER_256);
  LL_IWDG_SetReloadCounter(IWDG, time);
  while (LL_IWDG_IsReady(IWDG) != 1)
  {
  }

  LL_IWDG_ReloadCounter(IWDG);
}

void watchdog_state(void)
{
  if (LL_IWDG_GetPrescaler(IWDG) == 0)
  { // PR neni nastaven -> software independant WD neni aktivni
  }
}

void WDI(void)
{
  LL_IWDG_ReloadCounter(IWDG);
}
