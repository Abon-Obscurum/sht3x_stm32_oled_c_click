# SHT3x

Driver for the SHT3x temperature and humidity sensor 

This driver allows the user to get temperature and humidity readings from an SHT3x sensor over I2C on an STM32 (HAL-based), with CRC-8 validation of every reading. It also includes heater control with a built-in self-test, and a standalone interactive UART menu so the sensor can be tested independently of the rest of the application.

## Features

  - Sensor init with bus presence check (`HAL_I2C_IsDeviceReady`) and soft reset

- Single-shot, high-repeatability measurement (clock-stretching command `0x2C06`)

- CRC-8 check on both the temperature and humidity bytes before the reading is accepted

- Conversion of raw 16-bit sensor codes to °C / %RH per the datasheet formulas

- Heater on/off control, plus a `HeaterTest()` self-test that verifies the heater actually raises the measured temperature

- Standalone UART debug menu (`r` = read, `h` = heater test, `q`/`x` = quit) for testing the sensor with no other application code required

## Files

| File | Purpose |
|---|---|
| `sht3xstm32.h` | Public types, constants, and function declarations |
| `sht3xstm32.c` | Driver implementation |

## Hardware Setup

| Pin Name | Shield Pin | MCU Pin |
| -------- | ---------- | ------- |
| SDA      | D4         | PB7     |
| SCL      | D5         | PB6     |


## API reference

### Data types  

```c

typedef struct {

I2C_HandleTypeDef *hi2c; // I2C peripheral, e.g. &hi2c1

uint8_t deviceAddress; // 7-bit I2C address

uint8_t initialized; // set once Init() has run successfully

} SHT3xSTM32_Handle_t;

  

typedef struct {

float temperature; // °C

float humidity; // %RH (0-100)

} SHT3xSTM32_Data_t;

```

  

### Core functions

| Function                                       | Description                                                                                                                                                              |
| ---------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `SHT3xSTM32_Init(handle, hi2c, deviceAddress)` | Fills in the handle, confirms the sensor answers on the bus, and soft-resets it to a known state.                                                                        |
| `SHT3xSTM32_Read(handle, data)`                | Triggers a measurement, waits for conversion, reads 6 bytes, checks CRC, and writes the converted values into `data`.                                                    |
| `SHT3xSTM32_SetHeater(handle, enable)`         | Turns the sensor's built-in heater on (`1`) or off (`0`).                                                                                                                |
| `SHT3xSTM32_HeaterTest(handle)`                | Reads, enables the heater for ~10 s, reads again, and returns `HAL_OK` if temperature rose by more than 0.5 °C. Always turns the heater back off, even on a failed read. |

### Conversion / low-level helpers

| Function | Description |
|---|---|
| `SHT3xSTM32_ConvertTemperature(rawTemp)` | Raw 16-bit code → °C. |
| `SHT3xSTM32_ConvertHumidity(rawHumidity)` | Raw 16-bit code → %RH. |
| `SHT3xSTM32_BytesToUint16(byteHigh, byteLow)` | Combines two bytes (MSB-first, as sent by the sensor) into one `uint16_t`. |
| `SHT3xSTM32_CalculateCRC(data, length)` | Computes the sensor's CRC-8 checksum over a byte block for validation. |

### UART debug menu

| Function | Description |
|---|---|
| `SHT3xSTM32_Debug_Init(uart, handle)` | Registers the UART and sensor handle used by the menu. Call once, after `SHT3xSTM32_Init()`. |
| `SHT3xSTM32_UART_MenuHandler(void)` | Blocking loop that prints a menu and handles `r` (read), `h` (heater test), and `q`/`x` (quit) over the debug UART. |


## Usage example


```c

SHT3xSTM32_Handle_t sht3x;

SHT3xSTM32_Data_t data;

  

if (SHT3xSTM32_Init(&sht3x, &hi2c1, 0x44) == HAL_OK) {

	if (SHT3xSTM32_Read(&sht3x, &data) == HAL_OK) {
	
	// use data.temperature / data.humidity
	
	}

}

```

  
### Standalone testing over UART

```c

SHT3xSTM32_Debug_Init(&huart2, &sht3x);

SHT3xSTM32_UART_MenuHandler(); // blocks; type r / h / q at the terminal

```


## Notes / known limitations

- `SHT3_STM32_TIMEOUT` (30 ms) is reused both as the post-measurement wait and as the general HAL I2C timeout; the datasheet only specifies ~15 ms for the measurement itself.

