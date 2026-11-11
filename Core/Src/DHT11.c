#include "DHT11.h"
#include <stdio.h>

char Humidity[32] = {0};

// Initialize the DWT Cycle Counter
void DWT_Init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

// Precise microsecond delay using CPU cycles
void delay_us(uint32_t us) {
    uint32_t startTicks = DWT->CYCCNT;
    uint32_t targetTicks = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - startTicks) < targetTicks);
}

void DHT11_Init(DHT11_HandleTypeDef *dht, TIM_HandleTypeDef *htim, GPIO_TypeDef* port, uint16_t pin) {
    dht->htim = htim;
    dht->port = port;
    dht->pin = pin;
}

uint8_t readDHT11(DHT11_HandleTypeDef *dht) {
    uint8_t data[5] = {0};

    // 1. Send Start Signal (Assumes pin is set to GPIO_MODE_OUTPUT_OD in main.c)
    HAL_GPIO_WritePin(dht->port, dht->pin, GPIO_PIN_RESET); // Pull low
    HAL_Delay(18);

    __disable_irq();

    // Release the line (Open Drain goes high-Z, pull-up brings it to 3.3V)
    HAL_GPIO_WritePin(dht->port, dht->pin, GPIO_PIN_SET);
    delay_us(30);

    // 2. Wait for sensor response (low then high)
    uint32_t timeout = 10000;
    while (HAL_GPIO_ReadPin(dht->port, dht->pin) == GPIO_PIN_SET) {
        if (--timeout == 0) { __enable_irq(); return 0; }
    }

    timeout = 10000;
    while (HAL_GPIO_ReadPin(dht->port, dht->pin) == GPIO_PIN_RESET) {
        if (--timeout == 0) { __enable_irq(); return 0; }
    }

    timeout = 10000;
    while (HAL_GPIO_ReadPin(dht->port, dht->pin) == GPIO_PIN_SET) {
        if (--timeout == 0) { __enable_irq(); return 0; }
    }

    // 3. Read 40 bits of data
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 8; j++) {
            timeout = 10000;
            while (HAL_GPIO_ReadPin(dht->port, dht->pin) == GPIO_PIN_RESET) {
                if (--timeout == 0) { __enable_irq(); return 0; }
            }

            delay_us(40);

            if (HAL_GPIO_ReadPin(dht->port, dht->pin) == GPIO_PIN_SET) {
                data[i] |= (1 << (7 - j));
                timeout = 10000;
                while (HAL_GPIO_ReadPin(dht->port, dht->pin) == GPIO_PIN_SET) {
                    if (--timeout == 0) { __enable_irq(); return 0; }
                }
            }
        }
    }

    __enable_irq();

    // 4. Verify Checksum
    if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        dht->humidity = data[0];
        dht->temperature = data[2];
        return 1;
    }

    return 0;
}

void Read_Humidity(DHT11_HandleTypeDef *dht) {
    if (readDHT11(dht)) {
        snprintf(Humidity, sizeof(Humidity), "%d%%", dht->humidity);
    } else {
        snprintf(Humidity, sizeof(Humidity), "Error");
    }
}
