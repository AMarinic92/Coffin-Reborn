/* ************************************************************************** */
/** NeoPixel Control Library

  @Company
    Microchip Technology Inc.

  @File Name
    neopixel.c

  @Summary
    NeoPixel/WS2812/SK6812 LED strip control implementation.

  @Description
    Controls a 144-LED strip using SERCOM1 SPI on SAME51J20A with a 3.3V-to-5V
    logic level shifter for MOSI (PA16). Supports manual and DMA SPI transmission.
 */
/* ************************************************************************** */

/* ************************************************************************** */
/* Section: Included Files */
/* ************************************************************************** */

#include "neopixel.h"
#include "definitions.h"
#include <stdio.h>

/* ************************************************************************** */
/* Section: File Scope or Global Data */
/* ************************************************************************** */

uint8_t neopixel_buffer[NEOPIXEL_BUFFER_SIZE];
volatile bool dma_complete = false;
static uint8_t dummy_buffer[384] = {0};

/* ************************************************************************** */
/* Section: Local Functions */
/* ************************************************************************** */

static void delay_us(uint32_t us) {
    uint32_t cycles = us * 120;
    for(volatile uint32_t i = 0; i < cycles; i++) {
        __NOP();
    }
}

static void DMA_0_Callback(DMAC_TRANSFER_EVENT event, uintptr_t context) {
    if(event == DMAC_TRANSFER_EVENT_COMPLETE) {
        dma_complete = true;
    } else if(event == DMAC_TRANSFER_EVENT_ERROR) {
        printf("DMA Error on Channel 0\n");
    }
}

static void debug_led_buffer(void) {
    printf("LED 0-7 Buffer (192 bytes):\n");
    printf("LED 0 (Red: 255,0,0): Bytes 0-23 (G:0x00, R:0xFF, B:0x00)\n");
    printf("LED 1 (Orange: 255,127,0): Bytes 24-47 (G:0x7F, R:0xFF, B:0x00)\n");
    printf("LED 2 (Yellow: 255,255,0): Bytes 48-71 (G:0xFF, R:0xFF, B:0x00)\n");
    printf("LED 3 (Green: 0,255,0): Bytes 72-95 (G:0xFF, R:0x00, B:0x00)\n");
    printf("LED 4 (Cyan: 0,255,255): Bytes 96-119 (G:0xFF, R:0x00, B:0xFF)\n");
    printf("LED 5 (Blue: 0,0,255): Bytes 120-143 (G:0x00, R:0x00, B:0xFF)\n");
    printf("LED 6 (Violet: 127,0,255): Bytes 144-167 (G:0x7F, R:0x00, B:0xFF)\n");
    printf("LED 7 (Magenta: 255,0,255): Bytes 168-191 (G:0x00, R:0xFF, B:0xFF)\n");
    for(int i = 0; i < 192; i++) {
        printf("Byte %d: 0x%02X\n", i, neopixel_buffer[NEOPIXEL_LEADING_ZEROS + i]);
    }
}

/* ************************************************************************** */
/* Section: Interface Functions */
/* ************************************************************************** */

void neopixel_init(void) {
    for(int i = 0; i < NEOPIXEL_BUFFER_SIZE; i++) {
        neopixel_buffer[i] = 0x00;
    }
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, DMA_0_Callback, 0);
}

void set_led_color(uint8_t led_index, uint8_t red, uint8_t green, uint8_t blue) {
    if(led_index >= NUM_LEDS) return;
    
    uint8_t *led_buffer = &neopixel_buffer[NEOPIXEL_LEADING_ZEROS + (led_index * 24)];
    
    // Green (MSB first)
    for(int bit = 0; bit < 8; bit++) {
        led_buffer[bit] = (green & (1 << (7-bit))) ? ONE_LED : ZERO_LED;
    }
    // Red
    for(int bit = 0; bit < 8; bit++) {
        led_buffer[8 + bit] = (red & (1 << (7-bit))) ? ONE_LED : ZERO_LED;
    }
    // Blue
    for(int bit = 0; bit < 8; bit++) {
        led_buffer[16 + bit] = (blue & (1 << (7-bit))) ? ONE_LED : ZERO_LED;
    }
}

void clear_all_leds(void) {
    for(int i = 0; i < NEOPIXEL_BUFFER_SIZE; i++) {
        neopixel_buffer[i] = 0x00;
    }
}

