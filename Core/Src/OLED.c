#include "OLED.h"
#include <stdio.h>
#include "Backdrop.h"
#include "AmbientLight.h"
#include "GasSensor.h"
#include "DHT11.h"
#include "bmp280.h"
#include "SystemControls.h"

// Updates the dynamic text values on the currently selected screen.
void RefreshOLED(uint16_t input) {
	    switch (input)
	    {
	        case 1:
	        	PrintLight(Ambiance);
	            break;

	        case 2:
				PrintTemp(Temperature);
				break;
	        case 3:
	        	PrintGas(Gas);
	            break;
	        case 4:
	        	PrintHumidity(Humidity);
	            break;
	        case 5:
	        	PrintPressure(Pressure);
	            break;

	        default:
	            // 0 or unrecognized
	            break;
	    }
	    ssd1306_UpdateScreen(&hi2c1);

}

// Draws the static background graphics for the currently selected screen
void DrawBackdrop(int16_t input)  {
	// Wipes out previous screen
    ssd1306_Fill(0);
    // Dynamically chnages loading bar
    LoadingBar(input);
    switch (input)
    {
        case 1:
        	 ssd1306_DrawBitmap(0, 0, Light_png, 128, 64, White);
            break;

        case 2:
        	ssd1306_DrawBitmap(0, 0, Temp_png, 128, 64, White);
			break;
        case 3:
        	ssd1306_DrawBitmap(0, 0, Gas_png, 128, 64, White);
            break;
        case 4:
            ssd1306_DrawBitmap(0, 0, Humidity_png, 128, 64, White);
            break;
        case 5:
        	ssd1306_DrawBitmap(0, 0, Pressure_png, 128, 64, White);
        	break;

        default:
            // 0 or unrecognized
            break;
    }
    ssd1306_UpdateScreen(&hi2c1);

}

// Text and Positioning based on selected screen
void PrintHumidity(const char *str1) {
    ssd1306_SetCursor(10, 5);
        ssd1306_WriteString("Humidity:", Font_7x10, 1);
        ssd1306_SetCursor(10,24);
        ssd1306_WriteString((char *)str1, Font_16x26, 1);
}

void PrintTemp(const char *str1) {
    ssd1306_SetCursor(10, 5);
        ssd1306_WriteString("Temp:", Font_7x10, 1);
        ssd1306_SetCursor(10,24);
        ssd1306_WriteString((char *)str1, Font_16x26, 1);
}

void PrintLight(const char *str1) {
    ssd1306_SetCursor(10, 5);
        ssd1306_WriteString("Ambiance:", Font_7x10, 1);
        ssd1306_SetCursor(10,24);
        ssd1306_WriteString((char *)str1, Font_11x18, 1);
}

void PrintGas(const char *str1) {
    ssd1306_SetCursor(10, 3);
        ssd1306_WriteString("Gas:", Font_7x10, 1);
        ssd1306_SetCursor(10,21);
        ssd1306_WriteString((char *)str1, Font_11x18, 1);
}

void PrintPressure(const char *str1) {
    ssd1306_SetCursor(10, 3);
        ssd1306_WriteString("Pressure:", Font_7x10, 1);
        ssd1306_SetCursor(10,21);
        ssd1306_WriteString((char *)str1, Font_11x18, 1);
}

//Draws a bar at the bottom of the screen.
void LoadingBar(int16_t input) {
	if (input == 1) {
		DrawFilledSquare(5, 54, 5, 1);
		DrawHollowSquare(12, 54, 5, 1);
		DrawHollowSquare(19, 54, 5, 1);
		DrawHollowSquare(26, 54, 5, 1);
		DrawHollowSquare(33, 54, 5, 1);
	} else if (input == 2) {
		DrawHollowSquare(5, 54, 5, 1);
		DrawFilledSquare(12, 54, 5, 1);
		DrawHollowSquare(19, 54, 5, 1);
		DrawHollowSquare(26, 54, 5, 1);
		DrawHollowSquare(33, 54, 5, 1);
	} else if (input == 3) {
		DrawHollowSquare(5, 54, 5, 1);
		DrawHollowSquare(12, 54, 5, 1);
		DrawFilledSquare(19, 54, 5, 1);
		DrawHollowSquare(26, 54, 5, 1);
		DrawHollowSquare(33, 54, 5, 1);
	} else if (input == 4) {
		DrawHollowSquare(5, 54, 5, 1);
		DrawHollowSquare(12, 54, 5, 1);
		DrawHollowSquare(19, 54, 5, 1);
		DrawFilledSquare(26, 54, 5, 1);
		DrawHollowSquare(33, 54, 5, 1);
	} else if (input == 5) {
		DrawHollowSquare(5, 54, 5, 1);
		DrawHollowSquare(12, 54, 5, 1);
		DrawHollowSquare(19, 54, 5, 1);
		DrawHollowSquare(26, 54, 5, 1);
		DrawFilledSquare(33, 54, 5, 1);
	}

}