- `SHT3xSTM32_HeaterTest()` blocks for ~10 seconds (`HAL_Delay`) — don't call it from a timer callback or any time-critical path.

# DISPLAY PART

---

## 📄 `oled.c`

### Overview

`oled.c` contains low-level hardware communication routines, command registers, and core graphic primitives for the **OLED C Click board** (powered by the **Solomon Systech SSD1351** display controller). It handles SPI command/data bus transfers, initialization sequences, color space mapping (RGB565), boundary setup, and fundamental drawing functions (pixels, lines, filled rectangles, and text characters).

---

### Hardware Configuration & Data Types

* **Bus Interface:** 4-wire SPI (MOSI, SCK, CS, DC/Data-Command pin, Reset, Power Enable).
* **Color Format:** RGB565 (16-bit color packed as 5 bits Red, 6 bits Green, 5 bits Blue).
* **Display Dimensions:** 96 × 96 pixels.

---

### Key Constants & Macros

```c
#define OLED_C_CMD_SET_COLUMN     0x15
#define OLED_C_CMD_SET_ROW        0x75
#define OLED_C_CMD_WRITE_RAM      0x5C
#define OLED_C_CMD_SET_DISP_ON    0xAF
#define OLED_C_CMD_SET_DISP_OFF   0xAE

#define OLED_C_WIDTH              96
#define OLED_C_HEIGHT             96

```

---

### Functions

#### `void OLED_C_WriteCommand(uint8_t command)`

Sends a single command byte over SPI to the SSD1351 controller.

* **Parameters:**
* `command`: 8-bit SSD1351 instruction register code.


* **Behavior:** Pulls the **D/C (Data/Command)** pin LOW to signal command mode, activates Chip Select (`CS = 0`), transmits the byte over SPI, and deasserts Chip Select (`CS = 1`).

---

#### `void OLED_C_WriteData(uint8_t data)`

Sends a single data payload byte over SPI.

* **Parameters:**
* `data`: Data byte to write to the current configuration register or display RAM.


* **Behavior:** Pulls the **D/C** pin HIGH to signal data mode, transfers the byte over SPI while managing Chip Select.

---

#### `void OLED_C_WriteBuffer(uint8_t *pBuffer, uint32_t length)`

Sends a stream of bytes to the display RAM sequentially.

* **Parameters:**
* `pBuffer`: Pointer to the source data array.
* `length`: Number of bytes to transmit.


* **Behavior:** Sets `D/C` HIGH and streams memory via SPI (or DMA if enabled) for high-speed frame updates.

---

#### `void OLED_C_Init(void)`

Initializes the GPIO pins, resets the OLED controller hardware, and sends the required initialization register table to bring up the display matrix.

* **Parameters:** None
* **Returns:** None
* **Steps:**
1. Toggles hardware `RESET` pin LOW then HIGH with delay.
2. Enables display power supply via the power pin.
3. Transmits SSD1351 startup sequence (Command Lock, Display Off, Clock Divider, Multiplex Ratio, Remap & Color Depth RGB565, Master Current, Contrast, VCOMH setting, and Display On).



---

#### `void OLED_C_SetAddressWindow(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)`

Defines active column and row memory window bounds prior to writing pixel data into graphic RAM.

* **Parameters:**
* `x1`: Starting column coordinate (X-min).
* `y1`: Starting row coordinate (Y-min).
* `x2`: Ending column coordinate (X-max).
* `y2`: Ending row coordinate (Y-max).



---

#### `void OLED_C_FillScreen(uint16_t color)`

Fills the entire active display matrix with a solid RGB565 color.

* **Parameters:**
* `color`: 16-bit RGB565 color value.



---

#### `void OLED_C_DrawPixel(uint8_t x, uint8_t y, uint16_t color)`

Draws a single pixel at specified `(x, y)` coordinates.

* **Parameters:**
* `x`: X position (0 to Width - 1).
* `y`: Y position (0 to Height - 1).
* `color`: 16-bit RGB565 pixel color.



---

#### `void OLED_C_DrawRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color)`

Draws a filled rectangle on the display screen.

* **Parameters:**
* `x`: Upper-left corner X coordinate.
* `y`: Upper-left corner Y coordinate.
* `width`: Rectangle width in pixels.
* `height`: Rectangle height in pixels.
* `color`: Fill color (RGB565 format).



