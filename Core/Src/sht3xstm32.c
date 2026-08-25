/*
 * sht3_stm32_driver.c
 *
 *  Created on: 24.08.2026
 *      Author: Moritz Wieser
 */
#include <sht3xstm32.h>
#include <string.h>
#include <stdio.h>

/**
 * Registers addresses.
 * Defined all register adresses for later use.
 */
typedef enum
{
	SHT3xSTM32_COMMAND_MEASURE_HIGHREP_STRETCH = 0x2c06,
	SHT3xSTM32_COMMAND_CLEAR_STATUS = 0x3041,
	SHT3xSTM32_COMMAND_SOFT_RESET = 0x30A2,
	SHT3xSTM32_COMMAND_HEATER_ENABLE = 0x306d,
	SHT3xSTM32_COMMAND_HEATER_DISABLE = 0x3066,
	SHT3xSTM32_COMMAND_READ_STATUS = 0xf32d,
	SHT3xSTM32_COMMAND_FETCH_DATA = 0xe000,
	SHT3xSTM32_COMMAND_MEASURE_HIGHREP_10HZ = 0x2737,
	SHT3xSTM32_COMMAND_MEASURE_LOWREP_10HZ = 0x272a
} SHT3xSTM32_command_t;

/* Time to wait after a soft-reset command before the sensor is ready
 * again. Datasheet specifies ~1.5ms; 2ms is used to leave a bit of margin. */
#define SHT3xSTM32_SOFT_RESET_DELAY_MS 2

/* How often the debug task auto-prints a reading, in ms. */
#define SHT3xSTM32_DEBUG_AUTOPRINT_INTERVAL_MS 1000

/* Module-local handle for the debug/test UART, set once via
 * SHT3xSTM32_UART_Init(). Kept static so it's private to this file. */

static UART_HandleTypeDef *sht3xstm32_debug_uart = NULL;
static SHT3xSTM32_Handle_t *sht3xstm32_debug_handle = NULL;

/**
 * Splits a 16-bit command into its MSB/LSB bytes for I2C transmission.
 * bytes must point to a buffer of at least 2 uint8_t.
 */
static void SHT3xSTM32_CommandToBytes(SHT3xSTM32_command_t cmd, uint8_t *bytes)
{
    bytes[0] = (uint8_t)(cmd >> 8);     // MSB
    bytes[1] = (uint8_t)(cmd & 0xFF);   // LSB
}


HAL_StatusTypeDef SHT3xSTM32_Init( SHT3xSTM32_Handle_t *handle, 	I2C_HandleTypeDef *hi2c, 	uint8_t deviceAddress){

		// Reject invalid pointers
		if(handle == NULL || hi2c == NULL){
			return HAL_ERROR;
		}

		handle->hi2c = hi2c;
		handle->deviceAddress = deviceAddress;

		// Check sensor responds on the bus
		if (HAL_I2C_IsDeviceReady(handle->hi2c, handle->deviceAddress << 1u,
										   3, 10) != HAL_OK) {
			return HAL_ERROR;
		}

		// Soft-reset the sensor to a known state
		uint8_t resetCmd[2];
		SHT3xSTM32_CommandToBytes(SHT3xSTM32_COMMAND_SOFT_RESET, resetCmd);
		if (HAL_I2C_Master_Transmit(
				handle->hi2c,
				(uint16_t)(handle->deviceAddress << 1u),
				resetCmd, sizeof(resetCmd), 10) != HAL_OK) {

		    		return HAL_ERROR;
		}
		// Give the sensor a moment to come back up after the reset
		HAL_Delay(SHT3xSTM32_SOFT_RESET_DELAY_MS);
		handle->initialized = 1;
		return HAL_OK;

}


