#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "lcd.h"
#include "pac.h"
#include "pac0.h"

static const char *TAG = "lab01";

#define DELAY_MS(ms) \
	vTaskDelay(((ms)+(portTICK_PERIOD_MS-1))/portTICK_PERIOD_MS)

//----------------------------------------------------------------------------//
// Car Implementation - Begin
//----------------------------------------------------------------------------//

// Car constants
#define CAR_CLR rgb565(220,30,0)
#define WINDOW_CLR rgb565(180,210,238)
#define TIRE_CLR BLACK
#define HUB_CLR GRAY

// Car geometry (coordinates are relative to the anchor point (0,0)).
#define CAR_W 60
#define CAR_H 32

// Body rectangle (main lower body)
#define BODY_X0 0
#define BODY_Y0 12
#define BODY_X1 59
#define BODY_Y1 24

// Roof / cabin rectangle (left portion)
#define ROOF_X0 1
#define ROOF_Y0 0
#define ROOF_X1 39
#define ROOF_Y1 11

// Front sloped roof triangle points
#define ROOF_SLOPE_X0 40
#define ROOF_SLOPE_Y0 11
#define ROOF_SLOPE_X1 40
#define ROOF_SLOPE_Y1 9
#define ROOF_SLOPE_X2 59
#define ROOF_SLOPE_Y2 11

// Windows (rounded rectangles)
#define WIN0_X0 3
#define WIN0_Y0 1
#define WIN0_X1 18
#define WIN0_Y1 8
#define WIN1_X0 21
#define WIN1_Y0 1
#define WIN1_X1 37
#define WIN1_Y1 8
#define WIN_R 2

// Wheel centers and radii
#define WHEEL0_X 11
#define WHEEL1_X 48
#define WHEEL_Y 24
#define WHEEL_R_OUT 7
#define WHEEL_R_IN 4

/**
 * @brief Draw a car at the specified location.
 * @param x      Top left corner X coordinate.
 * @param y      Top left corner Y coordinate.
 * @details Draw the car components relative to the anchor point (top, left).
 */
void drawCar(coord_t x, coord_t y)
{
	// Draw roof
	lcd_fillRect2(x + ROOF_X0, y + ROOF_Y0, x + ROOF_X1, y + ROOF_Y1, CAR_CLR);

	// Draw front triangle
	lcd_fillTriangle(x + ROOF_SLOPE_X0, y + ROOF_SLOPE_Y0,
					 x + ROOF_SLOPE_X1, y + ROOF_SLOPE_Y1,
					 x + ROOF_SLOPE_X2, y + ROOF_SLOPE_Y2,
					 CAR_CLR);

	// Draw main body
	lcd_fillRect2(x + BODY_X0, y + BODY_Y0, x + BODY_X1, y + BODY_Y1, CAR_CLR);

	// Draw windows
	lcd_fillRoundRect2(x + WIN0_X0, y + WIN0_Y0, x + WIN0_X1, y + WIN0_Y1, WIN_R, WINDOW_CLR);
	lcd_fillRoundRect2(x + WIN1_X0, y + WIN1_Y0, x + WIN1_X1, y + WIN1_Y1, WIN_R, WINDOW_CLR);

	// Draw wheels
	lcd_fillCircle(x + WHEEL0_X, y + WHEEL_Y, WHEEL_R_OUT, TIRE_CLR);
	lcd_fillCircle(x + WHEEL0_X, y + WHEEL_Y, WHEEL_R_IN, HUB_CLR);

	lcd_fillCircle(x + WHEEL1_X, y + WHEEL_Y, WHEEL_R_OUT, TIRE_CLR);
	lcd_fillCircle(x + WHEEL1_X, y + WHEEL_Y, WHEEL_R_IN, HUB_CLR);
}

//----------------------------------------------------------------------------//
// Car Implementation - End
//----------------------------------------------------------------------------//

// Main display constants
#define BACKGROUND_CLR rgb565(0,60,90)
#define TITLE_CLR GREEN
#define STATUS_CLR WHITE
#define STR_BUF_LEN 12 // string buffer length
#define FONT_SIZE 2
#define FONT_W (LCD_CHAR_W*FONT_SIZE)
#define FONT_H (LCD_CHAR_H*FONT_SIZE)
#define STATUS_W (FONT_W*3)

#define WAIT 2000 // milliseconds
#define DELAY_EX3 20 // milliseconds

// Object position and movement
#define OBJ_X 100
#define OBJ_Y 100
#define OBJ_MOVE 3 // pixels


