/**
 *******************************************************************************
 * @file  gui_rs232.c
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
#include "gui_rs232.h"

#include "spi_flash.h"

/* Private defines -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static GUITextBox prvTextBox = {0};
static GUIButton prvButton = {0};
static GUIContainer prvContainer = {0};

/* Private function prototypes -----------------------------------------------*/
/* Functions -----------------------------------------------------------------*/
/* RS232 GUI Elements ========================================================*/
/**
 * @brief	Manages how data is displayed in the main text box when the source is RS232
 * @param	None
 * @retval	None
 */
void guiRs232ManageMainTextBox(bool ShouldRefresh)
{
	const uint32_t constStartFlashAddress = FLASH_ADR_RS232_DATA;

	/* Get the current write address, this is the address where the last data is */
	uint32_t currentWriteAddress = rs232GetCurrentWriteAddress();
	/* Get the current settings of the channel */
	UARTSettings* settings = rs232GetSettings();
	SemaphoreHandle_t* settingsSemaphore = rs232GetSettingsSemaphore();

	lcdManageGenericUartMainTextBox(constStartFlashAddress, currentWriteAddress, settings,
									settingsSemaphore, GUITextBoxId_Rs232Main, ShouldRefresh);

	/* Info textbox */
	static uint32_t lastAmountOfDataSaved = 1;
	static uint32_t lastFirstDataItem = 0;
	static uint32_t lastLastDataItem = 0;
	uint32_t firstDataItem = GUITextBox_GetNumberForFirstDisplayedData(GUITextBoxId_Rs232Main);
	uint32_t lastDataItem = GUITextBox_GetNumberForLastDisplayedData(GUITextBoxId_Rs232Main);

	if (ShouldRefresh || lastAmountOfDataSaved != settings->amountOfDataSaved ||
		lastFirstDataItem != firstDataItem || lastLastDataItem != lastDataItem)
	{
		lastAmountOfDataSaved = settings->amountOfDataSaved;
		GUITextBox_ClearAndResetWritePosition(GUITextBoxId_Rs232Info);
		GUITextBox_SetYWritePositionToCenter(GUITextBoxId_Rs232Info);
		GUITextBox_SetXWritePosition(GUITextBoxId_Rs232Info, 5);
		GUITextBox_WriteString(GUITextBoxId_Rs232Info, "Data Count: ");
		GUITextBox_WriteNumber(GUITextBoxId_Rs232Info, (int32_t)lastAmountOfDataSaved);
		GUITextBox_WriteString(GUITextBoxId_Rs232Info, " bytes, ");

		GUITextBox_WriteString(GUITextBoxId_Rs232Info, "Displayed data: ");
		if (lastAmountOfDataSaved != 0)
		{
			uint32_t firstDataItem = GUITextBox_GetNumberForFirstDisplayedData(GUITextBoxId_Rs232Main);
			uint32_t lastDataItem = GUITextBox_GetNumberForLastDisplayedData(GUITextBoxId_Rs232Main);
			GUITextBox_WriteNumber(GUITextBoxId_Rs232Info, (int32_t)firstDataItem);
			GUITextBox_WriteString(GUITextBoxId_Rs232Info, " to ");
			GUITextBox_WriteNumber(GUITextBoxId_Rs232Info, (int32_t)lastDataItem);
		}
		else
			GUITextBox_WriteString(GUITextBoxId_Rs232Info, "None");
		/* Update the variables */
		lastFirstDataItem = firstDataItem;
		lastLastDataItem = lastDataItem;
	}
}

