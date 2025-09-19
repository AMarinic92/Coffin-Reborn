/* ************************************************************************** */
/** D-SUN Proximity Sensor Library

  @Company
    Microchip Technology Inc.

  @File Name
    dsun_sensor.c

  @Summary
    D-SUN ultrasonic proximity sensor implementation.

  @Description
    Implementation of D-SUN proximity sensor functions with debouncing
    and edge detection for reliable object detection on SAMD21/SAME50.
 */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include "dsun_sensor.h"
#include "definitions.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

/* ************************************************************************** */
/** Sensor State Tracking Variables

  @Summary
    Internal variables for tracking sensor state and debouncing.
    
  @Description
    These variables maintain the sensor's current and previous states
    for debouncing and edge detection functionality.
    
  @Remarks
    All variables are static (file scope) and managed internally.
 */
static dsun_state_t current_state = DSUN_STATE_UNKNOWN;
static dsun_state_t previous_state = DSUN_STATE_UNKNOWN;
static uint32_t last_change_time = 0;
static bool sensor_initialized = false;

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

/** 
  @Function
    uint32_t get_system_time_ms(void)

  @Summary
    Get current system time in milliseconds.

  @Description
    Simple millisecond counter for debounce timing. This is a placeholder
    implementation - replace with actual system tick counter if available.

  @Returns
    Current time in milliseconds (approximate)

  @Remarks
    This is a simple implementation. For more accurate timing, integrate
    with your system's tick timer or use SysTick counter.
 */
static uint32_t get_system_time_ms(void) {
    // Simple approximation - replace with actual system timer
    // This assumes your main loop runs frequently enough for reasonable timing
    static uint32_t time_counter = 0;
    time_counter++;
    return time_counter / 1000; // Very rough approximation
}

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

// *****************************************************************************
/** 
  @Function
    void dsun_sensor_init(void)

  @Summary
    Initialize the D-SUN proximity sensor system.

  @Remarks
    Refer to the dsun_sensor.h interface header for function usage details.
 */
void dsun_sensor_init(void) {
    current_state = DSUN_STATE_UNKNOWN;
    previous_state = DSUN_STATE_UNKNOWN;
    last_change_time = 0;
    sensor_initialized = true;
    
    // Read initial state
    dsun_get_state();
}

// *****************************************************************************
/** 
  @Function
    bool dsun_read_raw(void)

  @Summary
    Read the raw digital state of the D-SUN sensor.

  @Remarks
    Refer to the dsun_sensor.h interface header for function usage details.
 */
bool dsun_read_raw(void) {
    // Read PA19 digital input
    return PORT_PinRead(DSUN_SENSOR_PIN);
}

// *****************************************************************************
/** 
  @Function
    dsun_state_t dsun_get_state(void)

  @Summary
    Get the current debounced state of the D-SUN sensor.

  @Remarks
    Refer to the dsun_sensor.h interface header for function usage details.
 */
dsun_state_t dsun_get_state(void) {
    if (!sensor_initialized) {
        return DSUN_STATE_UNKNOWN;
    }
    
    // Read raw sensor state
    bool raw_reading = dsun_read_raw();
    dsun_state_t new_state = raw_reading ? DSUN_OBJECT_DETECTED : DSUN_NO_OBJECT;
    
    // Get current time
    uint32_t current_time = get_system_time_ms();
    
    // Check if state has changed
    if (new_state != current_state) {
        // State changed - check if enough time has passed for debouncing
        if (current_time - last_change_time >= DSUN_DEBOUNCE_MS) {
            // Update states
            previous_state = current_state;
            current_state = new_state;
            last_change_time = current_time;
        }
        // If not enough time has passed, keep current state
    } else {
        // State hasn't changed - update time to prevent timeout issues
        last_change_time = current_time;
    }
    
    return current_state;
}

// *****************************************************************************
/** 
  @Function
    bool dsun_object_detected(void)

  @Summary
    Check if an object is currently detected (debounced).

  @Remarks
    Refer to the dsun_sensor.h interface header for function usage details.
 */
bool dsun_object_detected(void) {
    return (dsun_get_state() == DSUN_OBJECT_DETECTED);
}

// *****************************************************************************
/** 
  @Function
    bool dsun_object_just_detected(void)

  @Summary
    Check if an object was just detected (rising edge detection).

  @Remarks
    Refer to the dsun_sensor.h interface header for function usage details.
 */
bool dsun_object_just_detected(void) {
    dsun_state_t current = dsun_get_state();
    
    // Check for rising edge: previous was no object, current is object detected
    bool rising_edge = (previous_state == DSUN_NO_OBJECT) && 
                      (current == DSUN_OBJECT_DETECTED);
    
    return rising_edge;
}

// *****************************************************************************
/** 
  @Function
    bool dsun_object_just_lost(void)

  @Summary
    Check if an object was just lost (falling edge detection).

  @Remarks
    Refer to the dsun_sensor.h interface header for function usage details.
 */
bool dsun_object_just_lost(void) {
    dsun_state_t current = dsun_get_state();
    
    // Check for falling edge: previous was object detected, current is no object
    bool falling_edge = (previous_state == DSUN_OBJECT_DETECTED) && 
                       (current == DSUN_NO_OBJECT);
    
    return falling_edge;
}

/* *****************************************************************************
 End of File
 */