#ifndef _OLED_H
#define _OLED_H

#include "main.h"
#include "ssd1306.h"
#include "fonts.h"

// 100ms = 10Hz refresh rate

// HARDWARE HANDLES
extern I2C_HandleTypeDef hi2c1;

// FUNCTION PROTOTYPES
void RefreshOLED(uint16_t input);
void PrintHumidity(const char *str1);
void PrintTemp(const char *str1);
void PrintLight(const char *str1);
void PrintGas(const char *str1);
void PrintPressure(const char *str1);
void LoadingBar(int16_t input);
void DrawBackdrop(int16_t input);

#endif