/**
 * @brief	Callback for the enable button
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiRs232EnableButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	static bool enabled = false;

	if (Event == GUITouchEvent_Up)
	{
		if (enabled)
		{
			ErrorStatus status = rs232SetConnection(UARTConnection_Disconnected);
			if (status == SUCCESS)
			{
				enabled = false;
				GUIButton_SetTextForRow(GUIButtonId_Rs232Enable, "Disabled", 1);
				GUIButton_SetState(GUIButtonId_Rs232Top, GUIButtonState_Disabled);

				GUIButton_SetTextForRow(GUIButtonId_Rs232BaudRate, "< Baud Rate:", 0);
				GUIButton_SetState(GUIButtonId_Rs232BaudRate, GUIButtonState_Disabled);
				GUIButton_SetTextForRow(GUIButtonId_Rs232Parity, "< Parity:", 0);
				GUIButton_SetState(GUIButtonId_Rs232Parity, GUIButtonState_Disabled);
			}
		}
		else
		{
			ErrorStatus status = rs232SetConnection(UARTConnection_Connected);
			if (status == SUCCESS)
			{
				enabled = true;
				GUIButton_SetTextForRow(GUIButtonId_Rs232Enable, "Enabled", 1);
				GUIButton_SetState(GUIButtonId_Rs232Top, GUIButtonState_Enabled);

				GUIButton_SetTextForRow(GUIButtonId_Rs232BaudRate, "Baud Rate:", 0);
				GUIButton_SetState(GUIButtonId_Rs232BaudRate, GUIButtonState_DisabledTouch);
				GUIButton_SetTextForRow(GUIButtonId_Rs232Parity, "Parity:", 0);
				GUIButton_SetState(GUIButtonId_Rs232Parity, GUIButtonState_DisabledTouch);
			}
		}
	}
}

/**
 * @brief	Callback for the format button
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiRs232FormatButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		UARTSettings* settings = rs232GetSettings();
		SemaphoreHandle_t* settingsSemaphore = rs232GetSettingsSemaphore();
		/* Try to take the settings semaphore */
		if (*settingsSemaphore != 0 && xSemaphoreTake(*settingsSemaphore, 100) == pdTRUE)
		{
			if (settings->textFormat == GUITextFormat_ASCII)
			{
				settings->textFormat = GUITextFormat_HexWithSpaces;
				GUIButton_SetTextForRow(GUIButtonId_Rs232Format, "Hex", 1);
			}
			else if (settings->textFormat == GUITextFormat_HexWithSpaces)
			{
				settings->textFormat = GUITextFormat_ASCII;
				GUIButton_SetTextForRow(GUIButtonId_Rs232Format, "ASCII", 1);
			}

			/* Give back the semaphore now that we are done */
			xSemaphoreGive(*settingsSemaphore);

			/* Update the text format for the text box */
			GUITextBox_ChangeTextFormat(GUITextBoxId_Rs232Main, settings->textFormat, GUITextFormatChangeStyle_LockEnd);

			/* Refresh the main text box */
			guiRs232ManageMainTextBox(true);
		}
	}
}


