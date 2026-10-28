#ifndef __CDC_BRIDGE_H__
#define __CDC_BRIDGE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

void cdc_bridge_init(void);
void cdc_bridge_task(void);
void cdc_bridge_tick(void);
void cdc_bridge_power(bool on);
bool cdc_bridge_power_state(void);

#ifdef __cplusplus
}
#endif
#endif /* __CDC_BRIDGE_H__ */
