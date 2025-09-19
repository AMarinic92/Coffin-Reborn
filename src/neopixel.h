/* ************************************************************************** */
/** NeoPixel Control Library

  @Company
    Microchip Technology Inc.

  @File Name
    neopixel.h

  @Summary
    NeoPixel/WS2812/SK6812 LED strip control library using SPI interface.

  @Description
    This library provides functions to control NeoPixel LED strips using SPI
    communication on SAMD21 microcontrollers. It supports 8 LEDs with 24-bit
    color control (RGB) and includes various test patterns.
 */
/* ************************************************************************** */

#ifndef _NEOPIXEL_H    /* Guard against multiple inclusion */
#define _NEOPIXEL_H

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include <stdint.h>
#include <stdbool.h>

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

    /* ************************************************************************** */
    /* ************************************************************************** */
    /* Section: Constants                                                         */
    /* ************************************************************************** */
    /* ************************************************************************** */

    /* ************************************************************************** */
    /** NeoPixel SPI Bit Patterns

      @Summary
        SPI byte patterns for encoding NeoPixel '0' and '1' bits at 8MHz SPI clock.
    
      @Description
        At 8MHz SPI clock, each SPI bit is 0.125?s. NeoPixel protocol requires:
        - '0' bit: ~0.4?s HIGH, ~0.85?s LOW
        - '1' bit: ~0.8?s HIGH, ~0.45?s LOW
        
        These patterns approximate the timing using SPI bytes.
    
      @Remarks
        ZERO_LED = 0xE0 (11100000) = ~0.375?s HIGH, ~0.625?s LOW
        ONE_LED = 0xF8 (11111000) = ~0.625?s HIGH, ~0.375?s LOW
     */
#define ZERO_LED 0xE0
#define ONE_LED 0xF8

    /* ************************************************************************** */
    /** Number of LEDs in Strip

      @Summary
        Maximum number of LEDs supported by this library.
    
      @Description
        This library is configured for 8 LEDs. Each LED requires 24 SPI bytes
        for color data transmission (8 bytes each for G, R, B channels).
     */
#define NUM_LEDS 8

    /* ************************************************************************** */
    /** Buffer Size

      @Summary
        Total buffer size required for all LED data.
    
      @Description
        Buffer size calculation: 24 bytes per LED × 8 LEDs = 192 bytes total.
        This accounts for the SPI encoding where each color bit becomes one SPI byte.
     */