/**
 * @brief	Callback for the debug button
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiRs232DebugButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	static bool enabled = false;

	if (Event == GUITouchEvent_Up)
	{
		UARTSettings* settings = rs232GetSettings();
		SemaphoreHandle_t* settingsSemaphore = rs232GetSettingsSemaphore();
		/* Try to take the settings semaphore */
		if (*settingsSemaphore != 0 && xSemaphoreTake(*settingsSemaphore, 100) == pdTRUE)
		{
			if (enabled)
			{
				settings->mode = UARTMode_TX_RX;
				enabled = false;
				GUIButton_SetTextForRow(GUIButtonId_Rs232Debug, "Disabled", 1);
			}
			else
			{
				settings->mode = UARTMode_DebugTX;
				enabled = true;
				GUIButton_SetTextForRow(GUIButtonId_Rs232Debug, "Enabled", 1);
			}

			/* Give back the semaphore now that we are done */
			xSemaphoreGive(*settingsSemaphore);
		}
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiRs232TopButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		/* Get the current display state of the sidebar */
		GUIDisplayState displayState = GUIContainer_GetDisplayState(GUIContainerId_SidebarRs232);
		/* Change the state of the sidebar */
		lcdChangeDisplayStateOfSidebar(GUIContainerId_SidebarRs232);
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiRs232BaudRateButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUIContainer_GetDisplayState(GUIContainerId_PopoutRs232BaudRate);

		if (displayState == GUIDisplayState_Hidden)
		{
			GUI_SetActiveLayer(GUILayer_1);
			GUIButton_SetLayer(GUIButtonId_Rs232BaudRate, GUILayer_1);
			GUIButton_SetState(GUIButtonId_Rs232BaudRate, GUIButtonState_Enabled);
			GUIContainer_Draw(GUIContainerId_PopoutRs232BaudRate);
		}
		else if (displayState == GUIDisplayState_NotHidden)
		{
			GUIContainer_Hide(GUIContainerId_PopoutRs232BaudRate);
			GUI_SetActiveLayer(GUILayer_0);
			GUIButton_SetLayer(GUIButtonId_Rs232BaudRate, GUILayer_0);
			GUIButton_SetState(GUIButtonId_Rs232BaudRate, GUIButtonState_Disabled);

			/* Refresh the main text box */
			guiRs232ManageMainTextBox(true);
		}
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiRs232ParityButtonCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		GUIDisplayState displayState = GUIContainer_GetDisplayState(GUIContainerId_PopoutRs232Parity);

		if (displayState == GUIDisplayState_Hidden)
		{
			GUI_SetActiveLayer(GUILayer_1);
			GUIButton_SetLayer(GUIButtonId_Rs232Parity, GUILayer_1);
			GUIButton_SetState(GUIButtonId_Rs232Parity, GUIButtonState_Enabled);
			GUIContainer_Draw(GUIContainerId_PopoutRs232Parity);
		}
		else if (displayState == GUIDisplayState_NotHidden)
		{
			GUIContainer_Hide(GUIContainerId_PopoutRs232Parity);
			GUI_SetActiveLayer(GUILayer_0);
			GUIButton_SetLayer(GUIButtonId_Rs232Parity, GUILayer_0);
			GUIButton_SetState(GUIButtonId_Rs232Parity, GUIButtonState_Disabled);

			/* Refresh the main text box */
			guiRs232ManageMainTextBox(true);
		}
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiRs232BaudRateSelectionCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		UARTBaudRate newBaudRate;
		switch (ButtonId)
		{
			case GUIButtonId_Rs232BaudRate4800:
				GUIButton_SetTextForRow(GUIButtonId_Rs232BaudRate, "4800 bps", 1);
				newBaudRate = UARTBaudRate_4800;
				break;
			case GUIButtonId_Rs232BaudRate7200:
				GUIButton_SetTextForRow(GUIButtonId_Rs232BaudRate, "7200 bps", 1);
				newBaudRate = UARTBaudRate_7200;
				break;
			case GUIButtonId_Rs232BaudRate9600:
				GUIButton_SetTextForRow(GUIButtonId_Rs232BaudRate, "9600 bps", 1);
				newBaudRate = UARTBaudRate_9600;
				break;
			case GUIButtonId_Rs232BaudRate19k2:
				GUIButton_SetTextForRow(GUIButtonId_Rs232BaudRate, "19200 bps", 1);
				newBaudRate = UARTBaudRate_19200;
				break;
			case GUIButtonId_Rs232BaudRate28k8:
				GUIButton_SetTextForRow(GUIButtonId_Rs232BaudRate, "28800 bps", 1);
				newBaudRate = UARTBaudRate_28800;
				break;
			case GUIButtonId_Rs232BaudRate38k4:
				GUIButton_SetTextForRow(GUIButtonId_Rs232BaudRate, "38400 bps", 1);
				newBaudRate = UARTBaudRate_38400;
				break;
			case GUIButtonId_Rs232BaudRate57k6:
				GUIButton_SetTextForRow(GUIButtonId_Rs232BaudRate, "57600 bps", 1);
				newBaudRate = UARTBaudRate_57600;
				break;
			case GUIButtonId_Rs232BaudRate115k:
				GUIButton_SetTextForRow(GUIButtonId_Rs232BaudRate, "115200 bps", 1);
				newBaudRate = UARTBaudRate_115200;
				break;
			default:
				newBaudRate = 0;
				break;
		}

		UARTSettings* settings = rs232GetSettings();
		SemaphoreHandle_t* settingsSemaphore = rs232GetSettingsSemaphore();
		/* Try to take the settings semaphore */
		if (newBaudRate != 0 && *settingsSemaphore != 0 && xSemaphoreTake(*settingsSemaphore, 100) == pdTRUE)
		{
			settings->baudRate = newBaudRate;
			rs232UpdateWithNewSettings();
			/* Give back the semaphore now that we are done */
			xSemaphoreGive(*settingsSemaphore);

			/* Restart the channel as we made some changes */
			rs232Restart();
		}

		/* Hide the pop out */
		GUIContainer_Hide(GUIContainerId_PopoutRs232BaudRate);
		GUI_SetActiveLayer(GUILayer_0);
		GUIButton_SetLayer(GUIButtonId_Rs232BaudRate, GUILayer_0);
		GUIButton_SetState(GUIButtonId_Rs232BaudRate, GUIButtonState_Disabled);

		/* Refresh the main text box */
		guiRs232ManageMainTextBox(true);
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiRs232ParitySelectionCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		UARTParity newParity;
		switch (ButtonId)
		{
			case GUIButtonId_Rs232ParityNone:
				GUIButton_SetTextForRow(GUIButtonId_Rs232Parity, "None", 1);
				newParity = UARTParity_None;
				break;
			case GUIButtonId_Rs232ParityOdd:
				GUIButton_SetTextForRow(GUIButtonId_Rs232Parity, "Odd", 1);
				newParity = UARTParity_Odd;
				break;
			case GUIButtonId_Rs232ParityEven:
				GUIButton_SetTextForRow(GUIButtonId_Rs232Parity, "Even", 1);
				newParity = UARTParity_Even;
				break;
			default:
				newParity = 0xFFFF;
				break;
		}

		UARTSettings* settings = rs232GetSettings();
		SemaphoreHandle_t* settingsSemaphore = rs232GetSettingsSemaphore();
		/* Try to take the settings semaphore */
		if (newParity != 0xFFFF && *settingsSemaphore != 0 && xSemaphoreTake(*settingsSemaphore, 100) == pdTRUE)
		{
			settings->parity = newParity;
			rs232UpdateWithNewSettings();

			/* Restart the channel if it was on */
			if (settings->connection == UARTConnection_Connected)
				rs232Restart();

			/* Give back the semaphore now that we are done */
			xSemaphoreGive(*settingsSemaphore);
		}

		/* Hide the pop out */
		GUIContainer_Hide(GUIContainerId_PopoutRs232Parity);
		GUI_SetActiveLayer(GUILayer_0);
		GUIButton_SetLayer(GUIButtonId_Rs232Parity, GUILayer_0);
		GUIButton_SetState(GUIButtonId_Rs232Parity, GUIButtonState_Disabled);

		/* Refresh the main text box */
		guiRs232ManageMainTextBox(true);
	}
}

