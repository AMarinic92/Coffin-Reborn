/* ************************************************************************** */
/** NeoPixel Control Library - TCC/DMA Implementation

  @Company
    Microchip Technology Inc.

  @File Name
    neopixel_tcc.h

  @Summary
    NeoPixel/WS2812/SK6812 LED strip control library using TCC and DMA.

  @Description
    This library provides functions to control NeoPixel LED strips using TCC
    timer with DMA on SAME51 microcontrollers. This method provides perfect
    timing and zero CPU usage during transmission.
 */
/* ************************************************************************** */

#ifndef _NEOPIXEL_TCC_H
#define _NEOPIXEL_TCC_H

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

    /** Number of LEDs in Strip */
    #define NUM_LEDS 144

    /** TCC Compare values for NeoPixel bits at 120 MHz
     * 
     * @Summary
     *   TCC compare values to generate proper timing for SK6812 LEDs.
     * 
     * @Description
     *   At 120 MHz, each TCC count = 8.33ns
     *   - ZERO bit: 36 counts = 300ns HIGH, 114 counts LOW = 950ns
     *   - ONE bit:  72 counts = 600ns HIGH, 78 counts LOW = 650ns
     *   Period: 150 counts = 1.25µs (800 kHz)
     */
    #define TCC_ZERO_BIT  36   // 300ns HIGH pulse
    #define TCC_ONE_BIT   72   // 600ns HIGH pulse

    /** Buffer size for TCC data
     * 
     * @Summary
     *   Total buffer size for all LED data.
     * 
     * @Description
     *   Each LED requires 24 bytes (8 bits × 3 colors)
     *   Total: 144 LEDs × 24 bytes = 3456 bytes
     */
    #define NEOPIXEL_TCC_BUFFER_SIZE (NUM_LEDS * 24)

    /* ************************************************************************** */
    /* ************************************************************************** */
    /* Section: Interface Functions                                               */
    /* ************************************************************************** */
    /* ************************************************************************** */

    // *****************************************************************************
    /**
      @Function
        void neopixel_tcc_init(void)

      @Summary
        Initialize the NeoPixel library with TCC and DMA.

      @Description
        Sets up TCC0 for PWM generation and registers DMA callback for transfer
        completion detection. Must be called once before using any other NeoPixel
        functions.

      @Precondition
        SYS_Initialize() must have been called to set up TCC0 and DMA peripherals.
        TCC0 must be configured in MCC with:
        - Period: 149 (for 800 kHz at 120 MHz)
        - Prescaler: DIV1
        - WO[0] output enabled on PA08

      @Parameters
        None.

      @Returns
        None.

      @Example
        @code
        SYS_Initialize(NULL);
        neopixel_tcc_init();
        @endcode
     */
    void neopixel_init(void);

    // *****************************************************************************
    /**
      @Function
        void set_led_color_tcc(uint8_t led_index, uint8_t red, uint8_t green, 
                               uint8_t blue)

      @Summary
        Set the color of a specific LED in the buffer.

      @Description
        Sets the RGB color values for a specific LED in the TCC transmission buffer.
        Colors are encoded as TCC compare values according to SK6812 protocol 
        (G-R-B order). Data is not transmitted until neopixel_send_tcc() is called.

      @Precondition
        neopixel_tcc_init() must have been called.

      @Parameters
        @param led_index LED position (0-143) in the strip
        @param red Red color component (0-255)
        @param green Green color component (0-255) 
        @param blue Blue color component (0-255)

      @Returns
        None.

      @Remarks
        Invalid led_index values (>= 144) are ignored for safety.
        Color data is stored in buffer but not transmitted until send function 
        is called.

      @Example
        @code
        set_led_color_tcc(0, 255, 0, 0);  // First LED red
        set_led_color_tcc(1, 0, 255, 0);  // Second LED green
        neopixel_send_tcc();
        @endcode
     */
    void set_led_color_tcc(uint8_t led_index, uint8_t red, uint8_t green, 
                           uint8_t blue);

    // *****************************************************************************
    /**
      @Function
        void clear_all_leds_tcc(void)

      @Summary
        Turn off all LEDs by setting their colors to black.

      @Description
        Sets all LEDs in the buffer to RGB(0,0,0) which turns them off.
        Data is not transmitted until neopixel_send_tcc() is called.

      @Precondition
        None.

      @Parameters
        None.

      @Returns
        None.

      @Example
        @code
        clear_all_leds_tcc();
        neopixel_send_tcc();
        @endcode
     */
    void clear_all_leds_tcc(void);

    // *****************************************************************************
    /**
      @Function
        void neopixel_send_tcc(void)

      @Summary
        Send LED color data to the strip using TCC and DMA.

      @Description
        Transmits the LED color buffer to the NeoPixel strip using DMA-controlled
        TCC PWM generation. Includes the required reset pulse after transmission.
        This function uses zero CPU during transmission - the hardware handles
        everything.

      @Precondition
        neopixel_tcc_init() must have been called.
        TCC0 peripheral must be configured and enabled.

      @Parameters
        None.

      @Returns
        None.

      @Remarks
        Function blocks until DMA transfer is complete.
        Automatically generates 100µs reset pulse after data transmission.
        Zero CPU usage during transmission - perfect for real-time applications.

      @Example
        @code
        set_led_color_tcc(0, 255, 0, 0);
        neopixel_send_tcc();
        @endcode
     */
    void neopixel_send_tcc(void);

    // *****************************************************************************
    /**
      @Function
        void gradient_green_white_purple_tcc(uint32_t duration, uint32_t step)

      @Summary
        Display a moving gradient pattern using TCC method.

      @Description
        Creates a smooth gradient that cycles through green, white, and purple 
        colors. The pattern moves along the LED strip in a circular fashion.
        Uses TCC/DMA for perfect timing.

      @Parameters
        @param duration Total time this pattern should run (in milliseconds)
        @param step Current position in the animation (updated periodically)

      @Returns
        None.

      @Example
        @code
        static uint32_t gradient_step = 0;
        if ((millis() - last_update) >= 50) {
            last_update = millis();
            gradient_green_white_purple_tcc(10000, gradient_step);
            gradient_step++;
        }
        @endcode
     */
    void gradient_green_white_purple_tcc(uint32_t duration, uint32_t step);

    // *****************************************************************************
    /**
      @Function
        void test_rainbow_moving_tcc(uint32_t step)

      @Summary
        Display a moving rainbow pattern using TCC method.

      @Description
        Creates a full spectrum rainbow that moves along the LED strip.
        Uses HSV to RGB conversion for accurate color rendering with TCC/DMA.

      @Parameters
        @param step Current position in the animation (increment to move)

      @Returns
        None.

      @Example
        @code
        static uint32_t rainbow_step = 0;
        if ((millis() - last_update) >= 50) {
            last_update = millis();
            test_rainbow_moving_tcc(rainbow_step);
            rainbow_step++;
        }
        @endcode
     */
    void test_rainbow_moving_tcc(uint32_t step);

    // *****************************************************************************
    /**
      @Function
        void test_all_red_tcc(void)

      @Summary
        Test function to set all LEDs to red using TCC method.

      @Returns
        None.
     */
    void test_all_red_tcc(void);

    // *****************************************************************************
    /**
      @Function
        void test_rgb_sequence_tcc(void)

      @Summary
        Test function to display RGB sequence using TCC method.

      @Description
        Lights up first 10 LEDs in red, next 10 in green, next 10 in blue.
        Useful for testing TCC/DMA functionality.

      @Returns
        None.
     */
    void test_rgb_sequence_tcc(void);

    /* ************************************************************************** */
    /* ************************************************************************** */
    /* Section: Global Data                                                       */
    /* ************************************************************************** */
    /* ************************************************************************** */

    /** NeoPixel TCC Data Buffer
     * 
     * @Summary
     *   Global buffer containing TCC compare values for all LED bits.
     * 
     * @Description
     *   This buffer holds 3456 bytes of TCC compare values (24 bytes per LED).
     *   Each byte represents one bit of color data as a TCC compare value.
     *   Buffer contents are transmitted via DMA to TCC CCB register.
     * 
     * @Remarks
     *   Buffer is filled by set_led_color_tcc() and transmitted by DMA.
     *   External access allows for advanced buffer manipulation if needed.
     */
    extern uint8_t neopixel_tcc_buffer[NEOPIXEL_TCC_BUFFER_SIZE];

    /** DMA Transfer Complete Flag
     * 
     * @Summary
     *   Volatile flag indicating DMA transfer completion status.
     * 
     * @Description
     *   Set to false before starting DMA transfer, set to true by DMA callback
     *   when transfer completes. Used for synchronization in neopixel_send_tcc().
     * 
     * @Remarks
     *   Volatile qualifier ensures proper behavior across interrupt boundaries.
     */
    extern volatile bool neopixel_dma_complete;

/* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _NEOPIXEL_TCC_H */

/* *****************************************************************************
 End of File
 */