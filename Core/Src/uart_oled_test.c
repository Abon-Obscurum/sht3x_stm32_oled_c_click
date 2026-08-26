#include "uart_oled_test.h"
#include <stdio.h>
#include <string.h>

#define RX_BUFFER_SIZE 64

static uint8_t rx_byte;
static char rx_buffer[RX_BUFFER_SIZE];
static uint8_t rx_index = 0;

/* Helper to transmit string over UART */
static void UART_SendString(const char *str) {
    HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

/* Print help menu to PC Serial Terminal */
static void UART_PrintMenu(void) {
    UART_SendString("\r\n========================================\r\n");
    UART_SendString("   OLED & Show_Display UART Test Menu   \r\n");
    UART_SendString("========================================\r\n");
    UART_SendString(" Commands:\r\n");
    UART_SendString("  1 : Run OLED Driver Test (Fill screen / draw pattern)\r\n");
    UART_SendString("  2 : Run Show_Display Test (Draw temperature/humidity UI)\r\n");
    UART_SendString("  3 : Clear OLED Screen\r\n");
    UART_SendString("  text <msg> : Display custom text on OLED\r\n");
    UART_SendString("  help       : Show this menu\r\n");
    UART_SendString("========================================\r\n> ");
}

/* Parse and execute received CLI command */
static void UART_ExecuteCommand(char *cmd) {
    UART_SendString("\r\n");

    if (strcmp(cmd, "1") == 0) {
        UART_SendString("[TEST] Executing OLED Driver Test...\r\n");
        oled_init();
        oled_fill(0xFFFF); // Fill White
        HAL_Delay(500);
        oled_fill(0x0000); // Clear Black
        UART_SendString("[TEST] OLED Driver Test Completed.\r\n");
    }
    else if (strcmp(cmd, "2") == 0) {
        UART_SendString("[TEST] Executing Show Display UI Test...\r\n");
        // Example mock values for SHT3x sensor display
        float mock_temp = 24.5f;
        float mock_hum  = 55.0f;

        oled_init();
        show_display_data(mock_temp, mock_hum); // Call display routine
        UART_SendString("[TEST] Show Display UI Updated.\r\n");
    }
    else if (strcmp(cmd, "3") == 0) {
        UART_SendString("[TEST] Clearing Screen...\r\n");
        oled_fill(0x0000);
        UART_SendString("[TEST] Screen Cleared.\r\n");
    }
    else if (strncmp(cmd, "text ", 5) == 0) {
        char *text_to_show = cmd + 5;
        UART_SendString("[TEST] Displaying custom string: ");
        UART_SendString(text_to_show);
        UART_SendString("\r\n");

        oled_fill(0x0000);
        // Assuming standard font printing function from oled.c
        oled_draw_string(0, 0, (uint8_t*)text_to_show, 0xFFFF, 0x0000);
    }
    else if (strcmp(cmd, "help") == 0) {
        UART_PrintMenu();
        return;
    }
    else {
        UART_SendString("Unknown Command. Type 'help' for options.\r\n");
    }

    UART_SendString("> ");
}

void UART_OLED_Test_Init(void) {
    rx_index = 0;
    memset(rx_buffer, 0, RX_BUFFER_SIZE);
    UART_PrintMenu();
    // Enable single-byte interrupt reception
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
}

void UART_OLED_Test_Process(void) {
    // Called in main loop if non-interrupt processing is needed
}

/* UART Interrupt Callback */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == huart2.Instance) {
        // Echo character back to terminal
        HAL_UART_Transmit(&huart2, &rx_byte, 1, HAL_MAX_DELAY);

        // Handle Enter key (\r or \n)
        if (rx_byte == '\r' || rx_byte == '\n') {
            rx_buffer[rx_index] = '\0';
            if (rx_index > 0) {
                UART_ExecuteCommand(rx_buffer);
                rx_index = 0;
            } else {
                UART_SendString("\r\n> ");
            }
        }
        // Handle Backspace
        else if (rx_byte == '\b' || rx_byte == 0x7F) {
            if (rx_index > 0) {
                rx_index--;
            }
        }
        // Buffer character
        else {
            if (rx_index < RX_BUFFER_SIZE - 1) {
                rx_buffer[rx_index++] = rx_byte;
            }
        }

        // Re-enable interrupt for next byte
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }
}
