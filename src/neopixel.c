/* ************************************************************************** */
/** NeoPixel Control Library

  @Company
    Microchip Technology Inc.

  @File Name
    neopixel.c

  @Summary
    NeoPixel/WS2812/SK6812 LED strip control implementation.

  @Description
    Implementation of NeoPixel control functions using SPI interface with
    DMA support for efficient LED strip control on SAMD21 microcontrollers.
 */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include "neopixel.h"
#include "definitions.h"
#include <stdio.h>

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

/* ************************************************************************** */
/** NeoPixel SPI Data Buffer

  @Summary
    Global buffer containing SPI-encoded color data for 8 LEDs.
    
  @Description
    This buffer holds 192 bytes of SPI-encoded data (24 bytes per LED).
    Each color bit is expanded to one SPI byte using ZERO_LED/ONE_LED patterns.
    Buffer is filled by set_led_color() and transmitted via SPI to LED strip.
    
  @Remarks
    Buffer layout: [LED0_G[8], LED0_R[8], LED0_B[8], LED1_G[8], LED1_R[8], ...]
    where each [8] represents 8 SPI bytes encoding the 8-bit color value.
 */
uint8_t neopixel_buffer[NEOPIXEL_BUFFER_SIZE];

/* ************************************************************************** */
/** DMA Transfer Complete Flag

  @Summary
    Volatile flag indicating DMA transfer completion status.
    
  @Description
    Set to false before starting DMA transfer, set to true by DMA callback
    when transfer completes. Used for synchronization in neopixel_send_data().
    
  @Remarks
    Volatile qualifier ensures proper behavior across interrupt boundaries.
 */
volatile bool dma_complete = false;

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

/** 
  @Function
    void DMA_0_Callback(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle)

  @Summary
    DMA transfer completion callback function.

  @Description
    Called by the DMA system when transfer events occur. Sets the dma_complete
    flag when transfer finishes successfully.

  @Precondition
    DMA system must be initialized and callback registered.

  @Parameters
    @param event DMA transfer event type
    @param contextHandle User-defined context (unused)

  @Returns
    None.

  @Remarks
    This callback is registered during neopixel_init().
 */
void DMA_0_Callback(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle) {
    if(event == DMAC_TRANSFER_EVENT_COMPLETE) {
        dma_complete = true;
    }
}

/** 
  @Function
    void delay_microseconds(uint32_t us)

  @Summary
    Generate a microsecond delay using timer counter.

  @Description
    Uses TC0 timer to generate precise microsecond delays. Required for
    NeoPixel reset pulse timing (>80?s low pulse between transmissions).

  @Parameters
    @param us Number of microseconds to delay

  @Returns
    None.

  @Remarks
    Assumes TC0 is configured for microsecond counting.
 */
static void delay_microseconds(uint32_t us) {
    TC0_TimerStart();
    while(TC0_Timer32bitCounterGet() < us) {
        // Wait
    }
    TC0_TimerStop();
}

/** 
  @Function
    void debug_buffer(void)

  @Summary
    Debug function to print buffer contents via printf.

  @Description
    Prints the first 48 bytes of the neopixel buffer (first two LEDs) in 
    hexadecimal format for debugging SPI encoding issues.

  @Remarks
    Used for development and debugging. May be removed in production code.
 */
#ifdef DEBUG_NEOPIXEL
static void debug_buffer(void) {
    for(int i = 0; i < 48; i++) { // Check first two LEDs (48 bytes)
        printf("Byte %d: 0x%02X\n", i, neopixel_buffer[i]);
    }
}
#endif

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

// *****************************************************************************
/** 
  @Function
    void neopixel_init(void)

  @Summary
    Initialize the NeoPixel library and DMA system.

  @Remarks
    Refer to the neopixel.h interface header for function usage details.
 */
void neopixel_init(void) {
    // Register DMA callback
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, DMA_0_Callback, 0);
}

// *****************************************************************************
/** 
  @Function
    void set_led_color(uint8_t led_index, uint8_t red, uint8_t green, uint8_t blue)

  @Summary
    Set the color of a specific LED in the buffer.

  @Remarks
    Refer to the neopixel.h interface header for function usage details.
 */
