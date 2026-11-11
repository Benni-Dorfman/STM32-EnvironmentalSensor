#include <string.h>
#include <stdio.h>
#include "AmbientLight.h"

// Global variables accessed by SystemControls.c
float lux = 0.0f;
char Ambiance[32] = {0};

void Read_TEMT6000(void)
{
    // 1. MUST stop the ADC before switching channels to prevent silent failures
    HAL_ADC_Stop(&hadc1);

    ADC_ChannelConfTypeDef sConfig = {0};

    // 2. Configure for Channel 11 (PC1) with a long sample time to prevent voltage bleed
    sConfig.Channel = ADC_CHANNEL_11;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        return;
    }

    uint32_t adc_accum = 0;

    // 3. Take 11 samples (discard 1st, keep 10) spread over 20ms to absorb AC light flicker
    for (int i = 0; i < 11; i++) {
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
            if (i == 0) {
                // Discard the very first reading to clear the MQ-2 ghost voltage
                HAL_ADC_GetValue(&hadc1);
            } else {
                // Keep the 10 real readings
                adc_accum += HAL_ADC_GetValue(&hadc1);
            }
        }
        HAL_ADC_Stop(&hadc1);

        // 2ms blocking delay * 10 samples = 20ms total delay
        HAL_Delay(2);
    }

    // 4. Calculate the stable average
    uint32_t avg_adc = adc_accum / 10;

        // 5. Calibrate your indoor lighting range
        uint32_t dark_baseline = 50;   // The ADC reading when covered
        uint32_t indoor_max = 3000;      // The ADC reading you want to represent 100%

        if (avg_adc <= dark_baseline) {
            lux = 0.0f;
        } else {
            // Map the realistic indoor range (150 -> 600) to (0.0% -> 100.0%)
            lux = (((float)(avg_adc - dark_baseline) / (float)(indoor_max - dark_baseline)) * 100.0f);

            // If someone shines a flashlight on it, cap it at 100%
            if (lux > 100.0f) {
                lux = 100.0f;
            }
        }

        // 6. Format the final string for the OLED and Bluetooth
        snprintf(Ambiance, sizeof(Ambiance), "%.1f %%", lux);
}
