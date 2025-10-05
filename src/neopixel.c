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
    for(int i = 0; i < NUM_LEDS; i+= 8){
        set_led_color(i, 255, 142, 0);
        set_led_color((1+i) , 255, 127, 0);
        set_led_color((2+i), 255, 82, 0);
        set_led_color((3+i), 255, 67, 0);
        set_led_color((4+i), 255, 52, 0);
        set_led_color((5+i), 255, 37, 0);
        set_led_color((6+i), 255, 22, 0);
        set_led_color((7+i), 255, 0, 0);
    }
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
        set_led_color(i, i, 0, 175); // Blue
        neopixel_send_data(neopixel_buffer, sizeof(neopixel_buffer));
        // Add delay here if you have a delay function
        for(volatile int d = 0; d < 1000000; d++); // Simple delay
    }
}

/** 
  @Function
    uint32_t get_random_number(void)

  @Summary
    Get a 32-bit random number from the TRNG using direct register access.
 */
uint32_t get_random_number(void) {
    // Enable TRNG if not already enabled
    if((TRNG_REGS->TRNG_CTRLA & TRNG_CTRLA_ENABLE_Msk) == 0) {
        // Enable TRNG peripheral clock
        MCLK_REGS->MCLK_APBCMASK |= MCLK_APBCMASK_TRNG_Msk;
        
        // Enable TRNG
        TRNG_REGS->TRNG_CTRLA = TRNG_CTRLA_ENABLE_Msk;
        
        // Wait for first random number to be ready
        while((TRNG_REGS->TRNG_INTFLAG & TRNG_INTFLAG_DATARDY_Msk) == 0);
    }
    
    // Wait for data ready
    while((TRNG_REGS->TRNG_INTFLAG & TRNG_INTFLAG_DATARDY_Msk) == 0);
    
    // Read and return random data
    return TRNG_REGS->TRNG_DATA;
}

// *****************************************************************************
/** 
  @Function
    void get_random_red_orange(uint8_t *red, uint8_t *green, uint8_t *blue)

  @Summary
    Generate a random red/orange color using TRNG.
 */
void get_random_red_orange(uint8_t *red, uint8_t *green, uint8_t *blue) {
    uint32_t random = get_random_number();
    
    // Red is always maximum for red/orange spectrum
    *red = 255;
    
    // Green varies from 0 to 150 to create red->orange gradient
    // 0 = pure red, 150 = orange
    *green = (uint8_t)(random % 151);
    
    // Blue is always 0 for red/orange spectrum
    *blue = 0;
}

// *****************************************************************************
/** 
  @Function
    void test_sequence_random(void)

  @Summary
    Sequentially light up LEDs with random red/orange colors.
 */
void test_sequence_random(void) {
    uint8_t r, g, b;
    
    // Clear all LEDs first
    clear_all_leds();
    neopixel_send_data(neopixel_buffer, sizeof(neopixel_buffer));
    
    // Light up LEDs one by one with random red/orange colors
    for(int i = 0; i < NUM_LEDS; i++) {
        // Get random red/orange color
        get_random_red_orange(&r, &g, &b);
        
        // Set LED color
        set_led_color(i, r, g, b);
        
        // Send data to strip
        neopixel_send_data(neopixel_buffer, sizeof(neopixel_buffer));
        
        // Delay (adjust as needed - approximately 50ms)
        for(volatile int d = 0; d < 500000; d++);
    }
}

// *****************************************************************************
/** 
  @Function
    void test_random_sparkle(uint8_t num_sparkles, uint32_t delay_ms)

  @Summary
    Create random red/orange sparkles across the LED strip.
 */
void test_random_sparkle(uint8_t num_sparkles, uint32_t delay_ms) {
    uint8_t r, g, b;
    
    for(uint8_t i = 0; i < num_sparkles; i++) {
        // Get random LED index
        uint32_t random = get_random_number();
        uint8_t led_index = (uint8_t)(random % NUM_LEDS);
        
        // Get random red/orange color
        get_random_red_orange(&r, &g, &b);
        
        // Set LED color
        set_led_color(led_index, r, g, b);
    }
    
    // Send all changes at once
    neopixel_send_data(neopixel_buffer, sizeof(neopixel_buffer));
    
    // Simple delay approximation (not precise milliseconds)
    for(volatile uint32_t d = 0; d < (delay_ms * 10000); d++);
}

// *****************************************************************************
/** 
  @Function
    void test_random_fill(void)

  @Summary
    Fill all LEDs with random red/orange colors simultaneously.
 */
void test_random_fill(void) {
    uint8_t r, g, b;
    
    // Set each LED to a random red/orange color
    for(int i = 0; i < NUM_LEDS; i++) {
        get_random_red_orange(&r, &g, &b);
        set_led_color(i, r, g, b);
    }
    
    // Send all at once
    neopixel_send_data(neopixel_buffer, sizeof(neopixel_buffer));
}

// *****************************************************************************
/** 
  @Function
    void test_random_wave(uint8_t wave_position)

  @Summary
    Create a traveling wave of random red/orange colors.
 */
void test_random_wave(uint8_t wave_position) {
    uint8_t r, g, b;
    
    // Shift existing colors down one position
    for(int i = NUM_LEDS - 1; i > 0; i--) {
        // Copy color data from previous LED (24 bytes per LED)
        uint8_t *src = &neopixel_buffer[(i-1) * 24];
        uint8_t *dst = &neopixel_buffer[i * 24];
        for(int j = 0; j < 24; j++) {
            dst[j] = src[j];
        }
    }
    
    // Set first LED to new random color
    get_random_red_orange(&r, &g, &b);
    set_led_color(0, r, g, b);
    
    // Send data
    neopixel_send_data(neopixel_buffer, sizeof(neopixel_buffer));
}

// *****************************************************************************
/** 
  @Function
    void test_red_orange_gradient_shift(uint8_t shift_offset)

  @Summary
    Display a smooth red-to-orange gradient that shifts along the strip.
 */
void test_red_orange_gradient_shift(uint8_t shift_offset) {
    // Gradient repeats every 24 LEDs (144/24 = 6 complete cycles)
    // Using dramatic color differences with blue accents for high contrast
    const uint8_t GRADIENT_LENGTH = 24;
    
    // Create strong color steps with distinct zones
    for(int i = 0; i < NUM_LEDS; i++) {
        // Calculate position in gradient with shift
        uint8_t pos = (i + shift_offset) % GRADIENT_LENGTH;
        
        uint8_t red, green, blue;
        
        // Create 6 highly contrasted color zones within each 24-LED cycle
        // Each zone is 4 LEDs wide for strong visual separation
        if(pos < 4) {
            // Zone 1: Deep Red
            red = 255; green = 0; blue = 0;
        }
        else if(pos < 8) {
            // Zone 2: Purple-Red (adds blue for contrast)
            red = 255; green = 0; blue = 80;
        }
        else if(pos < 12) {
            // Zone 3: Orange-Red
            red = 255; green = 100; blue = 0;
        }
        else if(pos < 16) {
            // Zone 4: Red-Orange with blue tint
            red = 255; green = 80; blue = 40;
        }
        else if(pos < 20) {
            // Zone 5: Bright Orange
            red = 255; green = 180; blue = 0;
        }
        else {
            // Zone 6: Pink-Orange (blue accent for variety)
            red = 255; green = 100; blue = 120;
        }
        
        set_led_color(i, red, green, blue);
    }
    
    neopixel_send_data(neopixel_buffer, sizeof(neopixel_buffer));
}
/* *****************************************************************************
 End of File
 */