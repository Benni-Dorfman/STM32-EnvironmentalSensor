#ifndef INC_GASSENSOR_H_
#define INC_GASSENSOR_H_

#include "main.h" // Provides ADC_HandleTypeDef, uint32_t, and HAL function prototypes

// Global variables
extern ADC_HandleTypeDef hadc1;
extern uint32_t mq2_raw;
extern float gas_percent;
extern char Gas[32];

void Read_GasLevel(void);

#endif
