/**
 *******************************************************************************
 * @file  background_task.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date  2015-08-15
 * @brief
 *******************************************************************************
  Copyright (c) 2015 Hampus Sandberg.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "background_task.h"

#include "mcp9808.h"
#include "spi_flash.h"

#include <string.h>

/* Private defines -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#define BUFFER_SIZE		1024
uint8_t rxData[BUFFER_SIZE] = {0x00};

/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	Text
 * @param	None
 * @retval	None
 */
void backgroundTask(void *pvParameters)
{
	prvHardwareInit();

	/* The parameter in vTaskDelayUntil is the absolute time
	 * in ticks at which you want to be woken calculated as
	 * an increment from the time you were last woken. */
	TickType_t xNextWakeTime;
	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	SPI_FLASH_Init();

//	SPI_FLASH_EraseSector(FLASH_ADR_THERM_DATA);

	float currentTemp = 0;

	while (1)
	{
		/* Only send a message if the queue exists */
		if (xLCDEventQueue != 0)
		{
			/* Read and send the temperature */
			currentTemp = MCP9808_GetTemperature();
			LCDEventMessage message;
			message.event = LCDEvent_TemperatureData;
			memcpy(message.data, &currentTemp, sizeof(float));
			xQueueSendToBack(xLCDEventQueue, &message, 100);
		}


		/* LED on for 500 ms */
		HAL_GPIO_WritePin(GPIOC, backgroundLED_0, GPIO_PIN_RESET);
		vTaskDelayUntil(&xNextWakeTime, 500 / portTICK_PERIOD_MS);

		/* LED off for 500 ms */
		HAL_GPIO_WritePin(GPIOC, backgroundLED_0, GPIO_PIN_SET);
		vTaskDelayUntil(&xNextWakeTime, 500 / portTICK_PERIOD_MS);
	}
}

/* Private functions .--------------------------------------------------------*/
/**
 * @brief	Initializes the hardware
 * @param	None
 * @retval	None
 */
static void prvHardwareInit()
{
	/* Set up the LED outputs */
	__GPIOC_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin  	= backgroundLED_0 | backgroundLED_1 | backgroundLED_2;
	GPIO_InitStructure.Mode  	= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull		= GPIO_NOPULL;
	GPIO_InitStructure.Speed 	= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

	HAL_GPIO_WritePin(GPIOC, backgroundLED_0, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, backgroundLED_1, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, backgroundLED_2, GPIO_PIN_SET);

	/* Temperature Sensor init */
	MCP9808_Init();
}

/* Interrupt Handlers --------------------------------------------------------*/
