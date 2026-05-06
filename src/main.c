/*******************************************************************************
  Main Source File

  @Company
    Microchip Technology Inc.

  @File Name
    main.c

  @Summary
    This file contains the "main" function for a project.

  @Description
    This file contains the "main" function for a project. The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "definitions.h"
#include "neopixel.h"
#include "dsun_sensor.h"
#include "FreeRTOS.h"
#include "task.

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

uint32_t millis(void) {
    return SYSTICK_GetTickCounter();
}

uint32_t seconds(void) {
    return (millis()) / (1000);
}


// 1. Move your bare-metal while(1) loop into a dedicated Task function
void NeoPixel_Task(void *pvParameters)
{
    uint8_t frame = 0;
    
    while(1)
    {
        // NeoPixel_Rainbow(frame++, 80);
        // NeoPixel_Fire(frame++, 80);
        NeoPixel_GreenPurple(frame++, 80);
        
        // 2. Replace the blocking SysTick delay with an RTOS thread yield
        // This frees the CPU to run other animatronic tasks for 20ms
        vTaskDelay(pdMS_TO_TICKS(20)); 
    }
}

int main(void)
{
    // Initialize Harmony hardware drivers
    SYS_Initialize(NULL);
    NeoPixel_Init();

    // 3. Register the task with the OS before starting the scheduler
    xTaskCreate(
        NeoPixel_Task,       // Function pointer to your task
        "NeoPixel",          // Debug name
        512,                 // Stack size in words (adjust if NeoPixel library uses heavy local vars)
        NULL,                // Task parameters
        1,                   // Priority (1 is standard low priority)
        NULL                 // Task handle
    );

    // 4. Hand control to FreeRTOS. 
    vTaskStartScheduler();

    // 5. Code below this line never executes unless heap_4 runs out of RAM during init.
    while(1)
    {
    }
    
    return 0;
}