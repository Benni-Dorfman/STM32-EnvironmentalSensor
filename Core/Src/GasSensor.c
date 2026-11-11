#include <stdio.h>
#include <string.h>
#include "GasSensor.h"

// Global buffers and variables
uint32_t mq2_raw = 0;
float gas_percent = 0.0f;
char Gas[32] = {0};

void Read_GasLevel(void)
{
	HAL_ADC_Stop(&hadc1);
    ADC_ChannelConfTypeDef sConfig = {0};

    // 1. Switch ADC1 to Channel 1 (PA1)
    sConfig.Channel = ADC_CHANNEL_1;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
            return;
        }
    // 2. Sample the Gas Sensor
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
        {
            uint32_t raw_adc = HAL_ADC_GetValue(&hadc1);

            // Defines clean air baseline
            uint32_t clean_air_baseline = 800;

            if (raw_adc <= clean_air_baseline) {
                gas_percent = 0.0f; // Clamp to zero to prevent negative numbers
            } else {
                // Map only the remaining dangerous voltage range to 100%
                gas_percent = ((float)(raw_adc - clean_air_baseline) / (4095.0f - clean_air_baseline)) * 100.0f;
            }

            snprintf(Gas, sizeof(Gas), "%.1f %%", gas_percent);
        }

    HAL_ADC_Stop(&hadc1);
}
