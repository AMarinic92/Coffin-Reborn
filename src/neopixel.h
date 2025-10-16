/* ************************************************************************** */
/** NeoPixel Control Library

  @Company
    Microchip Technology Inc.

  @File Name
    neopixel.h

  @Summary
    NeoPixel/WS2812/SK6812 LED strip control library using SPI interface.

  @Description
    Controls a 144-LED NeoPixel strip using SERCOM1 SPI on a SAME51J20A
    microcontroller with a 3.3V-to-5V logic level shifter on MOSI (PA16).
    Supports 24-bit RGB color control with uniform color and moving rainbow patterns.
    Uses SPI at ~6.402 MHz.
 */
/* ************************************************************************** */

#ifndef _NEOPIXEL_H    /* Guard against multiple inclusion */
#define _NEOPIXEL_H

/* ************************************************************************** */
/* Section: Included Files */
/* ************************************************************************** */

#include <stdint.h>
#include <stdbool.h>

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

    /* ************************************************************************** */
    /* Section: Constants */
    /* ************************************************************************** */

    /** NeoPixel SPI Bit Patterns

      @Summary
        SPI byte patterns for encoding NeoPixel '0' and '1' bits at ~6.402 MHz SPI clock.
    
      @Description
        At ~6.402 MHz SPI clock, each bit is ~0.156 µs. NeoPixel protocol requires:
        - '0' bit: ~0.4 µs HIGH, ~0.85 µs LOW
        - '1' bit: ~0.8 µs HIGH, ~0.45 µs LOW
        Patterns account for logic level shifter delay (~10-20ns per bit).
    
      @Remarks
        ZERO_LED = 0xE0 (11100000) = ~0.468 µs HIGH, ~0.781 µs LOW
        ONE_LED = 0xF8 (11111000) = ~0.780 µs HIGH, ~0.468 µs LOW
     */
#define ZERO_LED 0xE0  // 11100000
#define ONE_LED  0xFC  // 11111000

    /** Number of LEDs in Strip

      @Summary
        Number of LEDs supported.
    
      @Description
        Configured for 144 LEDs. Each LED requires 24 SPI bytes (8 bytes each for G, R, B).
     */
#define NUM_LEDS 144

    /** Buffer Size

      @Summary
        Total buffer size for LED data and padding.
    
      @Description
        Buffer size: 24 bytes per LED × 144 LEDs + 96 padding bytes = 3552 bytes.
        Padding zeros absorb SPI startup glitches.
     */
#define NEOPIXEL_LEADING_ZEROS 96  // 4 LEDs worth of padding
#define NEOPIXEL_BUFFER_SIZE ((24 * NUM_LEDS) + NEOPIXEL_LEADING_ZEROS)

    /* ************************************************************************** */
    /* Section: Interface Functions */
    /* ************************************************************************** */

    void neopixel_init(void);
    void set_led_color(uint8_t led_index, uint8_t red, uint8_t green, uint8_t blue);
    void clear_all_leds(void);
    void neopixel_send_data(uint8_t *buffer, uint16_t length);
    void neopixel_send_data_manual(uint8_t *buffer, uint16_t length);
    void test_all_color(uint8_t red, uint8_t green, uint8_t blue);
    void test_moving_rainbow(uint8_t offset);

    /* ************************************************************************** */
    /* Section: Global Data */
    /* ************************************************************************** */

    extern uint8_t neopixel_buffer[NEOPIXEL_BUFFER_SIZE];
    extern volatile bool dma_complete;

/* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _NEOPIXEL_H */

/* *****************************************************************************
 End of File
 */