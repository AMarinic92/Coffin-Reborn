#include "neopixel.h"
#include "definitions.h"
#include <stdint.h>
#include <stdbool.h>

// Buffer to hold TCC compare values
uint8_t neopixel_tcc_buffer[144 * 24];

// TCC compare values for SK6812 timing
#define TCC_ZERO_BIT  36   // ~300ns high
#define TCC_ONE_BIT   72   // ~600ns high

// DMA complete flag
volatile bool neopixel_dma_complete = false;

// DMA Callback
void NeoPixel_DMA_Callback(DMAC_TRANSFER_EVENT event, uintptr_t context) {
    if(event == DMAC_TRANSFER_EVENT_COMPLETE) {
        neopixel_dma_complete = true;
    }
}

// Initialize neopixel system
void neopixel_init(void) {
    // Register DMA callback
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, NeoPixel_DMA_Callback, 0);
    
    // Disable pattern generator (if you had issues with it)
    TCC0_REGS->TCC_PATT = 0;
    TCC0_REGS->TCC_PATTBUF = 0;
}

void set_led_color_tcc(uint8_t led_index, uint8_t red, uint8_t green, uint8_t blue) {
    if(led_index >= 144) return;
    
    // Calculate bit position in buffer (G-R-B order for SK6812)
    uint16_t bit_offset = led_index * 24;
    
    // Green (8 bits, MSB first)
    for(int i = 7; i >= 0; i--) {
        neopixel_tcc_buffer[bit_offset++] = (green & (1 << i)) ? TCC_ONE_BIT : TCC_ZERO_BIT;
    }
    
    // Red (8 bits, MSB first)
    for(int i = 7; i >= 0; i--) {
        neopixel_tcc_buffer[bit_offset++] = (red & (1 << i)) ? TCC_ONE_BIT : TCC_ZERO_BIT;
    }
    
    // Blue (8 bits, MSB first)
    for(int i = 7; i >= 0; i--) {
        neopixel_tcc_buffer[bit_offset++] = (blue & (1 << i)) ? TCC_ONE_BIT : TCC_ZERO_BIT;
    }
}

void neopixel_send_tcc(void) {
    // Set initial compare value
    TCC0_REGS->TCC_CC[0] = neopixel_tcc_buffer[0];
    
    // Start DMA transfer
    neopixel_dma_complete = false;
    DMAC_ChannelTransfer(
        DMAC_CHANNEL_0,
        neopixel_tcc_buffer,
        (const void*)&TCC0_REGS->TCC_CCBUF[0],  // Buffered register
        144 * 24
    );
    
    
    // Start TCC - DMA will feed data on each overflow
    TCC0_PWMStart();
    
//     Wait for completion
    while(!neopixel_dma_complete){
        TCC0_PWMForceUpdate();
    }
    // Stop TCC
    TCC0_PWMStop();
    
    // Reset pulse (>80µs for SK6812)
    SYSTICK_DelayUs(100);
}