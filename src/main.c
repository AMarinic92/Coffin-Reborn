#include "definitions.h" // Harmony generated
#include <sam.h>
#include "FreeRTOS.h"
#include "task.h"

// Your custom packages
#include "actuator.h"
#include "fog.h"
#include "sound.h"
#include <stdio.h>

// Set to 0 to fall back to the original src/neopixel.c driver. Both are
// compiled; the unused one is dropped by -Wl,--gc-sections.
#define USE_WS2812_LIB 1

// Set to 1 to start the DY sound module task and the card walk. At 0 neither
// task is created, so sound cannot take CPU time from the actuator or fog.
#define USE_SOUND 0

#if USE_WS2812_LIB
  #include "ws2812/ws2812.h"
  #include "ws2812/ws2812_fx.h"
  #include "ws2812/ws2812_freertos.h"

  #define LED_COUNT 144
  static uint8_t  strip_buf[WS2812_BUF_BYTES(LED_COUNT)];
  static ws2812_t strip;
#else
  #include "neopixel.h"
#endif

// Define an LED pin for your heartbeat (assuming PA14)
#define BLINKY_LED_PIN PORT_PA14

// ---------------------------------------------------------
// Blinky RTOS Task
// ---------------------------------------------------------
void Blinky_Task(void *pvParameters)
{
    // Setup LED pin as output
    PORT_REGS->GROUP[0].PORT_DIRSET = BLINKY_LED_PIN;

    while(1)
    {
        // Toggle the LED
        PORT_REGS->GROUP[0].PORT_OUTTGL = BLINKY_LED_PIN;
        // Sleep this task for 500ms (1Hz blink rate)
        vTaskDelay(pdMS_TO_TICKS(500)); 
    }
}

// ---------------------------------------------------------
// NeoPixel RTOS Task
// ---------------------------------------------------------
void NeoPixel_Task(void *pvParameters)
{
    uint8_t frame = 0;

    // Start from a known-blank strip
#if USE_WS2812_LIB
    ws2812_clear(&strip);
    ws2812_rtos_show(&strip, 20);
#else
    NeoPixel_Clear();
    NeoPixel_Show();
#endif

    while(1)
    {
        // Fire while idle, GreenPurple while the actuator is moving
#if USE_WS2812_LIB
        if (Actuator_IsActive())
            ws2812_fx_green_purple(&strip, frame++, 80);
        else
            ws2812_fx_fire(&strip, frame++, 80);

        // Effects only stage pixels now - the show call is ours. The RTOS path
        // blocks on a task notification instead of burning ~4.5ms spinning.
        ws2812_rtos_show(&strip, 20);
#else
        if (Actuator_IsActive())
            NeoPixel_GreenPurple(frame++, 80);
        else
            NeoPixel_Fire(frame++, 80);
#endif

        // Yield the CPU for 20ms (~50 FPS update rate)
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// ---------------------------------------------------------
// Main Entry
// ---------------------------------------------------------
int main(void)
{

    // Cache Enable (Improves performance for NeoPixel math & RTOS context switching)
    if ((CMCC_REGS->CMCC_SR & CMCC_SR_CSTS_Msk) == 0) {
        CMCC_REGS->CMCC_CTRL = CMCC_CTRL_CEN_Msk;
    }

    // 1. Initialize System and Hardware Drivers
    SYS_Initialize(NULL);
    
    // 2. Initialize Custom Peripherals
    Actuator_InitPorts();
    Fog_InitPorts();

    // Binds the DY module to its SERCOM and creates the sound task. Must run
    // before the scheduler starts. Does nothing if the driver submodule has
    // not been populated yet.
#if USE_SOUND
    Sound_Init();
#endif

#if USE_WS2812_LIB
    ws2812_cfg_t cfg = {
        .sercom   = SERCOM1_REGS,
        .dma_ch   = DMAC_CHANNEL_0,
        .buf      = strip_buf,
        .buf_len  = sizeof(strip_buf),
        .num_leds = LED_COUNT,
        .order    = WS2812_ORDER_GRB,
        // Tick rate is 1000Hz, so ticks are milliseconds. Without this the
        // show timeout is not enforced and a stalled DMA would spin forever.
        .now_ms   = xTaskGetTickCount,
    };
    if (!ws2812_init(&strip, &cfg)) {
        #ifndef NDEBUG
            printf("ws2812_init FAILED\n");
        #endif
    }
#else
    NeoPixel_Init();
#endif

#ifndef NDEBUG
    printf("~~~DEBUG ENABLED~~~\n");
#endif

    // 3. Create RTOS Tasks
    
    // Low priority system heartbeat
    xTaskCreate(
        Blinky_Task,              
        "Blinky",                 
        configMINIMAL_STACK_SIZE, 
        NULL,                     
        1,                        
        NULL                      
    );

    // Medium priority logic controller
    xTaskCreate(
        Actuator_Task,            // Implemented in actuator.c
        "Actuator",               
        512,                      // Increased stack for RNG and logic
        NULL,                     
        2,                        
        NULL                      
    );

    // Low priority fog cycle (also woken indirectly by Actuator via Fog_Trigger)
    xTaskCreate(
        Fog_Task,
        "Fog",
        256,                      // only needs the debug printf
        NULL,
        1,
        NULL
    );

    // High priority visual updates (Keeps animations smooth)
    xTaskCreate(
        NeoPixel_Task,
        "NeoPixel",
        512,                      // Increased stack for NeoPixel array buffering
        NULL,
        3,
        NULL
    );

    // Walks the pirate card one track at a time so they can be identified by
    // ear. Only prints and posts cues - the sound task created by Sound_Init()
    // is what actually talks to the module.
#if USE_SOUND
    xTaskCreate(
        SoundWalk_Task,
        "SoundWalk",
        512,                      // printf per track
        NULL,
        1,
        NULL
    );
#endif

    // 4. Hand control to the FreeRTOS Scheduler
    // Execution context shifts here. The bare-metal while(1) loop is gone.
    vTaskStartScheduler();

    // 5. Code below this line never executes unless heap memory allocation fails.
    while (1) {}
    
    return 0;
}