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

# DISPALAY PART
## oled.c
oled.c is based on the supplied library from Moodle. It was slightly changed, so that the coordinates we later use actually land on the Display and the whole Display is toggled.
We also Added Text rendering via Bitmasks with 5x7 px per Character. 

The included Startup animation wasn't necessary but considered nice.
## show_display.c
Naming of the two files could have been better. The functions in this file accomplish the higher Level Display tasks, like toggling the display modes.
The functions called in main correspond to this file. 
