#ifndef _SYSTEMCONTROLS_H
#define _SYSTEMCONTROLS_H

#include "main.h"

// GLOBAL ASYNCHRONOUS FLAGS
extern uint16_t choice;
extern uint32_t sensor_timer;
extern uint32_t LEDTimer;
extern uint32_t system_timer;
extern uint16_t pause;
extern uint32_t alarm_flash_timer;
extern uint8_t alarm_active;


// FUNCTION PROTOTYPES
uint8_t CheckDelay(volatile uint32_t *start_time, uint32_t delay_ms);
uint8_t Input(const char *str);
void Menu(void);
void CheckAlarms(void);
void System_Init(void);
#endif