/**
 * @brief
 * @param	Event: The event that caused the callback
 * @param	ButtonId: The button ID that the event happened on
 * @retval	None
 */
void guiRs232SidebarForwardBackwardsButtonsCallback(GUITouchEvent Event, uint32_t ButtonId)
{
	if (Event == GUITouchEvent_Up)
	{
		if (ButtonId == GUIButtonId_Rs232SidebarBackwards)
		{
			/* Decrease the page by one step */
			GUIContainer_DecrementPage(GUIContainerId_SidebarRs232);
		}
		else if (ButtonId == GUIButtonId_Rs232SidebarForwards)
		{
			/* Increase the page by one step */
			GUIContainer_IncrementPage(GUIContainerId_SidebarRs232);
		}

		/* Update the state of the forward and backwards buttons to indicate if the ends have been reached */
		GUIContainerPage activePage = GUIContainer_GetActivePage(GUIContainerId_SidebarRs232);
		GUIContainerPage lastPage = GUIContainer_GetLastPage(GUIContainerId_SidebarRs232);
		if (activePage == GUIContainerPage_1)
			GUIButton_SetState(GUIButtonId_Rs232SidebarBackwards, GUIButtonState_DisabledTouch);
		else
			GUIButton_SetState(GUIButtonId_Rs232SidebarBackwards, GUIButtonState_Enabled);

		if (activePage == lastPage)
			GUIButton_SetState(GUIButtonId_Rs232SidebarForwards, GUIButtonState_DisabledTouch);
		else
			GUIButton_SetState(GUIButtonId_Rs232SidebarForwards, GUIButtonState_Enabled);
	}
}

/**
 * @brief	Update the GUI elements for this channel that are dependent on the value of the settings
 * @param	None
 * @retval	None
 */
