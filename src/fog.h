#ifndef FOG_H
#define FOG_H

// Fog machine relay on PB13 (FOG_RELAY). This relay only closes the machine's
// "on" switch - if the heater is below temperature the machine ignores it, so
// the relay can never force a burst, only permit one.

// Default cycle: fog on for FOG_ON_MS out of every FOG_PERIOD_MS.
#define FOG_ON_MS      (5UL * 1000UL)
#define FOG_PERIOD_MS  (180UL * 1000UL)

void Fog_InitPorts(void);
void Fog_Task(void *pvParameters);

// Turn the fog on now, or push back the off time of a burst already running.
// Safe to call from any task. Never shortens a burst.
void Fog_Trigger(void);

#endif /* FOG_H */
