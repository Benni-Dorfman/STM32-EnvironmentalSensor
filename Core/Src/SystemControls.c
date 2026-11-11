#include "SystemControls.h"
#include <string.h>
#include "AmbientLight.h"
#include "GasSensor.h"
#include "DHT11.h"
#include "OLED.h"
#include "bluetooth.h"
#include "main.h"
#include "bmp280.h"

// Sensor handles mapped to hardware peripherals
 BMP280_HandleTypedef bmp280;
 DHT11_HandleTypeDef dht11;

 // Hardware handles defined in main.c
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim10;
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c3;

// Global state variables for system navigation and timing
uint16_t choice = 1;
uint32_t sensor_timer = 0;
uint32_t LEDTimer = 0;
uint32_t system_timer = 0;
uint16_t pause = 0;
uint32_t alarm_flash_timer = 0;
uint8_t alarm_active = 0;

// Non-blocking delay to replace HAL_Delay()
uint8_t CheckDelay(volatile uint32_t *start_time, uint32_t delay_ms) {
    if (*start_time == 0) {
        *start_time = HAL_GetTick();
        return 0;
    }

    if (HAL_GetTick() - *start_time >= delay_ms) {
    	*start_time = HAL_GetTick();
        return 1;
    }

    return 0;
}

// Maps incoming Bluetooth strings to menu screen numbers
uint8_t Input(const char *str)
{
    if (str == NULL) {
        return 0;
    }
    if (strcmp(str, "Ambiance") == 0) {
        return 1;
    }
    else if (strcmp(str, "Temperature") == 0) {
        return 2;
    }
    else if (strcmp(str, "Gas") == 0) {
        return 3;
    }
    else if (strcmp(str, "Humidity") == 0) {
        return 4;
    }
    else if (strcmp(str, "Pressure") == 0) {
        return 5;
    }
    else if (strcmp(str, "Pause") == 0) {
    	pause = 1;
    	// Remain on current screen
        return choice;
    }
    else if (strcmp(str, "Unpause") == 0) {
    	pause = 0;
    	// Remain on current screen
        return choice;
    }

    return 0;
}

// Menu loop
void Menu(void) {
	// 1. Evaluate safety thresholds immediately
	CheckAlarms();

	// 2. Fetches pending Bluetooth commands from the background buffer
	char *my_cmd = Bluetooth_GetString();

	// 3. Staggered sensor polling
	if (CheckDelay(&sensor_timer, 500) == 1) {
	        static uint8_t sensor_toggle = 0; // Remembers its state between loops

	        if (sensor_toggle == 0) {
	            Read_TEMT6000();
	            sensor_toggle = 1;
	        } else {
	            Read_GasLevel();
	            Read_BMP280();
		        Read_Humidity(&dht11);

	            sensor_toggle = 0;
	        }
	    }

	// Limits OLED refresh
	static uint32_t oled_timer = 0;
	    if (CheckDelay(&oled_timer, 200) == 1) {
	        RefreshOLED(choice);
	    }	// Check if the mobile app transmitted a command
	if (my_cmd != NULL && strlen(my_cmd) > 0)
	{
		uint8_t new_choice = Input(my_cmd);
		if (new_choice != 0) {
			choice = new_choice;
			system_timer = 0;
			system_timer = 0;
	    if (strcmp(my_cmd, "Pause") != 0) {

	    	// Transmit requested data back to the phone based on input
	    switch (choice)
	    {
	        case 1:
	            Bluetooth_SendString("\r\n");
	            Bluetooth_SendString("Ambiance level is: ");
	            Bluetooth_SendString(Ambiance);
	            break;

	        case 2:
	            Bluetooth_SendString("\r\n");
	            Bluetooth_SendString("Temperature level is: ");
	            Bluetooth_SendString(Temperature);
	            break;

	        case 3:
	            Bluetooth_SendString("\r\n");
	            Bluetooth_SendString("Gas percent is: ");
	            Bluetooth_SendString(Gas);
	            break;
	        case 4:
	            Bluetooth_SendString("\r\n");
	            Bluetooth_SendString("Humidity percent is: ");
	            Bluetooth_SendString(Humidity);
	            break;
	        case 5:
	            Bluetooth_SendString("Jew");
	            Bluetooth_SendString("\r\n");
	            break;

	        default:
	            // 0 or unrecognized
	            break;
	    }
	    // Updates UI layout for the requested sensor
	    DrawBackdrop(choice);
	    }
		}
	    // Auto-cycles menu every 10 seconds if not paused
	} else if (CheckDelay(&system_timer, 10000) == 1 && pause == 0) {
			choice++;
		if (choice > 5) {
			choice = 1;
		}
	    DrawBackdrop(choice);
	}

}