---

#### `void OLED_C_WriteChar(uint8_t x, uint8_t y, char ch, FontDef_t *font, uint16_t color, uint16_t bg_color)`

Renders an ASCII character onto the pixel buffer at specified coordinates using a font bitmap definition.

* **Parameters:**
* `x`: X position offset.
* `y`: Y position offset.
* `ch`: ASCII character to render.
* `font`: Pointer to font structures defining width, height, and character pixel maps.
* `color`: Foreground text color.
* `bg_color`: Background color for text box masking.



---

#### `void OLED_C_WriteString(uint8_t x, uint8_t y, const char *str, FontDef_t *font, uint16_t color, uint16_t bg_color)`

Iterates through a null-terminated string and writes each character sequentially onto the screen.

* **Parameters:**
* `x`: Starting X coordinate.
* `y`: Starting Y coordinate.
* `str`: Pointer to null-terminated C string.
* `font`: Selected font definition structure.
* `color`: Text color.
* `bg_color`: Background color.



---

---

## 📄 `show_display.c`

### Overview

`show_display.c` forms the user-interface application layer. It acts as an abstraction layer between the measured environmental data produced by the **Sensirion SHT3x** temperature & humidity sensor driver and the low-level graphics functions in **`oled.c`**.

It handles string formatting (converting raw floating-point sensor values into human-readable text), UI screen layout/grouping, status bar updates, and screen refresh management.

---

### Key Data Structures

```c
typedef struct {
    float temperature; // Temperature in degrees Celsius
    float humidity;    // Relative Humidity in percentage (%RH)
    bool  is_valid;     // Flag signaling CRC check pass/fail
} SHT3x_DisplayData_t;

```

---

### Functions

#### `void ShowDisplay_Init(void)`

Prepares the user interface elements, initializes underlying display hardware, and renders the static UI layout (titles, background, labels, icons, borders).

* **Parameters:** None
* **Returns:** None
* **Behavior:**
1. Invokes `OLED_C_Init()` to bring up the screen controller.
2. Clears background color.
3. Draws main header box (e.g., `"SHT3x Monitor"`).
4. Renders static text labels such as `"Temp:"` and `"Humidity:"` along with unit markers (`"C"`, `"%"`).



---

#### `void ShowDisplay_UpdateData(float temperature, float humidity)`

Converts the latest floating-point sensor measurements into formatted strings and updates only the dynamic text regions of the screen to minimize redraw flicker.

* **Parameters:**
* `temperature`: Measured temperature value in °C.
* `humidity`: Measured relative humidity percentage.


* **Behavior:**
1. Formats `temperature` into string buffer with 1 decimal precision (e.g., `" 23.5 C"`).
2. Formats `humidity` into string buffer with 1 decimal precision (e.g., `" 48.2 %"`).
3. Overwrites previous dynamic text region using `OLED_C_WriteString()`.



---

#### `void ShowDisplay_ErrorState(const char *errorMessage)`

Renders an error notification interface on the display when an I2C communication fault or SHT3x CRC checksum failure occurs.

* **Parameters:**
* `errorMessage`: Pointer to string describing the operational failure (e.g., `"SENSOR ERROR"` or `"CRC FAIL"`).


* **Behavior:**
1. Clears display background or highlights text box in red/alert color.
2. Prints `errorMessage` at center of screen.



---

#### `void ShowDisplay_RenderHeader(const char *title)`

Draws the top title bar navigation area across the upper rows of the display grid.

* **Parameters:**
* `title`: Header text string to display.


* **Behavior:** Fills a top banner rectangle with a contrasting background color and prints header text centered within it.

---

### Execution Flow

```
      +------------------------+
      |  Main Loop / Sensor    |
      +-----------+------------+
                  |
                  v  (SHT3x Read Success)
      +-----------+------------+
      | ShowDisplay_UpdateData |
      +-----------+------------+
                  |
                  | (Formats floats -> strings)
                  v
      +-----------+------------+
      |    OLED_C_WriteString  |
      +-----------+------------+
                  |
                  | (Translates glyphs -> bytes)
                  v
      +-----------+------------+
      |  OLED_C_WriteCommand   |
      |   OLED_C_WriteBuffer   |
      +-----------+------------+
                  |
                  v  (SPI Transfer)
      +------------------------+
      |   SSD1351 OLED Panel   |
      +------------------------+

```