void set_led_color(uint8_t led_index, uint8_t red, uint8_t green, uint8_t blue) {
    if(led_index >= NUM_LEDS) return; // Safety check
    
    uint8_t *led_buffer = &neopixel_buffer[led_index * 24]; // 24 SPI bytes per LED
    
    // At 8 MHz: Each SPI bit = 0.125탎
    // '0' bit: 0xE0 (11100000) = ~0.375탎 HIGH, ~0.625탎 LOW
    // '1' bit: 0xF8 (11111000) = ~0.625탎 HIGH, ~0.375탎 LOW
    
    // SK6812 order is G-R-B, each color needs 8 SPI bytes
    // Green first (8 bytes)
    for(int bit = 7; bit >= 0; bit--) {
        led_buffer[7-bit] = (green & (1 << bit)) ? ONE_LED : ZERO_LED;
    }
    // Red next (8 bytes)
    for(int bit = 7; bit >= 0; bit--) {
        led_buffer[8 + (7-bit)] = (red & (1 << bit)) ? ONE_LED : ZERO_LED;
    }
    // Blue last (8 bytes)  
    for(int bit = 7; bit >= 0; bit--) {
        led_buffer[16 + (7-bit)] = (blue & (1 << bit)) ? ONE_LED : ZERO_LED;
    }
}

// *****************************************************************************
/** 
  @Function
    void clear_all_leds(void)

  @Summary
    Turn off all LEDs by setting their colors to black.

  @Remarks
    Refer to the neopixel.h interface header for function usage details.
 */
void clear_all_leds(void) {
    for(int i = 0; i < NUM_LEDS; i++) {
        set_led_color(i, 0, 0, 0);
    }
}

// *****************************************************************************
/** 
  @Function
    void neopixel_send_data(uint8_t *buffer, uint16_t length)

  @Summary
    Send LED color data to the strip using DMA transfer.

  @Remarks
    Refer to the neopixel.h interface header for function usage details.
 */
void neopixel_send_data(uint8_t *buffer, uint16_t length) {
    dma_complete = false;
    
    // Configure and start DMA transfer
    DMAC_ChannelTransfer(DMAC_CHANNEL_0, 
                        buffer,                    // Source
                        (const void*)&SERCOM1_REGS->SPIM.SERCOM_DATA,  // Destination
                        length);                   // Length
    
    // Wait for transfer complete
    while(!dma_complete);
    
    // Send reset pulse (keep line low for >80탎)
    delay_microseconds(100);
}

// *****************************************************************************
/** 
  @Function
    void neopixel_send_data_manual(uint8_t *buffer, uint16_t length)

  @Summary
    Send LED color data using manual SPI transmission (blocking).

  @Remarks
    Refer to the neopixel.h interface header for function usage details.
 */
void neopixel_send_data_manual(uint8_t *buffer, uint16_t length) {
    for(uint16_t i = 0; i < length; i++) {
        while((SERCOM1_REGS->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_DRE_Msk) == 0U); // Wait for DRE (Data Register Empty)
        SERCOM1_REGS->SPIM.SERCOM_DATA = buffer[i]; // Write byte
    }
    while((SERCOM1_REGS->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_TXC_Msk) == 0U); // Wait for TXC (Transmission Complete) for the last byte
    delay_microseconds(100);
}

// *****************************************************************************
/** 
  @Function
    void test_single_red(void)

  @Summary
    Test function to light up the first LED in red.

  @Remarks
    Refer to the neopixel.h interface header for function usage details.
 */
void test_single_red(void) {
    clear_all_leds(); // Clear neopixel_buffer
    delay_microseconds(100); // Reset pulse
    set_led_color(0, 255, 0, 0); // Set first LED (index 0) to red
    neopixel_send_data_manual(neopixel_buffer, 24); // Send 24 bytes (first LED only)
    delay_microseconds(100); // Ensure reset pulse
}

// *****************************************************************************
/** 
  @Function
    void test_single_green(void)

  @Summary
    Test function to light up the first LED in green.

  @Remarks
    Refer to the neopixel.h interface header for function usage details.
 */
void test_single_green(void) {
    clear_all_leds();
    set_led_color(0, 111, 255, 0); // First LED green
    neopixel_send_data(neopixel_buffer, sizeof(neopixel_buffer));
}

// *****************************************************************************
/** 
  @Function
    void test_single_blue(void)

  @Summary
    Test function to light up the first LED in blue.

  @Remarks
    Refer to the neopixel.h interface header for function usage details.
 */
void test_single_blue(void) {
    clear_all_leds();
    set_led_color(0, 0, 0, 255); // First LED blue
    neopixel_send_data(neopixel_buffer, sizeof(neopixel_buffer));
}

