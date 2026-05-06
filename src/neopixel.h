/* =============================================================================
 * neopixel.h  ?  WS2812B / NeoPixel driver via SERCOM SPI + DMAC
 * Target : ATSAME51J20A  (SAME51 Curiosity Nano, EV76S68A)
 * Toolchain : MPLAB X + MCC Melody + XC32
 *
 * ENCODING SCHEME
 * ---------------
 * Each NeoPixel bit is encoded as 3 SPI bits at 2.4 MHz (416.7 ns/bit):
 *
 *   NeoPixel '1' ? SPI 1-1-0  ?  T1H = 833 ns  T1L = 417 ns   ? (spec 800±150 / 450±150)
 *   NeoPixel '0' ? SPI 1-0-0  ?  T0H = 417 ns  T0L = 833 ns   ? (spec 400±150 / 850±150)
 *
 * 24 NeoPixel bits (1 LED, GRB order) ? 72 SPI bits ? 9 SPI bytes
 * Buffer tail: RESET_BYTES × 0x00 ? keeps MOSI low ? 167 µs  (> 50 µs reset minimum)
 *
 * HARDWARE CONNECTIONS
 * --------------------
 *   SAME51 SERCOM1 MOSI (PA16, MUX-C) ??? 74AHCT125 input (powered at 5 V)
 *   74AHCT125 output ??[330 ?]??? NeoPixel DATA-IN
 *   SCK (PA17) is toggled by SPI but NOT connected to NeoPixel.
 *   SS  (PA18) leave as GPIO or disable ? not used.
 *
 * Do NOT use BSS138 or TXB0104 shifters ? both are too slow for 800 kHz edges.
 * 74AHCT125 has ~7 ns propagation and full 5 V swing from a 3.3 V input.
 * ============================================================================= */

#ifndef NEOPIXEL_H
#define NEOPIXEL_H

#include <stdint.h>
#include <stdbool.h>

/* ?? User configuration ?????????????????????????????????????????????????????? */
#define NUM_LEDS            144          /* adjust to your strip length          */
#define DMAC_CHANNEL_NEO    DMAC_CHANNEL_0  /* must match MCC DMAC assignment   */

/* ?? Derived constants ? do not edit ???????????????????????????????????????? */
#define NEO_RESET_BYTES     50u         /* 50 × 8 × 416.7 ns ? 167 µs ? 50 µs */
#define NEO_DATA_BYTES      ((uint16_t)(NUM_LEDS) * 9u)
#define NEO_BUF_SIZE        (NEO_DATA_BYTES + NEO_RESET_BYTES)

/* ?? Public API ?????????????????????????????????????????????????????????????? */

/** Call once after SYSTEM_Initialize(). Registers DMA callback, zeroes buffer. */
void NeoPixel_Init(void);

/**
 * Stage a single pixel colour into the DMA buffer (does NOT transmit yet).
 * index : 0 ? NUM_LEDS-1
 * r,g,b : 0 ? 255
 */
void NeoPixel_SetPixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

/** Stage all pixels off (does NOT transmit). */
void NeoPixel_Clear(void);

/**
 * Transmit the buffer via SPI+DMA. Blocks until DMA finishes.
 * The reset pulse is baked into the tail of the buffer, so no extra delay needed.
 */
void NeoPixel_Show(void);

/**
 * Fill the strip with a moving rainbow and call Show().
 * offset : 0-255, increment each frame to animate.
 * brightness : 0-255 (keep ? 128 when powering many LEDs from USB).
 */
void NeoPixel_Rainbow(uint8_t offset, uint8_t brightness);

void NeoPixel_GreenPurple(uint8_t offset, uint8_t brightness);

void NeoPixel_Fire(uint8_t offset, uint8_t brightness);

/**
 * HSV ? RGB helper (public so main.c can compose custom effects).
 * h,s,v : 0-255
 */
void NeoPixel_HSVtoRGB(uint8_t h, uint8_t s, uint8_t v,
                        uint8_t *r, uint8_t *g, uint8_t *b);

#endif /* NEOPIXEL_H */