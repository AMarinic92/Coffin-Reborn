#include "actuator.h"
#include <sam.h>
#include "FreeRTOS.h"
#include "definitions.h"
#include "task.h"
#include <stdio.h>
// If you completely removed the dcc_stdio from your build, remove this include.
// Otherwise, keep it for your debug prints.
// #include "dcc_stdio.h" 

// Internal State
static volatile bool is_up = false;
static volatile bool is_down = false;

// Internal Helpers
static uint32_t interpolateNum(uint32_t min, uint32_t max, uint32_t number);
static void act_off(void);
static void act_up(void);
static void act_down(void);
static void act_reset(void);

// Actuator Sequences
static void act_random_drop(void);
static void act_quick_up(void);
static void act_violent(void);

void Actuator_InitPorts(void)
{
    // Set PA20 and PA21 as outputs
    PORT_REGS->GROUP[0].PORT_DIRSET = PIN_ACT_UP | PIN_ACT_DOWN;
    // Ensure they start off
    act_off();
}





// ---------------------------------------------------------
// Core Relay Control (Interlocked for safety)
// ---------------------------------------------------------
static void act_up(void)
{
    PORT_REGS->GROUP[0].PORT_OUTCLR = PIN_ACT_DOWN; // Ensure down is off
    is_down = false;
    PORT_REGS->GROUP[0].PORT_OUTSET = PIN_ACT_UP;   // Turn up on
    is_up = true;
}

static void act_down(void)
{
    PORT_REGS->GROUP[0].PORT_OUTCLR = PIN_ACT_UP;   // Ensure up is off
    is_up = false;
    PORT_REGS->GROUP[0].PORT_OUTSET = PIN_ACT_DOWN; // Turn down on
    is_down = true;
}

static void act_off(void)
{
    PORT_REGS->GROUP[0].PORT_OUTCLR = PIN_ACT_UP;
    PORT_REGS->GROUP[0].PORT_OUTCLR = PIN_ACT_DOWN;
    is_up = false;
    is_down = false;
}

static void act_reset(void)
{
    act_down();
    vTaskDelay(pdMS_TO_TICKS(MS_PER_SECOND));
    act_off();
}

// ---------------------------------------------------------
// Sequences
// ---------------------------------------------------------
static void act_violent(void)
{
    act_up();
    vTaskDelay(pdMS_TO_TICKS(MS_PER_SECOND));

    for (int count = 0; count < SLAM_MAX; count++)
    {
        uint32_t timeMod = (count % 2 == 0) ? MS_SLAM_LONG : MS_SLAM_SHORT;
        
        act_down();
        vTaskDelay(pdMS_TO_TICKS(timeMod));
        
        act_up();
        vTaskDelay(pdMS_TO_TICKS(timeMod));
    }
    act_reset();
}

static void act_random_drop(void)
{
    // Grab a true random number from the hardware
    uint32_t randomNumber = TRNG_ReadData();
    
    act_up();
    vTaskDelay(pdMS_TO_TICKS(interpolateNum(MIN_DROP_MS, MAX_DROP_MS, randomNumber)));
    act_reset();
}

static void act_quick_up(void)
{
    act_up();
    vTaskDelay(pdMS_TO_TICKS(MS_PER_SECOND));
    act_reset();
}

static uint32_t interpolateNum(uint32_t min, uint32_t max, uint32_t number)
{
    uint32_t out = number;
    if (out > max) {
        while (out > max) out = out >> 1;
        if (out < min) out = min;
    } else if (out < min) {
        while (out < min) out = out << 1;
        if (out > max) out = max;
    }
    return out;
}

// ---------------------------------------------------------
// Main RTOS Task
// ---------------------------------------------------------
void Actuator_Task(void *pvParameters)
{
    uint32_t act_index;
    uint32_t randomNumber;

    // Initial boot delay (15 seconds)
    vTaskDelay(pdMS_TO_TICKS(MS_PER_SECOND * 15));

    while (1)
    {
        // 1. Pick a sequence using the hardware TRNG
        act_index = TRNG_ReadData();
        #ifndef NDEBUG
            printf("Actuator Sequence done start\n");
        #endif
        

        if (act_index < 0x7FFFFFFFUL) act_index = 1;
        else if (act_index < 0XD5555554UL) act_index = 0;
        else act_index = 2;


        // 2. Execute sequence (These functions block internally via vTaskDelay)
        switch(act_index) {
            case 0: act_random_drop(); break;
            case 1: act_quick_up();    break;
            case 2: act_violent();     break;
            default: act_reset();      break;
        }

        // 3. Calculate next event time using the hardware TRNG
        randomNumber = TRNG_ReadData();
        randomNumber = interpolateNum(MS_MIN_START, MS_MAX_START, randomNumber);
        #ifndef NDEBUG
            printf("Actuator Sequence done next %lu ms\n", randomNumber);
        #endif
        
        // 4. Sleep the task until the next event
        vTaskDelay(pdMS_TO_TICKS(randomNumber));
    }
}