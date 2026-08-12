#include "fog.h"
#include "definitions.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>
#include <stdio.h>

#define FOG_POLL_MS 50UL

// The relay is on while (int32_t)(now - fog_off_at) < 0. Only Fog_Trigger()
// writes it, and a 32-bit aligned store is atomic on Cortex-M, so no mutex or
// queue is needed to poke the fog from another task.
// ponytail: single deadline word instead of task notifications / timers.
static volatile TickType_t fog_off_at;

// One place to flip if the relay board turns out to be active-low.
static void fog_relay(bool on)
{
    if (on) { FOG_RELAY_Set(); } else { FOG_RELAY_Clear(); }
}

void Fog_InitPorts(void)
{
    FOG_RELAY_Clear();          // start off, then drive the pin
    FOG_RELAY_OutputEnable();   // Harmony leaves PB13 as an input
}

void Fog_Trigger(void)
{
    TickType_t until = xTaskGetTickCount() + pdMS_TO_TICKS(FOG_ON_MS);

    // Extend only - a longer burst already in flight wins.
    if ((int32_t)(until - fog_off_at) > 0) {
        fog_off_at = until;
    }
}

void Fog_Task(void *pvParameters)
{
    TickType_t next_auto = xTaskGetTickCount() + pdMS_TO_TICKS(FOG_PERIOD_MS);
    bool relay_on = false;

    while (1)
    {
        TickType_t now = xTaskGetTickCount();

        if ((int32_t)(now - next_auto) >= 0) {
            next_auto = now + pdMS_TO_TICKS(FOG_PERIOD_MS);
            Fog_Trigger();
        }

        bool want_on = ((int32_t)(now - fog_off_at) < 0);
        if (want_on != relay_on) {
            relay_on = want_on;
            fog_relay(relay_on);
            #ifndef NDEBUG
                printf("Fog %s\n", relay_on ? "ON" : "off");
            #endif
        }

        vTaskDelay(pdMS_TO_TICKS(FOG_POLL_MS));
    }
}