void neopixel_send_data(uint8_t *buffer, uint16_t length) {
    __disable_irq();
    
    SERCOM1_REGS->SPIM.SERCOM_CTRLA &= ~SERCOM_SPIM_CTRLA_ENABLE_Msk;
    while((SERCOM1_REGS->SPIM.SERCOM_SYNCBUSY) != 0U);
    
    PORT_REGS->GROUP[0].PORT_PINCFG[16] &= ~PORT_PINCFG_PMUXEN_Msk;
    PORT_REGS->GROUP[0].PORT_DIRSET = (1UL << 16);
    PORT_REGS->GROUP[0].PORT_OUTCLR = (1UL << 16);
    
    delay_us(200);
    
    PORT_REGS->GROUP[0].PORT_OUTSET = (1UL << 16);
    delay_us(10);
    
    PORT_REGS->GROUP[0].PORT_PINCFG[16] |= PORT_PINCFG_PMUXEN_Msk;
    SERCOM1_REGS->SPIM.SERCOM_CTRLA |= SERCOM_SPIM_CTRLA_ENABLE_Msk;
    while((SERCOM1_REGS->SPIM.SERCOM_SYNCBUSY) != 0U);
    
    delay_us(10); // Extra sync delay
    
    dma_complete = false;
    DMAC_ChannelTransfer(DMAC_CHANNEL_0, 
                        dummy_buffer,
                        (const void*)&SERCOM1_REGS->SPIM.SERCOM_DATA,
                        384);
    while(!dma_complete) {
        if(DMAC_ChannelTransferStatusGet(DMAC_CHANNEL_0) == DMAC_TRANSFER_EVENT_ERROR) {
            printf("Dummy DMA Transfer Error\n");
            break;
        }
    }
    
    dma_complete = false;
    DMAC_ChannelTransfer(DMAC_CHANNEL_0, 
                        buffer,
                        (const void*)&SERCOM1_REGS->SPIM.SERCOM_DATA,
                        length);
    while(!dma_complete) {
        if(DMAC_ChannelTransferStatusGet(DMAC_CHANNEL_0) == DMAC_TRANSFER_EVENT_ERROR) {
            printf("Main DMA Transfer Error\n");
            break;
        }
    }
    
    PORT_REGS->GROUP[0].PORT_PINCFG[16] &= ~PORT_PINCFG_PMUXEN_Msk;
    PORT_REGS->GROUP[0].PORT_OUTCLR = (1UL << 16);
    delay_us(200);
    
    __enable_irq();
}

void neopixel_send_data_manual(uint8_t *buffer, uint16_t length) {
    __disable_irq();
    
    SERCOM1_REGS->SPIM.SERCOM_CTRLA &= ~SERCOM_SPIM_CTRLA_ENABLE_Msk;
    while((SERCOM1_REGS->SPIM.SERCOM_SYNCBUSY) != 0U);
    
    PORT_REGS->GROUP[0].PORT_PINCFG[16] &= ~PORT_PINCFG_PMUXEN_Msk;
    PORT_REGS->GROUP[0].PORT_DIRSET = (1UL << 16);
    PORT_REGS->GROUP[0].PORT_OUTCLR = (1UL << 16);
    
    delay_us(100);
    
    PORT_REGS->GROUP[0].PORT_OUTSET = (1UL << 16);
    delay_us(10);
    
    PORT_REGS->GROUP[0].PORT_PINCFG[16] |= PORT_PINCFG_PMUXEN_Msk;
    SERCOM1_REGS->SPIM.SERCOM_CTRLA |= SERCOM_SPIM_CTRLA_ENABLE_Msk;
    while((SERCOM1_REGS->SPIM.SERCOM_SYNCBUSY) != 0U);
    
    for(uint16_t i = 0; i < length; i++) {
        while((SERCOM1_REGS->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_DRE_Msk) == 0U);
        SERCOM1_REGS->SPIM.SERCOM_DATA = buffer[i];
    }
    while((SERCOM1_REGS->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_TXC_Msk) == 0U);
    
    PORT_REGS->GROUP[0].PORT_PINCFG[16] &= ~PORT_PINCFG_PMUXEN_Msk;
    PORT_REGS->GROUP[0].PORT_OUTCLR = (1UL << 16);
    delay_us(100);
    
    __enable_irq();
}

void test_all_color(uint8_t red, uint8_t green, uint8_t blue) {
    clear_all_leds();
    for(int i = 0; i < NUM_LEDS; i++) {
        set_led_color(i, red, green, blue);
    }
    debug_led_buffer();
    neopixel_send_data_manual(neopixel_buffer, NEOPIXEL_BUFFER_SIZE);
}

void test_moving_rainbow(uint8_t offset) {
    clear_all_leds();
    for(int i = 0; i < NUM_LEDS; i++) {
        uint8_t pos = (i + offset) % 8;
        switch(pos) {
            case 0: set_led_color(i, 255, 0, 0); break;    // Red
            case 1: set_led_color(i, 255, 127, 0); break;  // Orange
            case 2: set_led_color(i, 255, 255, 0); break;  // Yellow
            case 3: set_led_color(i, 0, 255, 0); break;    // Green
            case 4: set_led_color(i, 0, 255, 255); break;  // Cyan
            case 5: set_led_color(i, 0, 0, 255); break;    // Blue
            case 6: set_led_color(i, 127, 0, 255); break;  // Violet
            case 7: set_led_color(i, 255, 0, 255); break;  // Magenta
            default: set_led_color(i, 0, 0, 0); break;
        }
    }
    if(offset == 0) {
        debug_led_buffer();
    }
    neopixel_send_data_manual(neopixel_buffer, NEOPIXEL_BUFFER_SIZE);
}