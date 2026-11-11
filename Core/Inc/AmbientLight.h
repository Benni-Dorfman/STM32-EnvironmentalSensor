#ifndef INC_AMBIENTLIGHT_H_
#define INC_AMBIENTLIGHT_H_

#include "main.h" // Provides ADC_HandleTypeDef, uint32_t, and HAL function prototypes

// Expose the ADC handle from main.c
extern ADC_HandleTypeDef hadc1;

// Expose the lux variable to main.c
extern float lux;
extern char Ambiance[32];
// Function prototype
void Read_TEMT6000(void);

#endif /* INC_AMBIENTLIGHT_H_ */
