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
void test_tcc_output(void) {
    // Manually set TCC compare value and start it
    TCC0_REGS->TCC_CC[0] = 75;  // 50% duty cycle (half of 150)
    TCC0_PWMStart();
    
    // Let it run for a few seconds so you can measure with scope/meter
    SYSTICK_DelayMs(5000);
    
    TCC0_PWMStop();
}

void test_tcc_running(void) {
    neopixel_init();
    
    // Set a fixed duty cycle
    TCC0_PWM24bitCounterSet(75);  // 50% duty cycle
    TCC0_PWMStart();
    
    // Blink status LED to show we're running
    for(int i = 0; i < 20; i++) {
        PORT_PinToggle(PORT_PIN_PA14);  // Your status LED
        SYSTICK_DelayMs(250);
    }
    
    TCC0_PWMStop();
}

void test_pa08_blink(void) {
    // Configure PA08 as regular GPIO output (override TCC)
    PORT_PinOutputEnable(PORT_PIN_PA08);
    
    for(int i = 0; i < 250; i++) {
        PORT_PinSet(PORT_PIN_PA08);
        SYSTICK_DelayMs(500);
        PORT_PinClear(PORT_PIN_PA08);
        SYSTICK_DelayMs(500);
    }
}

void test_dma_completion(void) {
    
    // Fill buffer with test data
    for(int i = 0; i < 100; i++) {
        neopixel_tcc_buffer[i] = 50;
    }
    
    // Try DMA transfer
    neopixel_dma_complete = false;
    
    DMAC_ChannelTransfer(
        DMAC_CHANNEL_0,
        neopixel_tcc_buffer,
        (const void*)&TCC0_REGS->TCC_CC[0],
        100
    );
    
    TCC0_PWMStart();
    
    // Wait with timeout
    uint32_t timeout = 0;
    while(!neopixel_dma_complete && timeout < 1000000) {
        timeout++;
    }
    
    TCC0_PWMStop();
    // Blink result
    if(neopixel_dma_complete) {
        // SUCCESS - fast blink
        for(int i = 0; i < 10; i++) {
            PORT_PinToggle(PORT_PIN_PA14);
            SYSTICK_DelayMs(100);
        }
    } else {
        // FAILED - slow blink
        for(int i = 0; i < 10; i++) {
            PORT_PinToggle(PORT_PIN_PA14);
            SYSTICK_DelayMs(500);
        }
       
    }
    
    
}

void test_tcc_detailed(void) {
    
    // Read TCC period
    uint32_t period = TCC0_REGS->TCC_PER;  // Should be 149
    
    // Set compare to 75 (50% of 150)
    TCC0_PWM24bitCounterSet(75);  // 50% duty cycle
    
    // Start TCC
    TCC0_PWMStart();
    
    // Wait and check if TCC is counting
    SYSTICK_DelayMs(100);
    volatile uint32_t count1 = TCC0_PWM24bitCounterGet();
    
    SYSTICK_DelayMs(101);
    volatile uint32_t count2 = TCC0_PWM24bitCounterGet();
    
    TCC0_PWMStop();
    
    // Blink to indicate results
    if(period == 149 && count2 != count1) {
        // SUCCESS - TCC is running!
        // Fast blink
        for(int i = 0; i < 20; i++) {
            PORT_PinToggle(PORT_PIN_PA14);
            SYSTICK_DelayMs(100);
        }
    } else {
        // FAIL - TCC not counting
        // Slow blink
        for(int i = 0; i < 10; i++) {
            PORT_PinToggle(PORT_PIN_PA14);
            SYSTICK_DelayMs(1000);
        }
    }
}

void test_neopixel_pattern(void) {
    
    while(1) {
        // All red
        for(int i = 0; i < 144; i++) {
            set_led_color_tcc(i, 255, 0, 0);
        }
        neopixel_send_tcc();
        SYSTICK_DelayMs(1000);
        
        // All off
        for(int i = 0; i < 144; i++) {
            set_led_color_tcc(i, 0, 0, 0);
        }
        neopixel_send_tcc();
        SYSTICK_DelayMs(1000);
        
        // Blink status LED to show we're looping
        PORT_PinToggle(PORT_PIN_PA14);
    }
}
void test_neopixel_single_led(void) {
    // Initialize
    
    // Set only first LED to red
    set_led_color_tcc(0, 255, 0, 0);
    for(int i = 1; i < 144; i++) {
        set_led_color_tcc(i, 0, 0, 0);
    }
    
    // Send
    neopixel_send_tcc();
    
    // Indicate completion
    for(int i = 0; i < 5; i++) {
        PORT_PinToggle(PORT_PIN_PA14);
        SYSTICK_DelayMs(200);
    }
}

