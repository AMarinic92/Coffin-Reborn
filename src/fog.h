#ifndef FOG_H
#define FOG_H

// Fog machine relay on PB13 (FOG_RELAY). This relay only closes the machine's
// "on" switch - if the heater is below temperature the machine ignores it, so
// the relay can never force a burst, only permit one.

#include "timing.h"

// Default cycle: fog on for FOG_ON_MS out of every FOG_PERIOD_MS. The period is
// counted from the last burst, so an actuator-triggered burst restarts it.
#define FOG_ON_MS      (17UL * MS_PER_SECOND)
#define FOG_PERIOD_MS  (45UL * MS_PER_SECOND)

void Fog_InitPorts(void);
void Fog_Task(void *pvParameters);

// Turn the fog on now, or push back the off time of a burst already running,
// and push the next automatic burst a full FOG_PERIOD_MS out.
// Safe to call from any task (not from an ISR). Never shortens a burst.
void Fog_Trigger(void);

#endif /* FOG_H */
