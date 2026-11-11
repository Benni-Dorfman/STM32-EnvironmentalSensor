#include "bluetooth.h"
#include <string.h>

// Pointer to the hardware UART channel
static UART_HandleTypeDef *bt_uart_handle = NULL;
// Temporary variable used by the interrupt to store the single byte that just arrived
static uint8_t rx_byte = 0;

// Circular FIFO ring buffer to prevent losing bytes during multi-character strings
static volatile uint8_t rx_fifo[128];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;

// String assembly buffers
static char command_line[64] = {0};
static char rx_temp[64] = {0};
static uint16_t rx_idx = 0;
static uint32_t last_char_time = 0;

// Initializes the Bluetooth module and arms the reception interrupt
void Bluetooth_Init(UART_HandleTypeDef *huart)
{
    bt_uart_handle = huart;
    rx_head = 0;
    rx_tail = 0;
    rx_idx = 0;

    if (bt_uart_handle != NULL)
    {
    	// Tells the STM32 to listen for  1 byte.
    	HAL_UART_Receive_IT(bt_uart_handle, &rx_byte, 1);
    }
}

// Transmits a string
void Bluetooth_SendString(const char *str)
{
    if (bt_uart_handle != NULL && str != NULL)
    {
    	// Sends the string. 100ms timeout prevents the system from locking up
        HAL_UART_Transmit(bt_uart_handle, (uint8_t *)str, (uint16_t)strlen(str), 100);
    }
}

// Pulls a single byte out of the FIFO ring buffer if one is available
bool Bluetooth_ReadByte(uint8_t *received_char)
{
    // Check if there are unread bytes in the FIFO ring buffer
    if (rx_head != rx_tail)
    {
        *received_char = rx_fifo[rx_tail];
        rx_tail = (rx_tail + 1) % 128;
        return true;
    }
    return false;
}

// Official HAL interrupt callback override
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (bt_uart_handle != NULL && huart->Instance == bt_uart_handle->Instance)
    {
        // Push incoming byte into the FIFO ring buffer
        uint16_t next_head = (rx_head + 1) % 128;
        // Only insert if buffer not full
        if (next_head != rx_tail)
        {
            rx_fifo[rx_head] = rx_byte;
            rx_head = next_head;
        }

        // Re-arms interrupt for the next character
        HAL_UART_Receive_IT(bt_uart_handle, &rx_byte, 1);
    }
}

// Error callback to unlock the receiver if an overrun (ORE) occurs
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (bt_uart_handle != NULL && huart->Instance == bt_uart_handle->Instance)
    {
        __HAL_UART_CLEAR_OREFLAG(huart);
        __HAL_UART_CLEAR_NEFLAG(huart);
        __HAL_UART_CLEAR_FEFLAG(huart);
        HAL_UART_Receive_IT(bt_uart_handle, &rx_byte, 1);
    }
}

// Assembles bytes from the buffer into strings.
char* Bluetooth_GetString(void)
{
    uint8_t character = 0;

    while (Bluetooth_ReadByte(&character))
    {
    	// Update timestamp on arrival
        last_char_time = HAL_GetTick();

        // Checks if line ending reached
        if (character == '\r' || character == '\n')
        {
        	// Ignores blank lines
            if (rx_idx == 0) continue;

            // Finishes the string with a null terminator
            rx_temp[rx_idx] = '\0';
            // Copys the  string to the final output buffer
            strncpy(command_line, rx_temp, sizeof(command_line) - 1);
            command_line[sizeof(command_line) - 1] = '\0';
            // Resets index so the next incoming message starts at the beginning
            rx_idx = 0;

            return command_line;
        }
        else // It's a normal letter or number
        {
        	// Adds to the buffer as long within the 64-character limit
            if (rx_idx < sizeof(rx_temp) - 1)
            {
                rx_temp[rx_idx++] = (char)character;
            }
            else
            {
            	// Buffer overflow protection: reset
                rx_idx = 0;
            }
        }
    }

    // Timeout fallback: if characters arrived but app didn't send \r or \n
    if (rx_idx > 0 && (HAL_GetTick() - last_char_time > 100))
    {
        rx_temp[rx_idx] = '\0';
        strncpy(command_line, rx_temp, sizeof(command_line) - 1);
        command_line[sizeof(command_line) - 1] = '\0';
        rx_idx = 0;

        return command_line;
    }
    // No complete string is ready yet
    return NULL;
}

// Copyies the string into final variable.
bool Bluetooth_ReadString(char *out_str, uint16_t max_len)
{
    char *str = Bluetooth_GetString();
    if (str != NULL)
    {
        strncpy(out_str, str, max_len - 1);
        out_str[max_len - 1] = '\0';
        return true;
    }
    return false;
}
