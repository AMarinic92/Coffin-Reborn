/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <stdio.h>
#include "definitions.h"                // SYS function prototypes
#include "neopixel.h"                   // NeoPixel library
#include "dsun_sensor.h"                //D-SUN proximity sensor Lib

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
// Global time tracking variables


// Get milliseconds (if tracking full milliseconds)
uint32_t millis(void)
{
    return SYSTICK_GetTickCounter();
}

// Get seconds
uint32_t seconds(void)
{
    return (millis())/(1000);
}
int main(void) {
    /* Initialize all modules */
    SYS_Initialize(NULL);
    
    
    // Start the SysTick timer
    SYSTICK_TimerStart();
    PORT_Initialize();
    /* Initialize NeoPixel library */
    neopixel_init();
    dsun_sensor_init();
    /* Initialize buffer to all zeros (LEDs off) */
    clear_all_leds();
    uint32_t sec = 0, lastSec = 0, mili = 0; 
    uint32_t GRADIENT_UPDATE = 500;
    uint32_t gradient_duration = 10000;
    uint32_t gradient_step = 0;
    uint32_t update_gradient = 0;
    while (true) {

        clear_all_leds();
        sec = seconds();
        mili = millis();
        update_gradient = mili/GRADIENT_UPDATE;
        if(!(update_gradient)){
            gradient_green_white_purple(gradient_duration, gradient_step);
            gradient_step++;
        }
        if(sec > lastSec){
            lastSec = sec;
            PORT_PinToggle(PORT_PIN_PA14);
        }
//        if (dsun_object_detected()) {
//            if (timer % 75000 == 0) {  // Faster update when object present
//                test_green_to_purple(counter);
//                counter++;
//            }
//        } else {
//            if (timer % 75000 == 0) {
//                test_green_to_purple(counter);
//                counter++;
//            }
//        }
        SYS_Tasks();

    }
    
    /* Execution should not come here during normal operation */
    return (EXIT_FAILURE);
}