void test_dma_updating(void) {
    neopixel_init();
    
    // Fill buffer with alternating values
    for(int i = 0; i < 100; i++) {
        neopixel_tcc_buffer[i] = (i % 2) ? 100 : 50;  // Should alternate between 33% and 66%
    }
    
    neopixel_dma_complete = false;
    
    DMAC_ChannelTransfer(
        DMAC_CHANNEL_0,
        neopixel_tcc_buffer,
        (const void*)&TCC0_REGS->TCC_CCBUF[0],
        100
    );
    
    TCC0_PWMStart();
    
    // Let it run
    SYSTICK_DelayMs(200);  // Should see alternating pulses
    
    TCC0_PWMStop();
}

void test_dma_event_basic(void) {
    neopixel_init();
    
    // Simple counter pattern
    for(int i = 0; i < 50; i++) {
        neopixel_tcc_buffer[i] = i + 20;  // 20, 21, 22, 23... 69
    }
    
    // Read initial DMA transfer count
//    uint16_t initial_count = DMAC_ChannelGetTransferredCount(DMAC_CHANNEL_0);
    
    // Setup DMA
    neopixel_dma_complete = false;
    DMAC_ChannelTransfer(
        DMAC_CHANNEL_0,
        neopixel_tcc_buffer,
        (const void*)&TCC0_REGS->TCC_CCBUF[0],
        50
    );
    
    // Start TCC
    TCC0_PWMStart();
    
    // Wait a bit (50 TCC periods = 50 * 1.25µs = 62.5µs)
    SYSTICK_DelayUs(100);
    
    // Check how many bytes were transferred
    uint16_t transferred = DMAC_ChannelGetTransferredCount(DMAC_CHANNEL_0);
    
    TCC0_PWMStop();
    
    // Read final CC value
//    volatile uint32_t final_cc = TCC0_REGS->TCC_CC[0];
    
    // Blink result
    // If transferred = 50, DMA completed all transfers
    // If transferred = 0, DMA never triggered
    if(transferred == 50) {
        // SUCCESS - fast blink
        for(int i = 0; i < 20; i++) {
            PORT_PinToggle(PORT_PIN_PA14);
            SYSTICK_DelayMs(100);
        }
    } else if(transferred > 0 && transferred < 50) {
        // PARTIAL - medium blink (some transfers happened)
        for(int i = 0; i < 10; i++) {
            PORT_PinToggle(PORT_PIN_PA14);
            SYSTICK_DelayMs(250);
        }
    } else {
        // FAILED - slow blink (no transfers)
        for(int i = 0; i < 5; i++) {
            PORT_PinToggle(PORT_PIN_PA14);
            SYSTICK_DelayMs(1000);
        }
    }
}





void verify_dmac_trigact(void) {
    uint32_t chctrla = DMAC_REGS->CHANNEL[0].DMAC_CHCTRLA;
    uint32_t trigact = (chctrla >> 20) & 0x3;  // Extract TRIGACT bits [21:20]
    
    // Blink the value
    for(int i = 0; i < trigact + 1; i++) {
        PORT_PinToggle(PORT_PIN_PA14);
        SYSTICK_DelayMs(500);
        PORT_PinToggle(PORT_PIN_PA14);
        SYSTICK_DelayMs(500);
    }
}

int main(void) {
    SYS_Initialize(NULL);
    SYSTICK_TimerStart();
    neopixel_init();
//    test_neopixel_single_led();
//    test_dma_updating();
//    test_dma_event_basic();
//    test_pa08_blink();
//    test_tcc_running();
//    test_tcc_detailed();
//    test_dma_completion();
//    dump_dma_tcc_config();
    test_neopixel_pattern();
//    verify_dmac_trigact();
    
    while(1) {
        SYS_Tasks();
    }
}