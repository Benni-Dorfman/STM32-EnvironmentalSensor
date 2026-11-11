#ifndef INC_DHT11_H_
#define INC_DHT11_H_

#include "stm32f4xx_hal.h"

extern char Humidity[32];

typedef struct{
    GPIO_TypeDef* port;
    uint16_t pin;
    TIM_HandleTypeDef* htim; // Kept for compatibility, though DWT replaces it
    uint8_t temperature;
    uint8_t humidity;
} DHT11_HandleTypeDef;

// DWT cycle counter functions for microsecond delay
void DWT_Init(void);
void delay_us(uint32_t us);

void DHT11_Init(DHT11_HandleTypeDef *dht, TIM_HandleTypeDef *htim, GPIO_TypeDef* port, uint16_t pin);
uint8_t readDHT11(DHT11_HandleTypeDef *dht);
void Read_Humidity(DHT11_HandleTypeDef *dht);

#endif /* INC_DHT11_H_ */