void guiRs232UpdateGuiElementsReadFromSettings()
{
	/* Get the current settings */
	UARTSettings* settings = rs232GetSettings();
	/* Update the baud rate text to match what is actually set */
	switch (settings->baudRate)
	{
		case UARTBaudRate_4800:
			GUIButton_SetTextForRow(GUIButtonId_Rs232BaudRate, "4800 bps", 1);
			break;
		case UARTBaudRate_7200:
			GUIButton_SetTextForRow(GUIButtonId_Rs232BaudRate, "7200 bps", 1);
			break;
		case UARTBaudRate_9600:
			GUIButton_SetTextForRow(GUIButtonId_Rs232BaudRate, "9600 bps", 1);
			break;
		case UARTBaudRate_19200:
			GUIButton_SetTextForRow(GUIButtonId_Rs232BaudRate, "19200 bps", 1);
			break;
		case UARTBaudRate_28800:
			GUIButton_SetTextForRow(GUIButtonId_Rs232BaudRate, "28800 bps", 1);
			break;
		case UARTBaudRate_38400:
			GUIButton_SetTextForRow(GUIButtonId_Rs232BaudRate, "38400 bps", 1);
			break;
		case UARTBaudRate_57600:
			GUIButton_SetTextForRow(GUIButtonId_Rs232BaudRate, "57600 bps", 1);
			break;
		case UARTBaudRate_115200:
			GUIButton_SetTextForRow(GUIButtonId_Rs232BaudRate, "115200 bps", 1);
			break;
		default:
			break;
	}
	/* Update the write format text to match what is actually set */
	switch (settings->textFormat)
	{
		case GUITextFormat_ASCII:
			GUIButton_SetTextForRow(GUIButtonId_Rs232Format, "ASCII", 1);
			break;
		case GUITextFormat_HexWithSpaces:
			GUIButton_SetTextForRow(GUIButtonId_Rs232Format, "Hex", 1);
			break;
		default:
			break;
	}
}

/**
 * @brief
 * @param	None
 * @retval	None
 */
void guiRs232InitGuiElements()
{
	/* Text boxes ----------------------------------------------------------------*/
	/* RS232 Label text box */
	prvTextBox.object.id = GUITextBoxId_Rs232Label;
	prvTextBox.object.xPos = 650;
	prvTextBox.object.yPos = 50;
	prvTextBox.object.width = 150;
	prvTextBox.object.height = 50;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.object.containerPage = GUIContainerPage_All;
	prvTextBox.textColor = GUI_PURPLE;
	prvTextBox.backgroundColor = GUI_WHITE;
	prvTextBox.staticText = "RS232";
	prvTextBox.textSize = LCDFontEnlarge_2x;
	GUITextBox_Add(&prvTextBox);

	/* RS232 Main text box */
	prvTextBox.object.id = GUITextBoxId_Rs232Main;
	prvTextBox.object.xPos = 0;
	prvTextBox.object.yPos = 50;
	prvTextBox.object.width = 650;
	prvTextBox.object.height = 400;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Right;
	prvTextBox.object.borderThickness = 1;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.object.containerPage = GUI_PURPLE;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = LCD_COLOR_BLACK;
	prvTextBox.textSize = LCDFontEnlarge_1x;
	prvTextBox.padding.bottom = guiConfigFONT_HEIGHT_UNIT;
	prvTextBox.padding.top = guiConfigFONT_HEIGHT_UNIT;
	prvTextBox.padding.left = guiConfigFONT_WIDTH_UNIT;
	prvTextBox.padding.right = guiConfigFONT_WIDTH_UNIT;
	prvTextBox.dataReadFunction = SPI_FLASH_ReadBufferDMA;
	prvTextBox.readStartAddress = FLASH_ADR_RS232_DATA;
	prvTextBox.readEndAddress = FLASH_ADR_RS232_DATA;
	prvTextBox.readMinAddress = FLASH_ADR_RS232_DATA;
	prvTextBox.readLastValidByteAddress = FLASH_ADR_RS232_DATA;
	prvTextBox.readMaxAddress = FLASH_ADR_RS232_DATA + FLASH_CHANNEL_DATA_SIZE - 1;
	GUITextBox_Add(&prvTextBox);

	/* RS232 Info Text Box */
	prvTextBox.object.id = GUITextBoxId_Rs232Info;
	prvTextBox.object.xPos = 0;
	prvTextBox.object.yPos = 450;
	prvTextBox.object.width = 650;
	prvTextBox.object.height = 30;
	prvTextBox.object.border = GUIBorder_Top | GUIBorder_Right;
	prvTextBox.object.borderThickness = 2;
	prvTextBox.object.borderColor = GUI_WHITE;
	prvTextBox.object.containerPage = GUIContainerPage_1;
	prvTextBox.textColor = GUI_WHITE;
	prvTextBox.backgroundColor = GUI_PURPLE;
	prvTextBox.textSize = LCDFontEnlarge_1x;
	prvTextBox.xWritePos = 0;
	prvTextBox.yWritePos = 0;
	GUITextBox_Add(&prvTextBox);

	/* Buttons -------------------------------------------------------------------*/
	/* RS232 Button */
	prvButton.object.id = GUIButtonId_Rs232Top;
	prvButton.object.xPos = 400;
	prvButton.object.yPos = 0;
	prvButton.object.width = 100;
	prvButton.object.height = 50;
	prvButton.object.displayState = GUIDisplayState_NotHidden;
	prvButton.object.border = GUIBorder_Bottom | GUIBorder_Right | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_PURPLE;
	prvButton.disabledBackgroundColor = LCD_COLOR_BLACK;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiRs232TopButtonCallback;
	prvButton.text[0] = "RS232";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUIButton_Add(&prvButton);

	/* RS232 Enable Button */
	prvButton.object.id = GUIButtonId_Rs232Enable;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 100;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.object.containerPage = GUIContainerPage_1;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiRs232EnableButtonCallback;
	prvButton.text[0] = "Output:";
//	prvButton.text[1] = "Enabled";
	prvButton.text[1] = "Disabled";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);

	/* RS232 Baud Rate Button */
	prvButton.object.id = GUIButtonId_Rs232BaudRate;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 150;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.object.containerPage = GUIContainerPage_1;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_DARK_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiRs232BaudRateButtonCallback;
	prvButton.text[0] = "< Baud Rate:";
	prvButton.text[1] = "115200 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);

	/* RS232 Parity Button */
	prvButton.object.id = GUIButtonId_Rs232Parity;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 200;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.object.containerPage = GUIContainerPage_1;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_DARK_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiRs232ParityButtonCallback;
	prvButton.text[0] = "< Parity:";
	prvButton.text[1] = "None";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);

	/* RS232 Format Button */
	prvButton.object.id = GUIButtonId_Rs232Format;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 250;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.object.containerPage = GUIContainerPage_1;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiRs232FormatButtonCallback;
	prvButton.text[0] = "Display Format:";
	prvButton.text[1] = "ASCII";