// *****************************************************************************
/** 
  @Function
    void test_all_red(void)

  @Summary
    Set all LEDs to red color.

  @Remarks
    Refer to the neopixel.h interface header for function usage details.
 */
void test_all_red(void) {
    for(int i = 0; i < NUM_LEDS; i++) {
        set_led_color(i, 255, 0, 0); // All LEDs red
    }
    neopixel_send_data_manual(neopixel_buffer, sizeof(neopixel_buffer));
}

// *****************************************************************************
/** 
  @Function
    void test_rainbow(void)

  @Summary
    Display a static rainbow pattern across all LEDs.

  @Remarks
    Refer to the neopixel.h interface header for function usage details.
 */
void test_rainbow(void) {
    // Rainbow pattern across 8 LEDs
    set_led_color(0, 255, 0, 0);   // Red
    set_led_color(1, 255, 127, 0); // Orange
    set_led_color(2, 255, 255, 0); // Yellow
    set_led_color(3, 0, 255, 0);   // Green
    set_led_color(4, 0, 255, 255); // Cyan
    set_led_color(5, 0, 0, 255);   // Blue
    set_led_color(6, 127, 0, 255); // Violet
    set_led_color(7, 255, 0, 255); // Magenta
    neopixel_send_data(neopixel_buffer, sizeof(neopixel_buffer));
}

// *****************************************************************************
/** 
  @Function
    void test_flame(uint8_t count)

  @Summary
    Display a rotating flame pattern on the LED strip.

  @Remarks
    Refer to the neopixel.h interface header for function usage details.
 */
void test_flame(uint8_t count) {
    uint8_t count_base = count % NUM_LEDS;
    set_led_color(count_base, 255, 142, 0);
    set_led_color((count_base+1) % NUM_LEDS, 255, 127, 0);
    set_led_color((count_base+2) % NUM_LEDS, 255, 82, 0);
    set_led_color((count_base+3) % NUM_LEDS, 255, 67, 0);
    set_led_color((count_base+4) % NUM_LEDS, 255, 52, 0);
    set_led_color((count_base+5) % NUM_LEDS, 255, 37, 0);
    set_led_color((count_base+6) % NUM_LEDS, 255, 22, 0);
    set_led_color((count_base+7) % NUM_LEDS, 255, 0, 0);
    neopixel_send_data(neopixel_buffer, sizeof(neopixel_buffer));
}

// *****************************************************************************
/** 
  @Function
    void test_green_to_purple(uint8_t count)

  @Summary
    Display a rotating green-to-purple gradient pattern.

  @Remarks
    Refer to the neopixel.h interface header for function usage details.
 */
void test_green_to_purple(uint8_t count) {
    uint8_t count_base = count % NUM_LEDS;
    set_led_color(count_base, 0, 255, 0);           // Light green
    set_led_color((count_base+1) % NUM_LEDS, 0, 195, 0);     // Medium-light green
    set_led_color((count_base+2) % NUM_LEDS, 0, 125, 0);     // Medium green
    set_led_color((count_base+3) % NUM_LEDS, 0, 25, 0);      // Dark green
    set_led_color((count_base+4) % NUM_LEDS, 25, 0, 25);     // Dark purple
    set_led_color((count_base+5) % NUM_LEDS, 125, 0, 125);   // Medium purple
    set_led_color((count_base+6) % NUM_LEDS, 195, 0, 195);   // Medium-light purple
    set_led_color((count_base+7) % NUM_LEDS, 255, 0, 255);   // Light purple
    neopixel_send_data(neopixel_buffer, sizeof(neopixel_buffer));
}

// *****************************************************************************
/** 
  @Function
    void test_sequence(void)

  @Summary
    Sequentially light up LEDs one by one in blue.

  @Remarks
    Refer to the neopixel.h interface header for function usage details.
 */
void test_sequence(void) {
    // Light up LEDs one by one
    clear_all_leds();
    neopixel_send_data(neopixel_buffer, sizeof(neopixel_buffer));
    
    for(int i = 0; i < NUM_LEDS; i++) {
        set_led_color(i, 0, 0, 255); // Blue
        neopixel_send_data(neopixel_buffer, sizeof(neopixel_buffer));
        // Add delay here if you have a delay function
        for(volatile int d = 0; d < 1000000; d++); // Simple delay
    }
}

/* *****************************************************************************
 End of File
 */