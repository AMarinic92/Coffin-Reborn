/* ************************************************************************** */
/** D-SUN Proximity Sensor Library

  @Company
    Microchip Technology Inc.

  @File Name
    dsun_sensor.h

  @Summary
    D-SUN ultrasonic proximity sensor control library.

  @Description
    This library provides functions to interface with D-SUN 3-pin ultrasonic
    proximity sensors using digital GPIO input on SAMD21/SAME50 microcontrollers.
    Sensors output HIGH when object is detected within adjustable range.
 */
/* ************************************************************************** */

#ifndef _DSUN_SENSOR_H    /* Guard against multiple inclusion */
#define _DSUN_SENSOR_H

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
    /** Sensor Pin Definition

      @Summary
        GPIO pin used for D-SUN sensor OUT signal.
    
      @Description
        The D-SUN sensor OUT pin is connected to PA19 on the microcontroller.
        This pin reads HIGH when an object is detected within the sensor's range.
    
      @Remarks
        Pin must be configured as digital input with pull-down resistor in MCC.
     */
#define DSUN_SENSOR_PIN     PORT_PIN_PA19

    /* ************************************************************************** */
    /** Detection Debounce Time

      @Summary
        Minimum time in milliseconds between valid detections.
    
      @Description
        Prevents false triggering due to sensor noise or rapid state changes.
        Adjust this value based on your application needs.
     */
#define DSUN_DEBOUNCE_MS    50

    // *****************************************************************************
    // *****************************************************************************
    // Section: Data Types
    // *****************************************************************************
    // *****************************************************************************

    // *****************************************************************************

    /** Sensor State Enumeration

      @Summary
        Enumeration of possible sensor detection states.
    
      @Description
        Represents the current state of object detection by the D-SUN sensor.

      @Remarks
        Used for state tracking and debouncing logic.
     */
    typedef enum {
        DSUN_NO_OBJECT = 0,     /* No object detected */
        DSUN_OBJECT_DETECTED,   /* Object detected within range */
        DSUN_STATE_UNKNOWN      /* Initial/undefined state */
    } dsun_state_t;

    // *****************************************************************************
    // *****************************************************************************
    // Section: Interface Functions
    // *****************************************************************************
    // *****************************************************************************

    // *****************************************************************************
    /**
      @Function
        void dsun_sensor_init(void)

      @Summary
        Initialize the D-SUN proximity sensor system.

      @Description
        Sets up the sensor state tracking and any required timers for debouncing.
        Must be called once before using other sensor functions.

      @Precondition
        PA19 must be configured as digital input with pull-down in MCC.

      @Parameters
        None.

      @Returns
        None.

      @Remarks
        This function initializes internal state variables but does not
        configure the GPIO pin - that should be done in MCC.

      @Example
        @code
        // After MCC configuration and SYS_Initialize()
        dsun_sensor_init();
        @endcode
     */
    void dsun_sensor_init(void);

    // *****************************************************************************
    /**
      @Function
        bool dsun_read_raw(void)

      @Summary
        Read the raw digital state of the D-SUN sensor.

      @Description
        Returns the immediate digital state of the sensor OUT pin.
        True indicates object detected, false indicates no object.
        No debouncing is applied.

      @Precondition
        dsun_sensor_init() must have been called.

      @Parameters
        None.

      @Returns
        @retval true Object detected (sensor OUT pin is HIGH)
        @retval false No object detected (sensor OUT pin is LOW)

      @Remarks
        This function provides immediate sensor reading without filtering.
        For reliable detection, use dsun_object_detected() instead.

      @Example
        @code
        if(dsun_read_raw()) {
            // Object immediately detected
        }
        @endcode
     */
    bool dsun_read_raw(void);

    // *****************************************************************************
    /**
      @Function
        dsun_state_t dsun_get_state(void)

      @Summary
        Get the current debounced state of the D-SUN sensor.

      @Description
        Returns the current sensor state after debounce filtering.
        Call this function regularly (in main loop) to update state tracking.

      @Precondition
        dsun_sensor_init() must have been called.

      @Parameters
        None.

      @Returns
        Current sensor state (DSUN_NO_OBJECT or DSUN_OBJECT_DETECTED)

      @Remarks
        This function includes debounce logic to prevent false triggering.
        Should be called regularly for proper state tracking.

      @Example
        @code
        dsun_state_t state = dsun_get_state();
        if(state == DSUN_OBJECT_DETECTED) {
            // Reliable object detection
        }
        @endcode
     */
    dsun_state_t dsun_get_state(void);

    // *****************************************************************************
    /**
      @Function
        bool dsun_object_detected(void)

      @Summary
        Check if an object is currently detected (debounced).

      @Description
        Convenience function that returns true if an object is reliably detected.
        Equivalent to checking if dsun_get_state() == DSUN_OBJECT_DETECTED.

      @Precondition
        dsun_sensor_init() must have been called.

      @Parameters
        None.

      @Returns
        @retval true Object is detected (after debouncing)
        @retval false No object detected

      @Example
        @code
        if(dsun_object_detected()) {
            test_flame(counter);  // Trigger LED pattern
        }
        @endcode
     */
    bool dsun_object_detected(void);

    // *****************************************************************************
    /**
      @Function
        bool dsun_object_just_detected(void)

      @Summary
        Check if an object was just detected (rising edge detection).

      @Description
        Returns true only on the first detection after no object was present.
        Useful for triggering actions only once per detection event.

      @Precondition
        dsun_sensor_init() must have been called.

      @Parameters
        None.

      @Returns
        @retval true Object just became detected (rising edge)
        @retval false Object was already detected or not detected

      @Remarks
        This function detects state transitions from no-object to object-detected.
        Perfect for triggering one-shot events like LED animations.

      @Example
        @code
        if(dsun_object_just_detected()) {
            counter = 0;  // Reset animation counter
            // Start new LED pattern
        }
        @endcode
     */
    bool dsun_object_just_detected(void);

    // *****************************************************************************
    /**
      @Function
        bool dsun_object_just_lost(void)

      @Summary
        Check if an object was just lost (falling edge detection).

      @Description
        Returns true only when object detection ends after being detected.
        Useful for cleanup actions when object moves away.

      @Parameters
        None.

      @Returns
        @retval true Object just became undetected (falling edge)
        @retval false Object was already undetected or still detected

      @Example
        @code
        if(dsun_object_just_lost()) {
            clear_all_leds();  // Turn off LEDs when object moves away
        }
        @endcode
     */
    bool dsun_object_just_lost(void);

/* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _DSUN_SENSOR_H */

/* *****************************************************************************
 End of File
 */