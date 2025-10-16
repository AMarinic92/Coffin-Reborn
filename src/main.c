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

int main(void) {
    SYS_Initialize(NULL);
    
    SYSTICK_TimerStart();
    PORT_Initialize();
    
    neopixel_init();
    dsun_sensor_init();
    
    clear_all_leds();

    while (true) {
        SYS_Tasks();
        test_all_color(255,255,255);
//        test_moving_rainbow(0);
        SYSTICK_DelayMs(100);
    }
    
    return (EXIT_FAILURE);
}