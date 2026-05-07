#ifndef ACTUATOR_H
#define ACTUATOR_H

#include <stdint.h>
#include <stdbool.h>

// Actuator Pin Definitions (Assigned to Group 0 / PORTA)
#define PIN_ACT_UP   PORT_PA20
#define PIN_ACT_DOWN PORT_PA21

// Timing Constants
#define MS_PER_SECOND 1000UL
#define MS_PER_MIN    (MS_PER_SECOND * 60UL)
#define MS_MIN_START  (MS_PER_SECOND * 25UL)
#define MS_MAX_START  (MS_PER_SECOND * 60UL)
#define MS_SLAM_LONG  500UL
#define MS_SLAM_SHORT 250UL
#define MAX_DROP_MS   (MS_PER_SECOND * 15UL)
#define MIN_DROP_MS   (MS_PER_SECOND * 5UL)
#define SLAM_MAX      5UL

// Public Function Prototypes
void Actuator_InitPorts(void);
void Actuator_Task(void *pvParameters);

#endif /* ACTUATOR_H */