//	prvButton.text[1] = "HEX";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);

	/* RS232 Clear Button */
	prvButton.object.id = GUIButtonId_Rs232Clear;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 300;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.object.containerPage = GUIContainerPage_1;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = lcdGenericUartClearButtonCallback;
	prvButton.text[0] = "Clear";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);

	/* RS232 Debug Button */
	prvButton.object.id = GUIButtonId_Rs232Debug;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 100;
	prvButton.object.width = 150;
	prvButton.object.height = 50;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.object.containerPage = GUIContainerPage_2;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_RED;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_RED;
	prvButton.pressedTextColor = GUI_RED;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiRs232DebugButtonCallback;
	prvButton.text[0] = "Debug TX:";
	prvButton.text[1] = "Disabled";
//	prvButton.text[1] = "Enabled";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	prvButton.textSize[1] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);

	/* RS232 Sidebar backwards button */
	prvButton.object.id = GUIButtonId_Rs232SidebarBackwards;
	prvButton.object.xPos = 650;
	prvButton.object.yPos = 400;
	prvButton.object.width = 75;
	prvButton.object.height = 50;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left | GUIBorder_Right;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.object.containerPage = GUIContainerPage_All;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_DARK_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_DisabledTouch;
	prvButton.touchCallback = guiRs232SidebarForwardBackwardsButtonsCallback;
	prvButton.text[0] = "<";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUIButton_Add(&prvButton);

	/* RS232 Sidebar forwards button */
	prvButton.object.id = GUIButtonId_Rs232SidebarForwards;
	prvButton.object.xPos = 725;
	prvButton.object.yPos = 400;
	prvButton.object.width = 75;
	prvButton.object.height = 50;
	prvButton.object.border = GUIBorder_Top | GUIBorder_Bottom | GUIBorder_Left;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.object.containerPage = GUIContainerPage_All;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_DARK_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Enabled;
	prvButton.touchCallback = guiRs232SidebarForwardBackwardsButtonsCallback;
	prvButton.text[0] = ">";
	prvButton.textSize[0] = LCDFontEnlarge_2x;
	GUIButton_Add(&prvButton);

	/* RS232 4800 bps Button */
	prvButton.object.id = GUIButtonId_Rs232BaudRate4800;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 150;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiRs232BaudRateSelectionCallback;
	prvButton.text[0] = "4800 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);

	/* RS232 7200 bps Button */
	prvButton.object.id = GUIButtonId_Rs232BaudRate7200;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 190;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiRs232BaudRateSelectionCallback;
	prvButton.text[0] = "7200 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);

	/* RS232 9600 bps Button */
	prvButton.object.id = GUIButtonId_Rs232BaudRate9600;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 230;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiRs232BaudRateSelectionCallback;
	prvButton.text[0] = "9600 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);

	/* RS232 19200 bps Button */
	prvButton.object.id = GUIButtonId_Rs232BaudRate19k2;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 270;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiRs232BaudRateSelectionCallback;
	prvButton.text[0] = "19200 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);

	/* RS232 28800 bps Button */
	prvButton.object.id = GUIButtonId_Rs232BaudRate28k8;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 310;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiRs232BaudRateSelectionCallback;
	prvButton.text[0] = "28800 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);

	/* RS232 38400 bps Button */
	prvButton.object.id = GUIButtonId_Rs232BaudRate38k4;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 350;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiRs232BaudRateSelectionCallback;
	prvButton.text[0] = "38400 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);

	/* RS232 57600 bps Button */
	prvButton.object.id = GUIButtonId_Rs232BaudRate57k6;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 390;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiRs232BaudRateSelectionCallback;
	prvButton.text[0] = "57600 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);

	/* RS232 115200 bps Button */
	prvButton.object.id = GUIButtonId_Rs232BaudRate115k;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 430;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiRs232BaudRateSelectionCallback;
	prvButton.text[0] = "115200 bps";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);

	/* RS232 Parity None Button */
	prvButton.object.id = GUIButtonId_Rs232ParityNone;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 200;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiRs232ParitySelectionCallback;
	prvButton.text[0] = "None";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);

	/* RS232 Parity Odd Button */
	prvButton.object.id = GUIButtonId_Rs232ParityOdd;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 240;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiRs232ParitySelectionCallback;
	prvButton.text[0] = "Odd";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);

	/* RS232 Parity None Button */
	prvButton.object.id = GUIButtonId_Rs232ParityEven;
	prvButton.object.xPos = 500;
	prvButton.object.yPos = 280;
	prvButton.object.width = 149;
	prvButton.object.height = 40;
	prvButton.object.layer = GUILayer_1;
	prvButton.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvButton.object.borderThickness = 1;
	prvButton.object.borderColor = GUI_WHITE;
	prvButton.enabledTextColor = GUI_WHITE;
	prvButton.enabledBackgroundColor = GUI_PURPLE;
	prvButton.disabledTextColor = GUI_WHITE;
	prvButton.disabledBackgroundColor = GUI_PURPLE;
	prvButton.pressedTextColor = GUI_PURPLE;
	prvButton.pressedBackgroundColor = GUI_WHITE;
	prvButton.state = GUIButtonState_Disabled;
	prvButton.touchCallback = guiRs232ParitySelectionCallback;
	prvButton.text[0] = "Even";
	prvButton.textSize[0] = LCDFontEnlarge_1x;
	GUIButton_Add(&prvButton);

	/* Containers ----------------------------------------------------------------*/
	/* Sidebar RS232 container */
	prvContainer.object.id = GUIContainerId_SidebarRs232;
	prvContainer.object.xPos = 650;
	prvContainer.object.yPos = 50;
	prvContainer.object.width = 150;
	prvContainer.object.height = 400;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.activePage = GUIContainerPage_1;
	prvContainer.lastPage = GUIContainerPage_2;
	prvContainer.contentHideState = GUIHideState_KeepBorders;
	prvContainer.buttons[0] = GUIButton_GetFromId(GUIButtonId_Rs232Enable);
	prvContainer.buttons[1] = GUIButton_GetFromId(GUIButtonId_Rs232BaudRate);
	prvContainer.buttons[2] = GUIButton_GetFromId(GUIButtonId_Rs232Parity);
	prvContainer.buttons[3] = GUIButton_GetFromId(GUIButtonId_Rs232Format);
	prvContainer.buttons[4] = GUIButton_GetFromId(GUIButtonId_Rs232Clear);
	prvContainer.buttons[5] = GUIButton_GetFromId(GUIButtonId_Rs232Debug);
	prvContainer.buttons[6] = GUIButton_GetFromId(GUIButtonId_Rs232SidebarBackwards);
	prvContainer.buttons[7] = GUIButton_GetFromId(GUIButtonId_Rs232SidebarForwards);
	prvContainer.textBoxes[0] = GUITextBox_GetFromId(GUITextBoxId_Rs232Label);
	GUIContainer_Add(&prvContainer);

	/* RS232 baud rate popout container */
	prvContainer.object.id = GUIContainerId_PopoutRs232BaudRate;
	prvContainer.object.xPos = 500;
	prvContainer.object.yPos = 150;
	prvContainer.object.width = 149;
	prvContainer.object.height = 320;
	prvContainer.object.layer = GUILayer_1;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 2;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.contentHideState = GUIHideState_HideAll;
	prvContainer.buttons[0] = GUIButton_GetFromId(GUIButtonId_Rs232BaudRate4800);
	prvContainer.buttons[1] = GUIButton_GetFromId(GUIButtonId_Rs232BaudRate7200);
	prvContainer.buttons[2] = GUIButton_GetFromId(GUIButtonId_Rs232BaudRate9600);
	prvContainer.buttons[3] = GUIButton_GetFromId(GUIButtonId_Rs232BaudRate19k2);
	prvContainer.buttons[4] = GUIButton_GetFromId(GUIButtonId_Rs232BaudRate28k8);
	prvContainer.buttons[5] = GUIButton_GetFromId(GUIButtonId_Rs232BaudRate38k4);
	prvContainer.buttons[6] = GUIButton_GetFromId(GUIButtonId_Rs232BaudRate57k6);
	prvContainer.buttons[7] = GUIButton_GetFromId(GUIButtonId_Rs232BaudRate115k);
	GUIContainer_Add(&prvContainer);

	/* RS232 parity popout container */
	prvContainer.object.id = GUIContainerId_PopoutRs232Parity;
	prvContainer.object.xPos = 500;
	prvContainer.object.yPos = 200;
	prvContainer.object.width = 149;
	prvContainer.object.height = 120;
	prvContainer.object.layer = GUILayer_1;
	prvContainer.object.border = GUIBorder_Left | GUIBorder_Top | GUIBorder_Bottom;
	prvContainer.object.borderThickness = 2;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.contentHideState = GUIHideState_HideAll;
	prvContainer.buttons[0] = GUIButton_GetFromId(GUIButtonId_Rs232ParityNone);
	prvContainer.buttons[1] = GUIButton_GetFromId(GUIButtonId_Rs232ParityOdd);
	prvContainer.buttons[2] = GUIButton_GetFromId(GUIButtonId_Rs232ParityEven);
	GUIContainer_Add(&prvContainer);

	/* RS232 main container */
	prvContainer.object.id = GUIContainerId_Rs232MainContent;
	prvContainer.object.xPos = 0;
	prvContainer.object.yPos = 50;
	prvContainer.object.width = 650;
	prvContainer.object.height = 400;
	prvContainer.object.containerPage = guiConfigMAIN_CONTAINER_RS232_PAGE;
	prvContainer.object.border = GUIBorder_Right | GUIBorder_Top;
	prvContainer.object.borderThickness = 1;
	prvContainer.object.borderColor = GUI_WHITE;
	prvContainer.activePage = GUIContainerPage_1;
	prvContainer.backgroundColor = GUI_BLACK;
	prvContainer.contentHideState = GUIHideState_HideAll;
	prvContainer.textBoxes[0] = GUITextBox_GetFromId(GUITextBoxId_Rs232Main);
	prvContainer.textBoxes[1] = GUITextBox_GetFromId(GUITextBoxId_Rs232Info);
	GUIContainer_Add(&prvContainer);
}

/* Interrupt Handlers --------------------------------------------------------*/