HAL_StatusTypeDef SHT3xSTM32_Read(SHT3xSTM32_Handle_t *handle, SHT3xSTM32_Data_t *data)
{
    //Guard against null pointers
	if(handle == NULL || data == NULL){
		return HAL_ERROR;
	}
    if (!handle->initialized) {
        return HAL_ERROR;
    }
    //Send measurement command (high repeatability, clock stretching)
	uint8_t measureCmd[2];
	SHT3xSTM32_CommandToBytes(SHT3xSTM32_COMMAND_MEASURE_HIGHREP_STRETCH, measureCmd);
	if (HAL_I2C_Master_Transmit(
			handle->hi2c,
			(uint16_t)(handle->deviceAddress << 1u),
			measureCmd,
			sizeof(measureCmd), 10) != HAL_OK) {

	    		return HAL_ERROR;
	}
    //Wait ~15ms for the sensor to finish converting
	HAL_Delay(SHT3_STM32_TIMEOUT);
    //Read back 6 bytes into a buffer
	uint8_t buffer[6];
	if (HAL_I2C_Master_Receive(handle->hi2c, handle->deviceAddress << 1u, buffer, sizeof(buffer), SHT3_STM32_TIMEOUT) != HAL_OK) {
		return HAL_ERROR;
	}
    //Verify CRC for temp bytes and humidity bytes
	uint8_t temperatureCrc = SHT3xSTM32_CalculateCRC(buffer, 2);
	uint8_t humidityCrc = SHT3xSTM32_CalculateCRC(buffer + 3, 2);
	if (temperatureCrc != buffer[2] || humidityCrc != buffer[5]) {
		return HAL_ERROR;
	}
	uint16_t temperatureRaw = SHT3xSTM32_BytesToUint16(buffer[0], buffer[1]);
	uint16_t humidityRaw = SHT3xSTM32_BytesToUint16(buffer[3], buffer[4]);
    //Convert raw values to °C / %RH
	data->humidity = SHT3xSTM32_ConvertHumidity(humidityRaw);
	data->temperature =SHT3xSTM32_ConvertTemperature(temperatureRaw);


    //return HAL_OK
	return HAL_OK;
}

uint8_t SHT3xSTM32_CalculateCRC(uint8_t *data, uint8_t length)
{
    // Starting value per the SHT3x datasheet's CRC-8 definition
    uint8_t crc = 0xFF;

    for (uint8_t i = 0; i < length; i++) {
        // Mix the next byte in
        crc ^= data[i];

        for (uint8_t bit = 0; bit < 8; bit++) {
				// Shift left, XOR with the poly (0x31) whenever the top bit is set
				if (crc & 0x80) {
					crc = (crc << 1) ^ 0x31;
				} else {
					crc = crc << 1;
				}
        }
    }

    return crc;
}

uint16_t SHT3xSTM32_BytesToUint16(uint8_t byteHigh, uint8_t byteLow)
{
    // High byte comes in first over I2C, so it goes in the top 8 bits
    return (uint16_t)(((uint16_t)byteHigh << 8u) | byteLow);
}

float SHT3xSTM32_ConvertTemperature(uint16_t rawTemp){
	// Straight from the datasheet's raw-to-°C formula
	return -45.0f + 175.0f * (float)rawTemp / 65535.0f;
}

float SHT3xSTM32_ConvertHumidity(uint16_t rawHumidity){
	// Straight from the datasheet's raw-to-%RH formula
	return 100.0f * (float)rawHumidity / 65535.0f;
}


HAL_StatusTypeDef SHT3xSTM32_SetHeater(SHT3xSTM32_Handle_t *handle, uint8_t enable)
{
    if (handle == NULL) {
        return HAL_ERROR;
    }
    if (!handle->initialized) {
        return HAL_ERROR;
    }
    SHT3xSTM32_command_t heaterCmd;

	if (enable) {
		 heaterCmd = SHT3xSTM32_COMMAND_HEATER_ENABLE;
	} else {
		 heaterCmd = SHT3xSTM32_COMMAND_HEATER_DISABLE;
	}

    uint8_t cmd[2];
    SHT3xSTM32_CommandToBytes(heaterCmd, cmd);

    return HAL_I2C_Master_Transmit(
        handle->hi2c,
        (uint16_t)(handle->deviceAddress << 1u),
        cmd, sizeof(cmd), SHT3_STM32_TIMEOUT
    );
}

