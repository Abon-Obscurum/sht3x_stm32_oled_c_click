#ifndef UART_OLED_TEST_H
#define UART_OLED_TEST_H

#include "main.h"
#include "oled.h"
#include "show_display.h"

// Set your configured UART handle here (e.g., huart2 for ST-LINK Virtual COM Port)
extern UART_HandleTypeDef huart2;

/**
 * @brief Initialize UART test interface and print menu.
 */
void UART_OLED_Test_Init(void);

/**
 * @brief Process incoming UART bytes/commands non-blocking or via polling.
 *        Call this function inside the main while(1) loop.
 */
void UART_OLED_Test_Process(void);

#endif /* UART_OLED_TEST_H */
