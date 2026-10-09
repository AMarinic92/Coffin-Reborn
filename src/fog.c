#include "fog.h"
#include "definitions.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>
#include <stdio.h>

#define FOG_POLL_MS 50UL

// The relay is on while (int32_t)(now - fog_off_at) < 0, and the next automatic
// burst fires once now reaches fog_next_auto. After start-up only Fog_Trigger()
// writes them, inside a critical section so the actuator task and the auto
// cycle cannot interleave; Fog_Task only reads them, and an aligned 32-bit load
// is atomic on Cortex-M.
// ponytail: two deadline words instead of task notifications / timers.
static volatile TickType_t fog_off_at;
static volatile TickType_t fog_next_auto;

// Active low: IN sinks the optocoupler LED, so a low pin closes NO-COM.
// Driving high (LED dark) is the off state. Do not "simplify" this.
static void fog_relay(bool on)
{
    if (on) { FOG_RELAY_Clear(); } else { FOG_RELAY_Set(); }
}

void Fog_InitPorts(void)
{
    FOG_RELAY_Set();            // preset the latch high (off) before driving
    FOG_RELAY_OutputEnable();
}

void Fog_Trigger(void)
{
    taskENTER_CRITICAL();

    TickType_t now   = xTaskGetTickCount();
    TickType_t until = now + pdMS_TO_TICKS(FOG_ON_MS);

    // Extend only - a longer burst already in flight wins.
    if ((int32_t)(until - fog_off_at) > 0) {
        fog_off_at = until;
    }

    // Every burst, automatic or actuator-driven, restarts the cycle, so the
    // machine always gets a full FOG_PERIOD_MS before the next automatic one.
    fog_next_auto = now + pdMS_TO_TICKS(FOG_PERIOD_MS);

    taskEXIT_CRITICAL();
}

void Fog_Task(void *pvParameters)
{
    bool relay_on = false;

    fog_next_auto = xTaskGetTickCount() + pdMS_TO_TICKS(FOG_PERIOD_MS);

    while (1)
    {
        TickType_t now = xTaskGetTickCount();

        if ((int32_t)(now - fog_next_auto) >= 0) {
            Fog_Trigger();   // also schedules the next automatic burst
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