HAL_StatusTypeDef SHT3xSTM32_HeaterTest(SHT3xSTM32_Handle_t *handle)
{
    if (handle == NULL) {
        return HAL_ERROR;
    }

    SHT3xSTM32_Data_t before, after;
    HAL_StatusTypeDef status;

    status = SHT3xSTM32_Read(handle, &before);
    if (status != HAL_OK){
    		return HAL_ERROR;
    }
    status = SHT3xSTM32_SetHeater(handle, 1);
    	if (status != HAL_OK){
    		return HAL_ERROR;
    }


    HAL_Delay(10000); // let the heater raise temp measurably

    status = SHT3xSTM32_Read(handle, &after);
    SHT3xSTM32_SetHeater(handle, 0); // always turn heater back off, even on read error

    if (status != HAL_OK){
    		return HAL_ERROR;
    }
    float delta = after.temperature - before.temperature;

    char buf[80];
    int len = snprintf(buf, sizeof(buf),
                        "Heater test: %.2f C -> %.2f C (delta %.2f C)\r\n",
                        before.temperature, after.temperature, delta);
    if (len > 0) {
        SHT3xSTM32_UART_PrintString(buf);
    }

    // Pass if temperature rose by a meaningful margin (avoids false positives from noise)
    if (delta > 0.5f) {
        return HAL_OK;
    }

    return HAL_ERROR;
}


static void SHT3xSTM32_UART_PrintMenu(void)
{
    static const char menu[] =
        "\r\n"
        "==================================================\r\n"
        " SHT3x Debug Menu\r\n"
        "==================================================\r\n"
        "   r - take one reading and print it\r\n"
        "   h - run heater self-test (PASS/FAIL)\r\n"
    		"   q - quite test enviorment\r\n"
        "==================================================\r\n";

    if (sht3xstm32_debug_uart == NULL) {
        return;
    }
    HAL_UART_Transmit(sht3xstm32_debug_uart, (uint8_t *)menu, sizeof(menu) - 1, HAL_MAX_DELAY);
}

/* Private: transmit a plain string over the debug UART. */
static void SHT3xSTM32_UART_PrintString(const char *msg)
{
    if (sht3xstm32_debug_uart == NULL || msg == NULL) {
        return;
    }
    HAL_UART_Transmit(sht3xstm32_debug_uart, (uint8_t *)msg, (uint16_t)strlen(msg), SHT3_STM32_TIMEOUT);
}
/* Private: format one reading and send it over the debug UART. */
static void SHT3xSTM32_UART_PrintReading(const SHT3xSTM32_Data_t *data)
{
    if (sht3xstm32_debug_uart == NULL || data == NULL) {
        return;
    }
    char buf[64];
    int len = snprintf(buf, sizeof(buf),
                        "T: %.2f C, RH: %.2f %%RH\r\n",
                        data->temperature, data->humidity);
    if (len > 0) {
        HAL_UART_Transmit(sht3xstm32_debug_uart, (uint8_t *)buf, (uint16_t)len, HAL_MAX_DELAY);
    }
}

HAL_StatusTypeDef SHT3xSTM32_Debug_Init(UART_HandleTypeDef *uart, SHT3xSTM32_Handle_t *handle)
{
    if (uart == NULL || handle == NULL) {
        return HAL_ERROR;
    }

    sht3xstm32_debug_uart   = uart;
    sht3xstm32_debug_handle = handle;

    return HAL_OK;
}

void SHT3xSTM32_UART_MenuHandler(void)
{

    if (sht3xstm32_debug_uart == NULL || sht3xstm32_debug_handle == NULL) {
        return;
    }

    uint8_t rxByte;
    uint8_t running = 1;

    while (running)
    {
        SHT3xSTM32_UART_PrintMenu();

        HAL_UART_Receive(sht3xstm32_debug_uart, &rxByte, 1, HAL_MAX_DELAY);

        switch (rxByte)
        {
            case 'r': {
                SHT3xSTM32_Data_t data;
                if (SHT3xSTM32_Read(sht3xstm32_debug_handle, &data) == HAL_OK) {
                    SHT3xSTM32_UART_PrintReading(&data);
                } else {
                    SHT3xSTM32_UART_PrintString("Read failed\r\n");
                }
                break;
            }

            case 'h': {
                SHT3xSTM32_UART_PrintString("Running heater test...\r\n");
                if (SHT3xSTM32_HeaterTest(sht3xstm32_debug_handle) == HAL_OK) {
                    SHT3xSTM32_UART_PrintString("Heater test: PASS\r\n");
                } else {
                    SHT3xSTM32_UART_PrintString("Heater test: FAIL\r\n");
                }
                break;
            }

            case 'x':
            case 'q':
                SHT3xSTM32_UART_PrintString("Exiting menu\r\n");
                running = 0;
                break;

            default:
                SHT3xSTM32_UART_PrintString("Unknown command\r\n");
                break;
        }
    }
}


