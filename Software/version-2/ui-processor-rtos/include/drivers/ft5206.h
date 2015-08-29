/**
 *******************************************************************************
 * @file    ft5206.h
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-16
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

/** Define to prevent recursive inclusion ------------------------------------*/
#ifndef FT5206_H_
#define FT5206_H_

/** Includes -----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "messages.h"

/** Defines ------------------------------------------------------------------*/
/** Typedefs -----------------------------------------------------------------*/
typedef enum
{
  FT5206Point_1 = 1,
  FT5206Point_2,
  FT5206Point_3,
  FT5206Point_4,
  FT5206Point_5,
} FT5206Point;

typedef enum
{
  FT5206Event_PutDown = 0,
  FT5206Event_PutUp = 1,
  FT5206Event_Contact = 2,
} FT5206Event;

typedef enum
{
  FT5206InterruptMode_Polling,
  FT5206InterruptMode_Trigger,
} FT5206InterruptMode;

typedef struct
{
  uint16_t x;
  uint16_t y;
} FT5206TouchCoordinate;

typedef struct
{
  FT5206Event Event;
  FT5206TouchCoordinate Coordinates;
} FT5206TouchEvent;

/** Function prototypes ------------------------------------------------------*/
void FT5206_Init();
uint32_t FT5206_GetNumOfTouchPoints();
void FT5206_GetTouchDataForPoint(FT5206Event* pEvent, FT5206TouchCoordinate* pCoordinate, FT5206Point Point);
void FT5206_TestMode();

void CTP_INT_Callback();

#endif /* FT5206_H_ */
