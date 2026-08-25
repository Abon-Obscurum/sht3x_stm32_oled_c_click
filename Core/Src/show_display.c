/*
 * show_display.c
 *
 *  Created on: 24 Aug 2026
 *      Author: ernst
 */
#include "show_display.h"
#include "math.h"
#include "main.h"
/**
 * @brief Renders placeholder text before valid readings arrive.
 */
static float last_temp = -999.0f;
static float last_hum  = -999.0f;

static float current_temp = 0.0f;
static float current_hum  = 0.0f;

static float max_temp = -999.0f;
static float max_hum  = -999.0f;
static float min_temp =  999.0f;
static float min_hum  =  999.0f;


static DisplayMode_t current_mode = DISPLAY_MODE_LIVE;
static uint8_t first_sample = 1;


void OLED_ShowPlaceholder(void) {
    Display_Clear(COLOR_BLACK);
    Display_DrawString(0, 0,  "LIVE SENSOR", COLOR_CYAN, COLOR_BLACK);
    Display_DrawString(0, 24, "TEMP: --.- C", COLOR_WHITE, COLOR_BLACK);
    Display_DrawString(0, 48, "HUM:  -- %RH", COLOR_WHITE, COLOR_BLACK);
}

void OLED_ShowCurrentValues(float temp, float hum) {
    char buf[32];
    Display_FillArea(32, 96, 24, 56, COLOR_BLACK);
    Display_DrawString(0, 0, "CURRENT READ", COLOR_CYAN, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "TEMP: %.1f C", temp);
    Display_DrawString(0, 24, buf, COLOR_WHITE, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "HUM:  %.1f %%RH", hum);
    Display_DrawString(0, 48, buf, COLOR_GREEN, COLOR_BLACK);
}

void OLED_ShowMaxValues(void) {
    char buf[32];
    //Display_Clear(COLOR_BLACK);
    Display_DrawString(0, 0, "MAX RECORDED", COLOR_YELLOW, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "MAX T: %.1f C", max_temp);
    Display_DrawString(0, 24, buf, COLOR_RED, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "MAX H: %.1f %%RH", max_hum);
    Display_DrawString(0, 48, buf, COLOR_ORANGE, COLOR_BLACK);
}

void OLED_ShowMinValues(void) {
    char buf[32];
    //Display_Clear(COLOR_BLACK);
    Display_DrawString(0, 0, "MIN RECORDED", COLOR_CYAN, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "MIN T: %.1f C", min_temp);
    Display_DrawString(0, 24, buf, COLOR_BLUE, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "MIN H: %.1f %%RH", min_hum);
    Display_DrawString(0, 48, buf, COLOR_WHITE, COLOR_BLACK);
}

void OLED_ShowResetConfirmation(void) {
    Display_Clear(COLOR_BLACK);
    Display_DrawString(0, 48, "  MIN/MAX   ", COLOR_ORANGE, COLOR_BLACK);
    Display_DrawString(0, 56, "  RESET OK! ", COLOR_GREEN, COLOR_BLACK);
    HAL_Delay(500);
    Display_Clear(COLOR_BLACK);
}

static void OLED_RenderCurrentScreen(void) {
    switch (current_mode) {
        case DISPLAY_MODE_LIVE:
            OLED_ShowCurrentValues(current_temp, current_hum);
            break;
        case DISPLAY_MODE_MAX:
            OLED_ShowMaxValues();
            break;
        case DISPLAY_MODE_MIN:
            OLED_ShowMinValues();
            break;
        default:
            break;
    }
}

void OLED_UpdateDisplay(float temp, float hum) {
    current_temp = temp;
    current_hum  = hum;

    // Ersten Messwert als Initialwert setzen
    if (first_sample) {
        max_temp = temp;
        min_temp = temp;
        max_hum  = hum;
        min_hum  = hum;
        first_sample = 0;
    } else {
        if (temp > max_temp) max_temp = temp;
        if (temp < min_temp) min_temp = temp;
        if (hum > max_hum)   max_hum  = hum;
        if (hum < min_hum)   min_hum  = hum;
    }

    // Neu zeichnen bei Wertänderung
    if (fabsf(temp - last_temp) > 0.1f || fabsf(hum - last_hum) > 0.1f) {
        last_temp = temp;
        last_hum  = hum;
        OLED_RenderCurrentScreen();
    }
}

void OLED_SetDisplayMode(DisplayMode_t mode) {
    if (mode < DISPLAY_MODE_COUNT && current_mode != mode) {
        current_mode = mode;
        OLED_RenderCurrentScreen();
    }
}

void OLED_NextDisplayMode(void) {
    current_mode = (DisplayMode_t)((current_mode + 1) % DISPLAY_MODE_COUNT);
    Display_Clear(COLOR_BLACK); // einmal bildschirm Löschen
    OLED_RenderCurrentScreen();
}

void OLED_ResetMinMax(void) {
	char msg[20];
    first_sample = 1;
    sprintf(msg,"MIN/MAX RESET!");
    //HAL_UART_Transmit(&huart2, msg, sizeof(msg), HAL_MAX_DELAY);
    OLED_ShowResetConfirmation();
}
