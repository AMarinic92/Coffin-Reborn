#include "definitions.h" // Harmony generated
#include <sam.h>
#include "FreeRTOS.h"
#include "task.h"

// Your custom packages
#include "actuator.h"
#include "neopixel.h" // Ensure your NeoPixel header is included
#include <stdio.h>
#define DEBUG_WAIT 10000000UL

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
    
    while(1)
    {
    // Execute the NeoPixel animation
    NeoPixel_GreenPurple(frame++, 80);
      
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
    NeoPixel_Init();

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
    // Currently appears bugged as this will break blinky 
 
    xTaskCreate(
        Actuator_Task,            // Implemented in actuator.c
        "Actuator",               
        512,                      // Increased stack for RNG and logic
        NULL,                     
        2,                        
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

    // 4. Hand control to the FreeRTOS Scheduler
    // Execution context shifts here. The bare-metal while(1) loop is gone.
    vTaskStartScheduler();

    // 5. Code below this line never executes unless heap memory allocation fails.
    while (1) {}
    
    return 0;
}