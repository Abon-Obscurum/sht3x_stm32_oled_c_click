#ifndef INC_SHT3XSTM32_H_
#define INC_SHT3XSTM32_H_

#include <stdint.h>
#include "stm32l4xx_hal.h"   // for I2C_HandleTypeDef, HAL_StatusTypeDef


/**
 * @brief Holds everything the driver needs to talk to one sensor.
 *
 * One of these gets created per sensor and a pointer is passed around
 * instead of using global variables - that way multiple SHT3x sensors on
 * different addresses/buses could be handled if that's ever needed.
 */
typedef struct {
    I2C_HandleTypeDef *hi2c;      /**< I2C peripheral used to talk to the sensor (e.g. &hi2c1). */
    uint8_t deviceAddress;        /**< I2C address of the sensor. */
    uint8_t initialized;          /**< 1 once SHT3xSTM32_Init() has run successfully, 0 otherwise. */
} SHT3xSTM32_Handle_t;


/**
 * @brief One finished sensor reading, already converted to real units.
 */
typedef struct {
    float temperature;   /**< Temperature in °C. */
    float humidity;       /**< Relative humidity in % (0-100). */
} SHT3xSTM32_Data_t;

/** How long we wait for an I2C transfer before giving up (ms). */
#define SHT3_STM32_TIMEOUT 30

/**
 * @brief  Sets up a sensor handle and makes sure the sensor actually answers.
 *
 * Call this once per sensor before doing any reads. It also sends a
 * soft-reset command, which puts the sensor back into its default state
 * (status register cleared, heater off, etc.), so the driver isn't
 * relying on whatever state it happened to power up in.
 *
 * @param  handle         Handle to fill in; reused by later calls like SHT3xSTM32_Read().
 * @param  hi2c            I2C peripheral the sensor is wired to.
 * @param  deviceAddress   I2C address of the sensor.
 * @return HAL_OK if the sensor was found and reset OK, an error code otherwise.
 */
HAL_StatusTypeDef SHT3xSTM32_Init(
    SHT3xSTM32_Handle_t *handle,
    I2C_HandleTypeDef *hi2c,
    uint8_t deviceAddress
);

/**
 * @brief  Grabs one temperature/humidity reading from the sensor.
 *
 * Triggers a measurement, waits for it to finish, pulls the raw bytes
 * over I2C, checks the CRC, and converts everything into normal
 * human-readable values in @p data.
 *
 * @param  handle  Handle previously set up with SHT3xSTM32_Init().
 * @param  data    Where the finished reading gets written.
 * @return HAL_OK on success, an error code if the I2C transfer failed or
 *         the CRC didn't match.
 */
HAL_StatusTypeDef SHT3xSTM32_Read(
    SHT3xSTM32_Handle_t *handle,
    SHT3xSTM32_Data_t *data
);

/**
 * @brief  Turns a raw temperature code from the sensor into °C.
 *
 * The sensor doesn't just send a temperature - it sends a raw 16-bit
 * value that needs to go through the conversion formula from the
 * datasheet first.
 *
 * @param  rawTemp  Raw 16-bit temperature value read from the sensor.
 * @return Temperature in °C.
 */
float SHT3xSTM32_ConvertTemperature(
	uint16_t rawTemp
);

/**
 * @brief  Turns a raw humidity code from the sensor into %RH.
 *
 * Same idea as SHT3xSTM32_ConvertTemperature() - raw sensor value in,
 * normal 0-100% value out.
 *
 * @param  rawHumidity  Raw 16-bit humidity value read from the sensor.
 * @return Relative humidity in % (0-100).
 */
float SHT3xSTM32_ConvertHumidity(
	uint16_t rawHumidity
);

/**
 * @brief  Combines a high and low byte into one 16-bit value.
 *
 * The sensor always sends its 16-bit values as two separate bytes,
 * just a little helper that puts them back together.
 *
 * @param  byteHigh  Upper 8 bits (sent first by the sensor).
 * @param  byteLow   Lower 8 bits (sent second by the sensor).
 * @return The combined 16-bit value.
 */
uint16_t SHT3xSTM32_BytesToUint16(uint8_t byteHigh, uint8_t byteLow);