/**
 * @brief Application entry point.
 * @details Runs the display exercises in sequence. This function performs
 *          drawing operations for exercises 1 to 5 and uses the LCD frame
 *          buffer where requested.
 */
void app_main(void)
{
	ESP_LOGI(TAG, "Start up");
	lcd_init();
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_setFontSize(FONT_SIZE);
	lcd_drawString(0, 0, "Hello World! (lcd)", TITLE_CLR);
	printf("Hello World! (terminal)\n");
	DELAY_MS(WAIT);

	// Exercise 1 - Draw car in one location.
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_drawString(0, 0, "Exercise 1", TITLE_CLR);
	drawCar(OBJ_X, OBJ_Y);
	DELAY_MS(WAIT);

	// Exercise 2 - Draw moving car (Method 1), one pass across display.
	// Clear the entire display and redraw all objects each iteration.
	// Use a loop and increment x by OBJ_MOVE each iteration.
	// Start x off screen (negative coordinate).
	{
		char str[STR_BUF_LEN];

		for (coord_t x = -CAR_W; x <= LCD_W; x += OBJ_MOVE) {
			lcd_fillScreen(BACKGROUND_CLR);
			lcd_drawString(0, 0, "Exercise 2", TITLE_CLR);

			drawCar(x, OBJ_Y);
			sprintf(str, "%3ld", (long)x);
			lcd_drawString(0, LCD_H - FONT_H, str, STATUS_CLR);
		}
	}

	// TODO: Exercise 3 - Draw moving car (Method 2), one pass across display.
	// Move by erasing car at old position, then redrawing at new position.
	// Objects that don't change or move are drawn once.
	{
		char str[STR_BUF_LEN];
		lcd_fillScreen(BACKGROUND_CLR);
		lcd_setFontSize(FONT_SIZE);
		lcd_drawString(0, 0, "Exercise 3", TITLE_CLR);

		/* Keep track of the previous x position for erasing */
		coord_t prev_x = -CAR_W;

		for (coord_t x = -CAR_W; x <= LCD_W; x += OBJ_MOVE) {
			/* Erase old car and draw new one */
			lcd_fillRect2(prev_x, OBJ_Y, prev_x + CAR_W - 1, OBJ_Y + CAR_H - 1, BACKGROUND_CLR);
			drawCar(x, OBJ_Y);
			
			/* Erase old status and draw new one */
			lcd_fillRect2(0, LCD_H - FONT_H, STATUS_W - 1, LCD_H - 1, BACKGROUND_CLR);
			sprintf(str, "%3ld", (long)x);
			lcd_drawString(0, LCD_H - FONT_H, str, STATUS_CLR);
			prev_x = x;

			DELAY_MS(DELAY_EX3);
		}
	}

	// TODO: Exercise 4 - Draw moving car (Method 3), one pass across display.
	// First, draw all objects into a cleared, off-screen frame buffer.
	// Then, transfer the entire frame buffer to the screen.
	{
		char str[STR_BUF_LEN];
		lcd_frameEnable();

		for (coord_t x = -CAR_W; x <= LCD_W; x += OBJ_MOVE) {
			lcd_fillScreen(BACKGROUND_CLR);
			lcd_drawString(0, 0, "Exercise 4", TITLE_CLR);
			
			drawCar(x, OBJ_Y);

			/* Draw status */
			sprintf(str, "%3ld", (long)x);
			lcd_drawString(0, LCD_H - FONT_H, str, STATUS_CLR);

			lcd_writeFrame();
		}

		lcd_frameDisable();
	}

	// TODO: Exercise 5 - Draw an animated Pac-Man moving across the display.
	// Use Pac-Man sprites instead of the car object.
	// Cycle through each sprite when moving the Pac-Man character.
	{
		const uint8_t pidx[] = {0, 1, 2, 1};
		uint16_t pi = 0;

		char str[STR_BUF_LEN];

		lcd_frameEnable();

		for (;;) {
			for (coord_t x = -PAC0_W; x <= LCD_W; x += OBJ_MOVE) {
				lcd_fillScreen(BACKGROUND_CLR);
				lcd_drawString(0, 0, "Exercise 5", TITLE_CLR);

				/* Draw Pac-Man */
				const uint8_t *bmp = pac[pidx[pi++ % (sizeof(pidx)/sizeof(pidx[0]))]];
				lcd_drawBitmap(x, OBJ_Y, bmp, PAC0_W, PAC0_H, YELLOW);

				/* Draw status */
				sprintf(str, "%3ld", (long)x);
				lcd_drawString(0, LCD_H - FONT_H, str, STATUS_CLR);

				lcd_writeFrame();
			}
		}
		lcd_frameDisable();
	}
}
