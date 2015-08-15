/**
 ******************************************************************************
 * @file	simple_gui.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-06-14
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
#ifndef SIMPLE_GUI_H_
#define SIMPLE_GUI_H_

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "stm32f4xx_hal.h"
#include "simple_gui_config.h"

#include "color.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* Defines -------------------------------------------------------------------*/
#define IS_GUI_WRITE_FORMAT(X)	(((X) == GUIWriteFormat_ASCII) || \
								 ((X) == GUIWriteFormat_Hex))

/* Typedefs ------------------------------------------------------------------*/
typedef enum
{
	GUIButtonState_NoState,
	GUIButtonState_Enabled,
	GUIButtonState_Disabled,
	GUIButtonState_TouchUp,
	GUIButtonState_TouchDown,
	GUIButtonState_DisabledTouch,
} GUIButtonState;

typedef enum
{
	GUITouchEvent_Down,
	GUITouchEvent_Up,
} GUITouchEvent;

typedef enum
{
	GUILayer_0,
	GUILayer_1,
} GUILayer;

typedef enum
{
	GUIDisplayState_Hidden,
	GUIDisplayState_NotHidden,
	GUIDisplayState_ContentHidden,
	GUIDisplayState_NoState,
} GUIDisplayState;

typedef enum
{
	GUIBorder_NoBorder = 0x00,
	GUIBorder_Left = 0x01,
	GUIBorder_Right = 0x02,
	GUIBorder_Top = 0x04,
	GUIBorder_Bottom = 0x08,
} GUIBorder;

typedef enum
{
	GUIHideState_HideAll,
	GUIHideState_KeepBorders,
} GUIHideState;

typedef enum
{
	GUIWriteFormat_ASCII = LCDWriteFormat_ASCII,
	GUIWriteFormat_Hex = LCDWriteFormat_Hex,
} GUIWriteFormat;

typedef enum
{
	GUIContainerPage_None = 0x0000,
	GUIContainerPage_1 = 0x0001,
	GUIContainerPage_2 = 0x0002,
	GUIContainerPage_3 = 0x0004,
	GUIContainerPage_4 = 0x0008,
	GUIContainerPage_5 = 0x0010,
	GUIContainerPage_6 = 0x0020,
	GUIContainerPage_7 = 0x0040,
	GUIContainerPage_8 = 0x0080,
	GUIContainerPage_9 = 0x0100,
	GUIContainerPage_10 = 0x0200,
	GUIContainerPage_11 = 0x0400,
	GUIContainerPage_12 = 0x0800,
	GUIContainerPage_All = 0xFFFF,
} GUIContainerPage;

/*
 * @name	GUIObject
 * @brief	-	The basic object i Simple GUI. All other elements have a GUIObject in them.
 * 			-	The GUIObject manages the position and size of the object and it's border.
 */
typedef struct
{
	/* Unique ID set in simple_gui_config.h for each GUI object */
	uint32_t id;

	/* Position and size */
	uint16_t xPos;
	uint16_t yPos;
	uint16_t width;
	uint16_t height;

	/* Layer where the object is */
	GUILayer layer;

	/* The display state of the object */
	GUIDisplayState displayState;

	/* Border */
	GUIBorder border;
	uint32_t borderThickness;
	uint16_t borderColor;

	/* Which page in the container the object should be on */
	GUIContainerPage containerPage;
} GUIObject;

/*
 * @name	GUIButton
 * @brief	- 	A button with a callback function which are called when running the function
 * 				GUI_CheckAllActiveButtonsForTouchEventAt() with appropriate arguments.
 * 			- 	A button can have either one or two rows of text. No check is done to make
 * 				sure the text will fit inside the button so the user has to make sure the
 * 				button is big enough.
 * 			- 	The maximum length of the text is determined by the text variable you send when
 * 				calling the GUI_AddButton function. You have to make sure you don't send any bigger
 * 				text than this using the GUI_SetButtonTextForRow function as that will probably
 * 				corrupt data! Think of it as static from when compiling.
 */
typedef struct
{
	/* Basic information about the object */
	GUIObject object;

	/* Colors */
	uint16_t enabledTextColor;
	uint16_t enabledBackgroundColor;
	uint16_t disabledTextColor;
	uint16_t disabledBackgroundColor;
	uint16_t pressedTextColor;
	uint16_t pressedBackgroundColor;

	/* The state which the button is in */
	GUIButtonState state;

	/* Pointer to a callback function called when a touch event has happened */
	void (*touchCallback)(GUITouchEvent, uint32_t);

	/* Text - Two rows of text can be displayed and it must have at least one row */
	uint8_t* text[2];
	LCDFontEnlarge textSize[2];

	uint32_t numOfChar[2];		/* These three are calculated automatically in GUI_AddButton */
	uint32_t textWidth[2];		/* --------------------------------------------------------- */
	uint32_t textHeight[2];		/* --------------------------------------------------------- */
} GUIButton;

/*
 * @name	GUITextBox
 * @brief	- 	A box that can display text of arbitrary length. When the text cursor reaches
 * 				the end of the text box size it will wrap around to the start again meaning it will
 * 				write over whatever text was written there.
 * 			- 	You can also create a static text box by setting the staticText when you add it.
 * 				This will make sure the text is centered in the box and written every time you call
 * 				the GUI_DrawTextBox. This can be useful if you want to display a label. You should
 * 				not call any GUI_Write function for a static text box.
 */
