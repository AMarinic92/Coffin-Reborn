// ============= NEW NEOPIXEL TCC IMPLEMENTATION =============

// Buffer to hold TCC compare values (one byte per NeoPixel bit)
// For 144 LEDs × 24 bits = 3456 bytes

#include "neopixel.h"
#include "definitions.h"
#include <stdint.h>
#include <stdbool.h>

// Buffer to hold TCC compare values
uint8_t neopixel_tcc_buffer[144 * 24];

// TCC compare values
#define TCC_ZERO_BIT  36
#define TCC_ONE_BIT   72

// DMA complete flag
volatile bool neopixel_dma_complete = false;
// DMA Callback
void NeoPixel_DMA_Callback(DMAC_TRANSFER_EVENT event, uintptr_t context) {
    if(event == DMAC_TRANSFER_EVENT_COMPLETE) {
        neopixel_dma_complete = true;
    }
}

void set_led_color_tcc(uint8_t led_index, uint8_t red, uint8_t green, uint8_t blue) {
    if(led_index >= 144) return;
    
    // Calculate bit position in buffer (G-R-B order for SK6812)
    uint16_t bit_offset = led_index * 24;
    
    // Green (8 bits)
    for(int i = 7; i >= 0; i--) {
        neopixel_tcc_buffer[bit_offset++] = (green & (1 << i)) ? TCC_ONE_BIT : TCC_ZERO_BIT;
    }
    
    // Red (8 bits)
    for(int i = 7; i >= 0; i--) {
        neopixel_tcc_buffer[bit_offset++] = (red & (1 << i)) ? TCC_ONE_BIT : TCC_ZERO_BIT;
    }
    
    // Blue (8 bits)
    for(int i = 7; i >= 0; i--) {
        neopixel_tcc_buffer[bit_offset++] = (blue & (1 << i)) ? TCC_ONE_BIT : TCC_ZERO_BIT;
    }
}

void neopixel_init(void) {
    // Register the DMA callback for channel 0
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, NeoPixel_DMA_Callback, 0);
}


void neopixel_send_tcc(void) {
    // Start DMA transfer
    neopixel_dma_complete = false;
    DMAC_ChannelTransfer(
        DMAC_CHANNEL_0,
        neopixel_tcc_buffer,
        (const void*)&TCC0_REGS->TCC_CC[0],  // CC buffer register
        144 * 24  // Total bytes to send
    );
    
    // Start TCC - DMA will feed data automatically
    TCC0_PWMStart();
    
    // Wait for completion
    while(!neopixel_dma_complete);
    
    // Stop TCC
    TCC0_PWMStop();
    
    // Reset pulse (>80µs)
    SYSTICK_DelayUs(100);
}