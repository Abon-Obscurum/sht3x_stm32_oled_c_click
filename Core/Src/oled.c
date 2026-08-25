#include "oled.h"
#include <string.h>


uint8_t display_offset_x = 0;
uint8_t display_offset_y = 0;
uint8_t display_height_virt = 96;
uint8_t display_width_virt = 96;

void Display_SetOrigin(uint8_t x_offset, uint8_t y_offset) {
    display_offset_x = x_offset;
    display_offset_y = y_offset;

}
// Diese Funktion sendet einen Befehl an das OLED-Display über das SPI-Interface.
void Display_SendCommand(uint8_t command) {
  HAL_GPIO_WritePin(DISPLAY_DC_GPIO_Port, DISPLAY_DC_Pin, GPIO_PIN_RESET); // Command mode

  HAL_GPIO_WritePin(DISPLAY_CS_GPIO_Port, DISPLAY_CS_Pin, GPIO_PIN_RESET); // Select Display

  HAL_SPI_Transmit(&hspi1, &command, 1, HAL_MAX_DELAY);

  HAL_GPIO_WritePin(DISPLAY_CS_GPIO_Port, DISPLAY_CS_Pin, GPIO_PIN_SET); // Deselect Display
}

// Diese Funktion sendet Daten an das OLED-Display über das SPI-Interface.
void Display_SendData(uint8_t data) {
  HAL_GPIO_WritePin(DISPLAY_DC_GPIO_Port, DISPLAY_DC_Pin, GPIO_PIN_SET); // Data mode

  HAL_GPIO_WritePin(DISPLAY_CS_GPIO_Port, DISPLAY_CS_Pin, GPIO_PIN_RESET); // Select Display

  HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);

  HAL_GPIO_WritePin(DISPLAY_CS_GPIO_Port, DISPLAY_CS_Pin, GPIO_PIN_SET); // Deselect Display
}

