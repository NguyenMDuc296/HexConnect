/**
 ******************************************************************************
 * @file	spi_flash_memory_map.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-09-09
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
#ifndef SPI_FLASH_MEMORY_MAP_H_
#define SPI_FLASH_MEMORY_MAP_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "can_common.h"
#include "uart_common.h"

/* Defines -------------------------------------------------------------------*/
#define FLASH_ADR_CHANNEL_SETTINGS	(0x001000)
#define FLASH_ADR_CAN1_SETTINGS		(FLASH_ADR_CHANNEL_SETTINGS)
#define FLASH_ADR_CAN2_SETTINGS		(FLASH_ADR_CHANNEL_SETTINGS + sizeof(CANSettings))
#define FLASH_ADR_UART1_SETTINGS	(FLASH_ADR_CHANNEL_SETTINGS + 2*sizeof(CANSettings))
#define FLASH_ADR_UART2_SETTINGS	(FLASH_ADR_CHANNEL_SETTINGS + 2*sizeof(CANSettings) + sizeof(UARTSettings))
#define FLASH_ADR_RS232_SETTINGS	(FLASH_ADR_CHANNEL_SETTINGS + 2*sizeof(CANSettings) + 2*sizeof(UARTSettings))

#define FLASH_ADR_CAN1_DATA			(0x010000)
#define FLASH_ADR_CAN2_DATA			(0x110000)
#define FLASH_ADR_UART1_DATA		(0x210000)
#define FLASH_ADR_UART2_DATA		(0x310000)
#define FLASH_ADR_RS232_DATA		(0x410000)
#define FLASH_ADR_GPIO0_DATA		(0x510000)
#define FLASH_ADR_GPIO1_DATA		(0x610000)
#define FLASH_ADR_ADC_DATA			(0x710000)
#define FLASH_ADR_THERM_DATA		(0x810000)

/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/


#endif /* SPI_FLASH_MEMORY_MAP_H_ */
