/**
 ******************************************************************************
 * @file	gpio.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-08-23
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
#ifndef GPIO_H_
#define GPIO_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
typedef struct
{
	GPIO_InitTypeDef* GPIO_Init;	/* Init structure */
	GPIO_TypeDef* GPIO_Port;		/* GPIO port */
	uint16_t GPIO_Pin;				/* GPIO pin */
} GPIO_Device;

/* Function prototypes -------------------------------------------------------*/


#endif /* GPIO_H_ */
