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

int main(void) {
    /* Initialize all modules */
    SYS_Initialize(NULL);
    
    /* Initialize NeoPixel library */
    neopixel_init();
    dsun_sensor_init();
    /* Initialize buffer to all zeros (LEDs off) */
    clear_all_leds();
    
    /* Application variables */
//    uint32_t counter = 0;
//    uint32_t timer = 0;
    
//     uint8_t offset = 0;
         while(1) {
             test_sequence();
//           test_red_orange_gradient_shift(offset);
//           offset++;
//           for(volatile int d = 0; d < 300000; d++); // Control speed
//         // Sequential fill
//         test_sequence_random();
//         for(volatile int i = 0; i < 5000000; i++);
//         
//         // Random fill
//         test_random_fill();
//         for(volatile int i = 0; i < 5000000; i++);
//         
         // Sparkle effect
//         for(int i = 0; i < 100; i++) {
//             test_random_sparkle(5, 30);
//         }
         
//         // Wave effect
//         for(int i = 0; i < 200; i++) {
//             test_random_wave(i);
//             for(volatile int d = 0; d < 500000; d++);
//         }
     }
    
//    while (true) {
//        
//        if (dsun_object_detected()) {
//            if (timer % 75000 == 0) {  // Faster update when object present
//                test_green_to_purple(counter);
//                counter++;
//            }
//        } else {
//            if (timer % 75000 == 0) {
////                test_sequence();
//                test_random_fill();
//                counter++;
//            }
//        }
//        
//        // Or use edge detection for one-shot triggers:
//
//
//        
//        /* Increment timer with overflow protection */
//        if (timer < (UINT32_MAX - 5)) {
//            timer++;
//        } else {
//            timer = 0;
//        }
//        
//        /* Maintain state machines of all polled MPLAB Harmony modules */
//        SYS_Tasks();
//    }
    
    /* Execution should not come here during normal operation */
    return (EXIT_FAILURE);
}