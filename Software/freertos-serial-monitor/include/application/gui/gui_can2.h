/**
 ******************************************************************************
 * @file	gui_can2.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-09-21
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
#ifndef GUI_CAN2_H_
#define GUI_CAN2_H_

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "stm32f4xx_hal.h"

#include "can2_task.h"
#include "lcd_task.h"
#include "simple_gui.h"
#include "simple_gui_config.h"

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/
void guiCan2ManageMainTextBox();
void guiCan2EnableButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiCan2TerminationButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiCan2TopButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiCan2BitRateButtonCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiCan2BitRateSelectionCallback(GUITouchEvent Event, uint32_t ButtonId);
void guiCan2UpdateGuiElementsReadFromSettings();
void guiCan2InitGuiElements();



#endif /* GUI_CAN2_H_ */
