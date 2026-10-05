#ifndef ACTUATOR_H
#define ACTUATOR_H

#include <stdbool.h>
#include "timing.h"

// Actuator Pin Definitions (Assigned to Group 0 / PORTA)
#define PIN_ACT_UP   PORT_PA20
#define PIN_ACT_DOWN PORT_PA21

// Timing Constants
#define MS_MIN_START  (MS_PER_SECOND * 25UL)
#define MS_MAX_START  (MS_PER_SECOND * 60UL)
#define MS_SLAM_LONG  500UL
#define MS_SLAM_SHORT 250UL
#define MAX_DROP_MS   (MS_PER_SECOND * 15UL)
#define MIN_DROP_MS   (MS_PER_SECOND * 5UL)
#define SLAM_MAX      5UL
#define MS_RESET_DOWN (MS_PER_SECOND * 3UL)  // must exceed full stroke time, with margin for low air pressure
#define MS_REVERSE_PAUSE 100UL               // let the relay and valve coil release before energizing the other side

// Public Function Prototypes
void Actuator_InitPorts(void);
void Actuator_Task(void *pvParameters);
bool Actuator_IsActive(void);   // true while a sequence is running (drives LED effect)

#endif /* ACTUATOR_H */