typedef struct
{
	/* Basic information about the object */
	GUIObject object;

	/* Colors */
	uint16_t textColor;
	uint16_t backgroundColor;

	/* Text */
	uint8_t* staticText;
	LCDFontEnlarge textSize;

	uint32_t staticTextNumOfChar;	/* These three are calculated automatically in GUI_AddTextBox */
	uint32_t staticTextWidth;		/* --------------------------------------------------------- */
	uint32_t staticTextHeight;		/* --------------------------------------------------------- */

	/* Position where the next character will be written. Referenced from the objects origin (xPos, yPos) */
	uint16_t xWritePos;
	uint16_t yWritePos;

	/* Pointer to a callback function called when a touch event has happened */
	void (*touchCallback)(GUITouchEvent, uint16_t, uint16_t);
} GUITextBox;

/*
 * @name	GUIContainer
 * @brief	- 	A collection of other GUI items to more easily hide/show groups of items.
 * 			- 	When a container is drawn it will draw all of it's containing elements as well.
 * 				The same happens when it is hidden.
 */
typedef struct GUIContainer GUIContainer;	/* We need to typedef here because a GUIContainer can contain other GUIContainers */
struct GUIContainer
{
	/* Basic information about the object */
	GUIObject object;

	GUIHideState contentHideState;

	/* Colors */
	uint16_t backgroundColor;

	/* Store a pointer to all the object on the page for easy access and small footprint */
	GUIButton* buttons[guiConfigNUMBER_OF_BUTTONS];
	GUITextBox* textBoxes[guiConfigNUMBER_OF_TEXT_BOXES];
	GUIContainer* containers[guiConfigNUMBER_OF_CONTAINERS];

	/* The active page of the container, starts at GUIContainerPage_None */
	GUIContainerPage activePage;
	GUIContainerPage lastPage;

	/* Pointer to a callback function called when a touch event has happened */
	void (*touchCallback)(GUITouchEvent, uint16_t, uint16_t);
};

/* Function prototypes -------------------------------------------------------*/
void GUI_Init();
void GUI_DrawBorder(GUIObject Object);
void GUI_RedrawLayer(GUILayer Layer);
void GUI_SetActiveLayer(GUILayer Layer);
GUILayer GUI_GetActiveLayer();

/* Button functions */
GUIButton* GUI_GetButtonFromId(uint32_t ButtonId);
ErrorStatus GUI_AddButton(GUIButton* Button);
void GUI_HideButton(uint32_t ButtonId);
ErrorStatus GUI_DrawButton(uint32_t ButtonId);
void GUI_DrawAllButtons();
void GUI_SetButtonState(uint32_t ButtonId, GUIButtonState State);
void GUI_SetButtonTextForRow(uint32_t ButtonId, uint8_t* Text, uint32_t Row);
void GUI_CheckAllActiveButtonsForTouchEventAt(GUITouchEvent Event, uint16_t XPos, uint16_t YPos);
GUIDisplayState GUI_GetDisplayStateForButton(uint32_t ButtonId);
void GUI_SetLayerForButton(uint32_t ButtonId, GUILayer Layer);

/* Text box functions */
GUITextBox* GUI_GetTextBoxFromId(uint32_t TextBoxId);
ErrorStatus GUI_AddTextBox(GUITextBox* TextBox);
void GUI_HideTextBox(uint32_t TextBoxId);
ErrorStatus GUI_DrawTextBox(uint32_t TextBoxId);
void GUI_DrawAllTextBoxes();
ErrorStatus GUI_WriteStringInTextBox(uint32_t TextBoxId, uint8_t* String);
ErrorStatus GUI_WriteBufferInTextBox(uint32_t TextBoxId, uint8_t* pBuffer, uint32_t Size, GUIWriteFormat Format);
ErrorStatus GUI_WriteNumberInTextBox(uint32_t TextBoxId, int32_t Number);
ErrorStatus GUI_SetStaticTextInTextBox(uint32_t TextBoxId, uint8_t* String);
void GUI_SetWritePosition(uint32_t TextBoxId, uint16_t XPos, uint16_t YPos);
void GUI_SetYWritePositionToCenter(uint32_t TextBoxId);
void GUI_GetWritePosition(uint32_t TextBoxId, uint16_t* XPos, uint16_t* YPos);
ErrorStatus GUI_ClearTextBox(uint32_t TextBoxId);
ErrorStatus GUI_ClearAndResetTextBox(uint32_t TextBoxId);
void GUI_CheckAllActiveTextBoxesForTouchEventAt(GUITouchEvent Event, uint16_t XPos, uint16_t YPos);
GUIDisplayState GUI_GetDisplayStateForTextBox(uint32_t TextBoxId);

/* Container functions */
GUIContainer* GUI_GetContainerFromId(uint32_t ContainerId);
ErrorStatus GUI_AddContainer(GUIContainer* Container);
void GUI_HideContentInContainer(uint32_t ContainerId);
void GUI_HideContainer(uint32_t ContainerId);
ErrorStatus GUI_DrawContainer(uint32_t ContainerId);
void GUI_ChangePageOfContainer(uint32_t ContainerId, GUIContainerPage NewPage);
GUIContainerPage GUI_GetActivePageOfContainer(uint32_t ContainerId);
GUIContainerPage GUI_GetLastPageOfContainer(uint32_t ContainerId);
void GUI_IncreasePageOfContainer(uint32_t ContainerId);
void GUI_DecreasePageOfContainer(uint32_t ContainerId);
void GUI_CheckAllContainersForTouchEventAt(GUITouchEvent Event, uint16_t XPos, uint16_t YPos);
GUIDisplayState GUI_GetDisplayStateForContainer(uint32_t ContainerId);


#endif /* SIMPLE_GUI_H_ */
