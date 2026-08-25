#ifndef OLED_H
#define OLED_H

#include "main.h"

// OLED Pin Definitions
#define DISPLAY_CS_GPIO_Port GPIOB
#define DISPLAY_CS_Pin GPIO_PIN_0

#define DISPLAY_DC_GPIO_Port GPIOB
#define DISPLAY_DC_Pin GPIO_PIN_1

#define DISPLAY_RESET_GPIO_Port GPIOB
#define DISPLAY_RESET_Pin GPIO_PIN_7

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 128

#define OFFSET_X 16
#define OFFSET_Y 0

extern SPI_HandleTypeDef hspi1;

extern uint8_t display_offset_x;
extern uint8_t display_offset_y;

// Font & Text Declarations
extern const uint8_t font5x7[][5];

// Dynamic Startup Animation
void OLED_PlayStartupAnimation(void);

void Display_DrawPixel(uint8_t x, uint8_t y, uint16_t color);
void Display_DrawChar(uint8_t x, uint8_t y, char c, uint16_t text_color, uint16_t bg_color);
void Display_DrawString(uint8_t x, uint8_t y, const char *str, uint16_t text_color, uint16_t bg_color);
// OLED Function Declarations
void Display_SendCommand(uint8_t command);
void Display_SendData(uint8_t data);
void Display_Init(void);
void Display_Clear(uint16_t color);

//void Display_Clear_2(uint16_t color);
void Display_FillQuadrant(char* direction, uint16_t color);
void Display_FillArea(uint8_t x_start, uint8_t x_end, uint8_t y_start, uint8_t y_end, uint16_t color);

void Display_SetOrigin(uint8_t x_offset, uint8_t y_offset);
void Display_FillArea(uint8_t x_start, uint8_t x_end, uint8_t y_start, uint8_t y_end, uint16_t color);
void Display_Clear(uint16_t color);
//void Display_FillArea_2(uint8_t x_start, uint8_t x_end, uint8_t y_start, uint8_t y_end, uint16_t color);

// Predefined RGB565 Colors

#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_CYAN    0x07FF
#define COLOR_MAGENTA 0xF81F
#define COLOR_ORANGE  0xFD20
#define RGB565(r, g, b) (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))
#endif /* OLED_H */
