/*
 * show_display.h
 *
 *  Created on: 24 Aug 2026
 *      Author: ernst
 */

#ifndef SHOW_DISPLAY_H
#define SHOW_DISPLAY_H

#include "oled.h"
#include <stdio.h>
typedef enum {
    DISPLAY_MODE_LIVE = 0,
    DISPLAY_MODE_MAX,
    DISPLAY_MODE_MIN,
    DISPLAY_MODE_COUNT // Hilfskonstante für Toggling
} DisplayMode_t;

// High-level rendering API
void OLED_ShowPlaceholder(void);
void OLED_ShowCurrentValues(float temp, float hum);
void OLED_ShowMaxValues(void);
void OLED_ShowMinValues(void);
void OLED_ShowResetConfirmation(void);

// Logic & Navigation API
void OLED_UpdateDisplay(float temp, float hum);
void OLED_SetDisplayMode(DisplayMode_t mode);
void OLED_NextDisplayMode(void);
void OLED_ResetMinMax(void);

#endif /* SHOW_DISPLAY_H */