// Continuously monitors sensor data against safety thresholds
void CheckAlarms(void) {
    // Evaluate thresholds
    alarm_active = 0;
    static uint32_t alarm_print_timer = 0;
    uint8_t print_now = CheckDelay(&alarm_print_timer, 1000);

    if (print_now == 1) {
    	if (gas_percent > 30.0f) { // High gas concentration
    		Bluetooth_SendString("WARNING!!! GAS PERCENTAGE IS DANGEROUSLY HIGH\r\n");
    		alarm_active = 1;
    }
    	if (temp_val > 35.) {// Critical temperature
    		Bluetooth_SendString("WARNING!!! TEMPERATURE IS DANGEROUSLY HIGH\r\n");
    		alarm_active = 1;
    }
    	if (dht11.humidity > 80) {            // Critical humidity
    		Bluetooth_SendString("WARNING!!! HUMIDITY PERCENTAGE IS DANGEROUSLY HIGH\r\n");
    		alarm_active = 1;
    }
    	if (pres_val < 95.0f) {    // Critical atmospheric pressure
    		Bluetooth_SendString("WARNING!!! PRESSURE IS DANGEROUSLY LOW\r\n");
    		alarm_active = 1;
	}
    	if	(lux < 5.0f)   {           // Low light
    		Bluetooth_SendString("WARNING!!! ROOM IS DANGEROUSLY DARK\r\n");
    		alarm_active = 1;
	}

    // Emergency Outputs
    	if (alarm_active == 1) {
        // Turn ON Buzzer
    		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
    		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

        // Flash LED every 300ms
    		if (CheckDelay(&alarm_flash_timer, 300) == 1) {
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
        }
    } else {
        // Turn OFF Buzzer, Fan, and LED safely when thresholds normalize
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

    }
    }
}

// Executes once on boot to set up peripherals before the main loop starts
void System_Init(void)
{
    // Initialize Communications
    Bluetooth_Init(&huart1);
    Bluetooth_SendString("TEST CONNECTION OK\r\n");
    DWT_Init();

    //Initialize Sensors
    DHT11_Init(&dht11, &htim10, GPIOA, GPIO_PIN_4);
    bmp280.i2c = &hi2c1; // Use the working OLED I2C1 bus (PB8/PB9)
        bmp280.addr = BMP280_I2C_ADDRESS_0;
        bmp280_init_default_params(&bmp280.params);
        bmp280_init(&bmp280, &bmp280.params);

    //Initialize OLED and Splash Screen
    ssd1306_Init(&hi2c1);
    ssd1306_Fill(0);
    ssd1306_SetCursor(5, 5);
    ssd1306_WriteString("Smart", Font_7x10, White);
    ssd1306_SetCursor(5, 20);
    ssd1306_WriteString("Environmental", Font_7x10, White);
    ssd1306_SetCursor(5, 35);
    ssd1306_WriteString("Safety Monitoring", Font_7x10, White);
    ssd1306_SetCursor(5, 50);
    ssd1306_WriteString("System", Font_7x10, White);
    ssd1306_UpdateScreen(&hi2c1);

    HAL_Delay(1000);
    //Setups Initial State for Main Loop
    Read_TEMT6000();
    DrawBackdrop(1);
}
