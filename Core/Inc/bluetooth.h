#ifndef INC_BLUETOOTH_H_
#define INC_BLUETOOTH_H_



#include "main.h"
#include <stdbool.h>

//Initialize Bluetooth module driver with a designated UART peripheral.
void Bluetooth_Init(UART_HandleTypeDef *huart);

//Transmits a string over Bluetooth.
void Bluetooth_SendString(const char *str);

//Checks if a new byte has been received.
bool Bluetooth_ReadByte(uint8_t *received_char);

// Reads the string transmitted over bluetooth
bool  Bluetooth_ReadString(char *out_str, uint16_t max_len);
char* Bluetooth_GetString(void);

#endif /* INC_BLUETOOTH_H_ */