#define NEOPIXEL_BUFFER_SIZE (24 * NUM_LEDS)

    // *****************************************************************************
    // *****************************************************************************
    // Section: Data Types
    // *****************************************************************************
    // *****************************************************************************

    // *****************************************************************************
    // *****************************************************************************
    // Section: Interface Functions
    // *****************************************************************************
    // *****************************************************************************

    // *****************************************************************************
    /**
      @Function
        void neopixel_init(void)

      @Summary
        Initialize the NeoPixel library and DMA system.

      @Description
        Sets up the DMA callback for SPI data transmission to the NeoPixel strip.
        Must be called once before using any other NeoPixel functions.

      @Precondition
        SYS_Initialize() must have been called to set up the SPI and DMA peripherals.

      @Parameters
        None.

      @Returns
        None.

      @Remarks
        This function registers the DMA callback for transfer completion detection.

      @Example
        @code
        SYS_Initialize(NULL);
        neopixel_init();
        @endcode
     */
    void neopixel_init(void);

    // *****************************************************************************
    /**
      @Function
        void set_led_color(uint8_t led_index, uint8_t red, uint8_t green, uint8_t blue)

      @Summary
        Set the color of a specific LED in the buffer.

      @Description
        Sets the RGB color values for a specific LED in the SPI transmission buffer.
        Colors are encoded according to SK6812 protocol (G-R-B order) with SPI
        bit patterns. Data is not transmitted until neopixel_send_data() is called.

      @Precondition
        neopixel_init() must have been called.

      @Parameters
        @param led_index LED position (0-7) in the strip
        @param red Red color component (0-255)
        @param green Green color component (0-255) 
        @param blue Blue color component (0-255)

      @Returns
        None.

      @Remarks
        Invalid led_index values (>= 8) are ignored for safety.
        Color data is stored in buffer but not transmitted until send function is called.

      @Example
        @code
        set_led_color(0, 255, 0, 0);  // First LED red
        set_led_color(1, 0, 255, 0);  // Second LED green
        neopixel_send_data(neopixel_buffer, NEOPIXEL_BUFFER_SIZE);
        @endcode
     */
    void set_led_color(uint8_t led_index, uint8_t red, uint8_t green, uint8_t blue);

    // *****************************************************************************
    /**
      @Function
        void clear_all_leds(void)

      @Summary
        Turn off all LEDs by setting their colors to black.

      @Description
        Sets all LEDs in the buffer to RGB(0,0,0) which turns them off.
        Data is not transmitted until neopixel_send_data() is called.

      @Precondition
        None.

      @Parameters
        None.

      @Returns
        None.

      @Example
        @code
        clear_all_leds();
        neopixel_send_data(neopixel_buffer, NEOPIXEL_BUFFER_SIZE);
        @endcode
     */
    void clear_all_leds(void);

    // *****************************************************************************
    /**
      @Function
        void neopixel_send_data(uint8_t *buffer, uint16_t length)

      @Summary
        Send LED color data to the strip using DMA transfer.

      @Description
        Transmits the LED color buffer to the NeoPixel strip using DMA-controlled
        SPI transfer. Includes the required reset pulse after transmission.

      @Precondition
        neopixel_init() must have been called.
        SPI peripheral must be configured and enabled.

      @Parameters
        @param buffer Pointer to the LED data buffer
        @param length Number of bytes to transmit

      @Returns
        None.

      @Remarks
        Function blocks until DMA transfer is complete.
        Automatically generates 100?s reset pulse after data transmission.

      @Example
        @code
        neopixel_send_data(neopixel_buffer, NEOPIXEL_BUFFER_SIZE);
        @endcode
     */
    void neopixel_send_data(uint8_t *buffer, uint16_t length);

    // *****************************************************************************
    /**
      @Function
        void neopixel_send_data_manual(uint8_t *buffer, uint16_t length)

      @Summary
        Send LED color data using manual SPI transmission (blocking).

      @Description
        Alternative transmission method that manually sends each byte via SPI
        without using DMA. Useful for debugging or when DMA is not available.

      @Precondition
        SPI peripheral must be configured and enabled.

      @Parameters
        @param buffer Pointer to the LED data buffer
        @param length Number of bytes to transmit

      @Returns
        None.

      @Remarks
        This is a blocking function that waits for each byte to transmit.
        Includes 100?s reset pulse after transmission.

      @Example
        @code
        neopixel_send_data_manual(neopixel_buffer, 24); // Send first LED only
        @endcode
     */
    void neopixel_send_data_manual(uint8_t *buffer, uint16_t length);

    // *****************************************************************************
    /**
      @Function
        void test_flame(uint8_t count)

      @Summary
        Display a rotating flame pattern on the LED strip.

      @Description
        Creates a flame-like color gradient that rotates around the 8-LED strip.
        Colors transition from bright orange-yellow to deep red.

      @Parameters
        @param count Rotation offset (0-255), modulo 8 determines starting position

      @Returns
        None.

      @Example
        @code
        for(int i = 0; i < 100; i++) {
            test_flame(i);
            delay(100);
        }
        @endcode
     */
    void test_flame(uint8_t count);

    // *****************************************************************************
    /**
      @Function
        void test_green_to_purple(uint8_t count)

      @Summary
        Display a rotating green-to-purple gradient pattern.

      @Description
        Creates a gradient from light green through dark green, then dark purple
        to light purple. Pattern rotates around the strip based on count value.

      @Parameters
        @param count Rotation offset (0-255), modulo 8 determines starting position

      @Returns
        None.

      @Example
        @code
        for(int i = 0; i < 100; i++) {
            test_green_to_purple(i);
            delay(100);
        }
        @endcode
     */
    void test_green_to_purple(uint8_t count);

    // *****************************************************************************
    /**
      @Function
        void test_rainbow(void)

      @Summary
        Display a static rainbow pattern across all LEDs.

      @Description
        Sets each LED to a different color of the rainbow spectrum:
        Red, Orange, Yellow, Green, Cyan, Blue, Violet, Magenta.

      @Returns
        None.

      @Example
        @code
        test_rainbow();
        @endcode
     */
    void test_rainbow(void);

    // *****************************************************************************
    /**
      @Function
        void test_single_red(void), test_single_green(void), test_single_blue(void)

      @Summary
        Test functions to light up the first LED in a specific color.

      @Description
        These functions clear all LEDs and then light up only the first LED
        in red, green, or blue respectively. Useful for testing and debugging.

      @Returns
        None.

      @Example
        @code
        test_single_red();   // First LED red, others off
        delay(1000);
        test_single_green(); // First LED green, others off
        @endcode
     */
    void test_single_red(void);
    void test_single_green(void);  
    void test_single_blue(void);

    // *****************************************************************************
    /**
      @Function
        void test_all_red(void)

      @Summary
        Set all LEDs to red color.

      @Returns
        None.
     */
    void test_all_red(void);

    // *****************************************************************************
    /**
      @Function
        void test_sequence(void)

      @Summary
        Sequentially light up LEDs one by one in blue.

      @Description
        Demonstrates sequential LED control by lighting each LED in blue
        with a delay between each one.

      @Returns
        None.
     */
    void test_sequence(void);

    // *****************************************************************************
    // *****************************************************************************
    // Section: Global Data                                                       
    // *****************************************************************************
    // *****************************************************************************

    /* ************************************************************************** */
    /** NeoPixel Data Buffer

      @Summary
        Global buffer containing SPI-encoded LED color data.
    
      @Description
        This buffer holds the SPI-encoded color data for all 8 LEDs.
        Each LED requires 24 bytes (8 bytes each for G, R, B channels).
        Buffer contents are transmitted to the LED strip via SPI.
    
      @Remarks
        Buffer is filled by set_led_color() and transmitted by send functions.
        External access allows for advanced buffer manipulation if needed.
     */
    extern uint8_t neopixel_buffer[NEOPIXEL_BUFFER_SIZE];

    /* ************************************************************************** */
    /** DMA Transfer Complete Flag

      @Summary
        Volatile flag indicating DMA transfer completion status.
    
      @Description
        Set to true by the DMA callback when transfer is complete.
        Used by neopixel_send_data() to wait for transmission completion.
    
      @Remarks
        Volatile qualifier ensures flag changes are visible across ISR boundary.
     */
    extern volatile bool dma_complete;

/* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _NEOPIXEL_H */

/* *****************************************************************************
 End of File
 */