/**
 * @brief  Runs the sensor's CRC-8 check over a block of bytes.
 *
 * Every value the sensor sends is followed by a checksum byte. The
 * return value of this function should be compared against that
 * checksum byte - if they don't match, the reading is corrupted (noise
 * on the bus, bad connection, etc.) and should be thrown away.
 *
 * @param  data    Bytes to check.
 * @param  length  Number of bytes in @p data.
 * @return The calculated checksum. Compare it to the sensor's checksum byte.
 */
uint8_t SHT3xSTM32_CalculateCRC(uint8_t *data, uint8_t length);



/**
 * @brief  Turns the sensor's built-in heater on or off.
 *
 *
 * @param  handle  Handle previously set up with SHT3xSTM32_Init().
 * @param  enable  Non-zero to turn the heater on, 0 to turn it off.
 * @return HAL_OK if the command was sent successfully, an error code
 *         otherwise.
 */
HAL_StatusTypeDef SHT3xSTM32_SetHeater(SHT3xSTM32_Handle_t *handle, uint8_t enable);

/**
 * @brief  Self-test that verifies the heater is working.
 *
 * Takes a reading, enables the heater for ~1 second, takes a second
 * reading, then always disables the heater again.
 * The test passes only if temperature rose by more than 0.5 °C.
 *
 * @param  handle  Handle previously set up with SHT3xSTM32_Init().
 * @return HAL_OK if the heater measurably raised the temperature,
 *         HAL_ERROR if a read/transmit failed or the rise wasn't big
 *         enough to count as a pass.
 */
HAL_StatusTypeDef SHT3xSTM32_HeaterTest(SHT3xSTM32_Handle_t *handle);

/**
 * @brief  Runs the interactive UART debug menu until the user exits.
 *
 * Prints the menu, blocks waiting for one keypress, and dispatches it:
 * 'r' takes and prints one reading, 'h' runs the heater self-test, and
 * 'q'/'x' exits the loop. Requires SHT3xSTM32_Debug_Init() to have been
 * called first — if it hasn't, the function returns immediately without
 * doing anything.
 *
 * @return None. Blocks until the user exits the menu.
 */
void SHT3xSTM32_UART_MenuHandler(void);

/**
 * @brief  Prints the debug menu text over the debug UART.
 *
 * Private helper used by SHT3xSTM32_UART_MenuHandler() before each
 * keypress is read. Does nothing if no debug UART has been registered
 * via SHT3xSTM32_Debug_Init().
 *
 * @return None.
 */
static void SHT3xSTM32_UART_PrintMenu(void);

/**
 * @brief  Sends a plain, null-terminated string over the debug UART.
 *
 * Private helper shared by the menu and other debug output (e.g. read
 * failures, the heater self-test result). Does nothing if the debug
 * UART hasn't been registered, or if @p msg is NULL.
 *
 * @param  msg  Null-terminated string to transmit.
 * @return None.
 */
static void SHT3xSTM32_UART_PrintString(const char *msg);

/**
 * Registers the UART and sensor handle used by the debug/test menu.
 * Must be called once before SHT3xSTM32_UART_MenuHandler().
 *
 * @param uart   UART handle for the debug console
 * @param handle Initialized sensor handle (must already have gone
 *               through SHT3xSTM32_Init())
 * @return HAL_OK on success, HAL_ERROR if either pointer is NULL
 */
HAL_StatusTypeDef SHT3xSTM32_Debug_Init(UART_HandleTypeDef *uart, SHT3xSTM32_Handle_t *handle);

/**
 * @brief  Formats one sensor reading and sends it over the debug UART.
 *
 * Builds a line like "T: 23.45 C, RH: 41.20 %RH\r\n" and transmits it
 * via the UART registered in SHT3xSTM32_Debug_Init(). Used internally
 * by SHT3xSTM32_UART_MenuHandler() after a successful read.
 *
 * @param  data  Reading to print. If NULL (or if no debug UART has been
 *               registered yet), the function does nothing.
 * @return None.
 */
static void SHT3xSTM32_UART_PrintReading(const SHT3xSTM32_Data_t *data);
#endif /* INC_SHT3XSTM32_H_ */