void Display_SendData_Batch(uint8_t *data, uint16_t length) {
  HAL_GPIO_WritePin(DISPLAY_DC_GPIO_Port, DISPLAY_DC_Pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(DISPLAY_CS_GPIO_Port, DISPLAY_CS_Pin, GPIO_PIN_RESET);

  HAL_SPI_Transmit(&hspi1, data, length, HAL_MAX_DELAY);

  HAL_GPIO_WritePin(DISPLAY_CS_GPIO_Port, DISPLAY_CS_Pin, GPIO_PIN_SET);
}


// Diese Funktion setzt das OLED-Display zurück.
void Display_Init(void) {
  HAL_GPIO_WritePin(DISPLAY_RESET_GPIO_Port, DISPLAY_RESET_Pin, GPIO_PIN_RESET); // Reset Display

  HAL_GPIO_WritePin(DISPLAY_RESET_GPIO_Port, DISPLAY_RESET_Pin, GPIO_PIN_SET);


  // Diese Initialisierungssequenz folgt dem SSD1351-Datenblatt. Sie stellt sicher,
  // dass das Display korrekt konfiguriert und einsatzbereit ist.

  Display_SendCommand(0xFD);  // Sperrt und entsperrt die Befehlssteuerung
  Display_SendData(0x12);
  Display_SendCommand(0xFD);
  Display_SendData(0xB1);

  Display_SendCommand(0xAE);  // Deaktiviert das Display (Display Off)

  Display_SendCommand(0xB3);  // Konfiguriert den Fronttaktteiler und die Oszillatorfrequenz
  Display_SendData(0xF0);  // Setzt die Bildwiederholrate auf 100 Frames/Sekunde

  Display_SendCommand(0xCA);  // Legt das Multiplex-Verhältnis fest
  Display_SendData(95);  // Maximale Anzahl der Zeilen (Display Height -1)

  Display_SendCommand(0xA1);  // Legt die Startzeile des Displays fest
  Display_SendData(0x00);  // Start bei Zeile 0

  Display_SendCommand(0xA0);  // Konfiguriert das Adressierungsverfahren und den Dual COM Line-Modus
  Display_SendData(0x60);  // Horizontaler Adressierungsmodus, vertikaler Spiegel deaktiviert

  Display_SendCommand(0xAB);  // Wählt die interne Spannungsquelle aus
  Display_SendData(0x01);  // Aktiviert den internen VDD-Regler

  Display_SendCommand(0xB4);  // Konfiguriert die Segment-Spannungspegel
  Display_SendData(0xA0);  // Externe VSL
  Display_SendData(0xB5);
  Display_SendData(0x55);

  Display_SendCommand(0xC1);  // Setzt den Kontraststrom
  Display_SendData(0xFF);  // Höchster Kontraststrom für Rot
  Display_SendData(0xFF);  // Höchster Kontraststrom für Grün
  Display_SendData(0xFF);  // Höchster Kontraststrom für Blau

  Display_SendCommand(0xBB);  // Setzt die Vorlade-Spannung
  Display_SendData(0x1F);  // Vorladespannung auf 0,6 * VCC gesetzt

  Display_SendCommand(0xA2);  // Setzt den Display-Offset
  Display_SendData(-32);  // Bei -32 füllt sich alles

  Display_SendCommand(0xA6);  // Aktiviert den normalen Display-Modus (keine Invertierung)

  Display_SendCommand(0xAF);  // Schaltet das Display ein (Display On)
  HAL_Delay(100);
}


void Display_Clear(uint16_t color) {
    Display_FillArea(0, DISPLAY_WIDTH - 1, 0, DISPLAY_HEIGHT - 1, color);
}


// Diese Funktion füllt einen bestimmten Quadranten des Displays basierend auf der Richtung.
void Display_FillQuadrant(char* direction, uint16_t color) {
    uint8_t x_start, x_end, y_start, y_end;

    if (strcmp(direction, "N") == 0 || strcmp(direction, "n") == 0) {
        // Norden - oberer Bereich
        x_start = 0;
        x_end = DISPLAY_WIDTH - 1;
        y_start = 0;
        y_end = DISPLAY_HEIGHT / 2 - 1;
    } else if (strcmp(direction, "S") == 0 || strcmp(direction, "s") == 0) {
        // Süden - unterer Bereich
        x_start = 0;
        x_end = DISPLAY_WIDTH - 1;
        y_start = DISPLAY_HEIGHT / 2;
        y_end = DISPLAY_HEIGHT - 1;
    } else if (strcmp(direction, "E") == 0 || strcmp(direction, "e") == 0) {
        // Osten - rechter Bereich
    	x_start = DISPLAY_WIDTH / 2;
    	x_end = DISPLAY_WIDTH - 1;
    	y_start = 0;
    	y_end = DISPLAY_HEIGHT - 1;
    } else if (strcmp(direction, "W") == 0 || strcmp(direction, "w") == 0) {
        // Westen - linker Bereich
        x_start = 0;
        x_end = DISPLAY_WIDTH / 2 - 1;
        y_start = 0;
        y_end = DISPLAY_HEIGHT - 1;
    } else if (strcmp(direction, "NE") == 0 || strcmp(direction, "ne") == 0) {
        // Nordost - oben rechts
        x_start = DISPLAY_WIDTH / 2;
        x_end = DISPLAY_WIDTH - 1;
        y_start = 0;
        y_end = DISPLAY_HEIGHT / 2 - 1;
    } else if (strcmp(direction, "SE") == 0 || strcmp(direction, "se") == 0) {
        // Südost - unten rechts
        x_start = DISPLAY_WIDTH / 2;
        x_end = DISPLAY_WIDTH - 1;
        y_start = DISPLAY_HEIGHT / 2;
        y_end = DISPLAY_HEIGHT - 1;
    } else if (strcmp(direction, "SW") == 0 || strcmp(direction, "sw") == 0) {
        // Südwest - unten links
    	 x_start = 0;
    	 x_end = DISPLAY_WIDTH / 2 - 1;
    	 y_start = DISPLAY_HEIGHT / 2;
    	 y_end = DISPLAY_HEIGHT - 1;
    } else if (strcmp(direction, "NW") == 0 || strcmp(direction, "nw") == 0) {
        // Nordwest - oben links
    	x_start = 0;
    	x_end = DISPLAY_WIDTH / 2 - 1;
    	y_start = 0;
    	y_end = DISPLAY_HEIGHT / 2 - 1;
    } else {
        return;  // Ungültige Richtung, keine Aktion
    }

    // Bereich füllen
    Display_FillArea(x_start, x_end, y_start, y_end, color);
}

// Diese Funktion füllt einen rechteckigen Bereich des Displays mit der angegebenen Farbe.
void Display_FillArea(uint8_t x_start, uint8_t x_end, uint8_t y_start, uint8_t y_end, uint16_t color) {
    // Calculate physical hardware coordinates
    uint8_t phys_x_start = x_start + display_offset_x;
    uint8_t phys_x_end   = x_end + display_offset_x;
    uint8_t phys_y_start = y_start + display_offset_y;
    uint8_t phys_y_end   = y_end + display_offset_y;

    // Clamp coordinates to physical display bounds (128x96 max)
    if (phys_x_start > 127) phys_x_start = 127;
    if (phys_x_end > 127)   phys_x_end = 127;
    if (phys_y_start > 127)  phys_y_start = 127;
    if (phys_y_end > 127)    phys_y_end = 127;

    // Set Column Address (0x15)
    Display_SendCommand(0x15);
    Display_SendData(phys_x_start);
    Display_SendData(phys_x_end);

    // Set Row Address (0x75)
    Display_SendCommand(0x75);
    Display_SendData(phys_y_start);
    Display_SendData(phys_y_end);

    // Write RAM (0x5C)
    Display_SendCommand(0x5C);

    uint8_t high_byte = (color >> 8) & 0xFF;
    uint8_t low_byte  = color & 0xFF;

    uint16_t total_pixels = (phys_x_end - phys_x_start + 1) * (phys_y_end - phys_y_start + 1);

    for (uint16_t i = 0; i < total_pixels; i++) {
        Display_SendData(high_byte);
        Display_SendData(low_byte);
    }
}
// Basic 5x7 ASCII font starting from ASCII space (0x20 / 32)
const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Space (0x20)
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}  // Z
};

