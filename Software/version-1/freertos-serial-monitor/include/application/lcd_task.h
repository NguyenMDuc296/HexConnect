/**
 ******************************************************************************
 * @file	lcd_task.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-09-07
 * @brief
 ******************************************************************************
	Copyright (c) 2014 Hampus Sandberg.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation, either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef LCD_TASK_H_
#define LCD_TASK_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "messages.h"
#include "spi_flash_memory_map.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
void lcdTask(void *pvParameters);

void lcdGenericUartClearButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void lcdManageGenericUartMainTextBox(const uint32_t constStartFlashAddress, uint32_t currentWriteAddress, UARTSettings* pSettings,
									 SemaphoreHandle_t* pSemaphore, uint32_t TextBoxId, bool ShouldRefresh);
void lcdChangeDisplayStateOfSidebar(uint32_t SidebarId);
void lcdForceRefreshOfActiveMainContent();


#endif /* LCD_TASK_H_ */
