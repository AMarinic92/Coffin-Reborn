/* =============================================================================
 * neopixel.c  ?  WS2812B / NeoPixel driver via SERCOM SPI + DMAC
 * Target : ATSAME51J20A   MPLAB X + MCC Melody + XC32
 * ============================================================================= */

#include "neopixel.h"
#include "definitions.h"   /* MCC Melody umbrella ? pulls in SERCOM1, DMAC, SYSTICK */
#include <string.h>

/* ?? Internal state ??????????????????????????????????????????????????????????? */

static uint8_t  neo_buf[NEO_BUF_SIZE];
static volatile bool tx_done = false;

/* ?? DMA callback ????????????????????????????????????????????????????????????? */

static void NeoPixel_DMA_Callback(DMAC_TRANSFER_EVENT event, uintptr_t context)
{
    (void)context;
    if (event == DMAC_TRANSFER_EVENT_COMPLETE)
        tx_done = true;
}

/* ?? Bit encoding ????????????????????????????????????????????????????????????? */

/*
 * encode_byte()
 * Converts one 8-bit NeoPixel colour component into 3 SPI bytes.
 *
 * Each NeoPixel bit ? 3 SPI bits:
 *   pixel '1'  ?  1 1 0  (0x6)
 *   pixel '0'  ?  1 0 0  (0x4)
 *
 * 8 pixel bits × 3 SPI bits = 24 SPI bits = 3 bytes, packed MSB-first.
 *
 * Example: pixel byte 0xC0 = 1100 0000
 *   bit7=1 ? 110 | bit6=1 ? 110 | bit5=0 ? 100 | bit4=0 ? 100 | ?
 *   24-bit SPI word = 110 110 100 100 100 100 100 100
 *                   = 1101 1010 0100 1001 0010 0100
 *                   = 0xDA  0x49  0x24
 */
static inline void encode_byte(uint8_t pixel_byte, uint8_t *out)
{
    uint32_t word = 0;
    for (int8_t i = 7; i >= 0; i--)
    {
        word = (word << 3u) | (((pixel_byte >> i) & 1u) ? 0x6u : 0x4u);
    }
    out[0] = (uint8_t)(word >> 16u);
    out[1] = (uint8_t)(word >>  8u);
    out[2] = (uint8_t)(word        );
}

/* ?? Public API implementation ???????????????????????????????????????????????? */

void NeoPixel_Init(void)
{
    memset(neo_buf, 0x00, sizeof(neo_buf));
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_NEO, NeoPixel_DMA_Callback, 0u);
}

void NeoPixel_SetPixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
    if (index >= NUM_LEDS) return;

    uint8_t *p = &neo_buf[(uint16_t)index * 9u];
    encode_byte(g, p);       /* WS2812B / SK6812 wire order is G ? R ? B */
    encode_byte(r, p + 3u);
    encode_byte(b, p + 6u);
}

void NeoPixel_Clear(void)
{
    for (uint8_t i = 0; i < NUM_LEDS; i++)
        NeoPixel_SetPixel(i, 0u, 0u, 0u);
}

void NeoPixel_Show(void)
{
    tx_done = false;

    DMAC_ChannelTransfer(
        DMAC_CHANNEL_NEO,
        (const void *)neo_buf,
        (const void *)&SERCOM1_REGS->SPIM.SERCOM_DATA,
        NEO_BUF_SIZE
    );

    while (!tx_done)
    {
        /* optionally: asm("nop"); */
    }
}

/* ?? Colour helpers ??????????????????????????????????????????????????????????? */

void NeoPixel_HSVtoRGB(uint8_t h, uint8_t s, uint8_t v,
                        uint8_t *r, uint8_t *g, uint8_t *b)
{
    if (s == 0u)
    {
        *r = *g = *b = v;
        return;
    }

    uint8_t region    = h / 43u;
    uint8_t remainder = (uint8_t)((h - (region * 43u)) * 6u);

    uint8_t p = (uint8_t)(((uint16_t)v * (255u - s)) >> 8u);
    uint8_t q = (uint8_t)(((uint16_t)v * (255u - (((uint16_t)s * remainder) >> 8u))) >> 8u);
    uint8_t t = (uint8_t)(((uint16_t)v * (255u - (((uint16_t)s * (255u - remainder)) >> 8u))) >> 8u);

    switch (region)
    {
        case 0:  *r = v; *g = t; *b = p; break;
        case 1:  *r = q; *g = v; *b = p; break;
        case 2:  *r = p; *g = v; *b = t; break;
        case 3:  *r = p; *g = q; *b = v; break;
        case 4:  *r = t; *g = p; *b = v; break;
        default: *r = v; *g = p; *b = q; break;
    }
}

void NeoPixel_Rainbow(uint8_t offset, uint8_t brightness)
{
    for (uint8_t i = 0; i < NUM_LEDS; i++)
    {
        uint8_t hue = (uint8_t)(((uint16_t)i * 256u / NUM_LEDS) + offset);
        uint8_t r, g, b;
        NeoPixel_HSVtoRGB(hue, 255u, brightness, &r, &g, &b);
        NeoPixel_SetPixel(i, r, g, b);
    }
    NeoPixel_Show();
}