// Draws a single pixel taking into account virtual origin
void Display_DrawPixel(uint8_t x, uint8_t y, uint16_t color) {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) return;
    Display_FillArea(x, x, y, y, color);
}

// Draws a single ASCII character (5x7)
void Display_DrawChar(uint8_t x, uint8_t y, char c, uint16_t text_color, uint16_t bg_color) {
    if (c < ' ' || c > 'Z') c = '?'; // Default out-of-bounds characters to '?'

    uint8_t font_index = c - ' ';

    for (uint8_t col = 0; col < 5; col++) {
        uint8_t line = font5x7[font_index][col];
        for (uint8_t row = 0; row < 7; row++) {
            if (line & (1 << row)) {
                Display_DrawPixel(x + col, y + row, text_color);
            } else if (bg_color != text_color) { // Draw background if provided
                Display_DrawPixel(x + col, y + row, bg_color);
            }
        }
    }

    // Draw 1-pixel column spacing between characters
    if (bg_color != text_color) {
        for (uint8_t row = 0; row < 7; row++) {
            Display_DrawPixel(x + 5, y + row, bg_color);
        }
    }
}

// Draws a null-terminated string onto the display
void Display_DrawString(uint8_t x, uint8_t y, const char *str, uint16_t text_color, uint16_t bg_color) {
    while (*str) {
        if (x + 6 > DISPLAY_WIDTH) { // Wrap to next line if end of screen reached
            x = 0;
            y += 8;
        }
        if (y + 7 > DISPLAY_HEIGHT) break; // Stop if screen height limit reached

        Display_DrawChar(x, y, *str, text_color, bg_color);
        x += 6; // Move cursor right by character width + 1 pixel spacing
        str++;
    }
}
void OLED_PlayStartupAnimation(void) {
    Display_Clear(COLOR_BLACK);

    // Phase 1: Tech-Rahmen zeichnen (Aufbau von außen nach innen)
    uint8_t step = 4;
    uint8_t max_offset = (display_height_virt / 2) - 8;

    for (uint8_t i = 0; i < max_offset; i += step) {
        uint8_t x1 = i;
        uint8_t x2 = display_width_virt - 1 - i;
        uint8_t y1 = i;
        uint8_t y2 = display_height_virt - 1 - i;

        // Oben & Unten
        Display_FillArea(x1, x2, y1, y1, COLOR_CYAN);
        Display_FillArea(x1, x2, y2, y2, COLOR_CYAN);

        // Links & Rechts (macht es zu einem echten geschlossenen Rahmen)
        //Display_FillArea(x1, x1, y1, y2, COLOR_CYAN);
        //Display_FillArea(x2, x2, y1, y2, COLOR_CYAN);

        HAL_Delay(20); // Leichte Verzögerung für flüssigen Aufbau
    }

    HAL_Delay(500);
    Display_Clear(COLOR_BLACK);

    // Phase 2: Ladebalken & Grid-Scan
    for (uint8_t x = 0; x < display_width_virt; x += 4) {
        // Vertical Scanning Line
        Display_FillArea(x, x + 1, 0, display_height_virt - 1, COLOR_BLUE);

        // Progress bar at the bottom
        Display_FillArea(0, x, display_height_virt - 6, display_height_virt - 1, COLOR_GREEN);

        HAL_Delay(15);

        // Trail-Effekt entfernen
        Display_FillArea(x, x + 1, 0, display_height_virt - 7, COLOR_BLACK);
    }

    Display_Clear(COLOR_BLACK);

    // Phase 3: Pop-in Text "SHT3X ONLINE" mit Farb-Flash
    Display_DrawString(10, 0, "SYSTEM INIT", COLOR_YELLOW, COLOR_BLACK);
    HAL_Delay(400);

    Display_DrawString(8, 16, "SHT3X SENSOR", COLOR_CYAN, COLOR_BLACK);
    Display_DrawString(16, 32, "[ READY ]", COLOR_GREEN, COLOR_BLACK);
    Display_DrawString(0, 48, "Emmerich / Wieser", COLOR_MAGENTA, COLOR_WHITE);
    HAL_Delay(700);

    // Phase 4: Smooth Fade-out Screen Flash
    Display_Clear(COLOR_WHITE);
    HAL_Delay(80);
    Display_Clear(COLOR_BLACK);
}
