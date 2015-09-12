/**
 *******************************************************************************
 * @file    spi_comm.c
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

/** Includes -----------------------------------------------------------------*/
#include "spi_comm.h"

/** Private defines ----------------------------------------------------------*/
#define COMM_SPI               (SPI5)
#define COMM_SPI_CLK_ENABLE()  __SPI5_CLK_ENABLE()
#define COMM_PORT              (GPIOF)
#define COMM_GPIO_CLK_ENABLE() __GPIOF_CLK_ENABLE()
#define COMM_CS_PIN            (GPIO_PIN_6)
#define COMM_SCK_PIN           (GPIO_PIN_7)
#define COMM_MISO_PIN          (GPIO_PIN_8)
#define COMM_MOSI_PIN          (GPIO_PIN_9)

#define COMM_SPI_TIMEOUT    (10)

/** SPI COMM Commands */

/** Private typedefs ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
static SPI_HandleTypeDef SPI_Handle = {
  .Instance                 = COMM_SPI,
  .Init.Mode                = SPI_MODE_MASTER,
  .Init.Direction           = SPI_DIRECTION_2LINES,
  .Init.DataSize            = SPI_DATASIZE_8BIT,
  .Init.CLKPolarity         = SPI_POLARITY_LOW,
  .Init.CLKPhase            = SPI_PHASE_1EDGE,
  .Init.NSS                 = SPI_NSS_SOFT,
  .Init.BaudRatePrescaler   = SPI_BAUDRATEPRESCALER_2,
  .Init.FirstBit            = SPI_FIRSTBIT_MSB,
  .Init.TIMode              = SPI_TIMODE_DISABLED,
  .Init.CRCCalculation      = SPI_CRCCALCULATION_DISABLED,
  .Init.CRCPolynomial       = 1,
};

static uint32_t prvDeviceId = 0;
static SemaphoreHandle_t xSemaphore;
static bool prvInitialized = false;

/** Private function prototypes ----------------------------------------------*/
static inline void prvSPI_COMM_CS_LOW();
static inline void prvSPI_COMM_CS_HIGH();

/** Functions ----------------------------------------------------------------*/
/**
 * @brief   Initializes the SPI COMM
 * @param   None
 * @retval  None
 */
ErrorStatus SPI_COMM_Init()
{
  /* Make sure we only initialize it once */
  if (!prvInitialized)
  {
    /* Mutex semaphore for mutual exclusion to the SPI Comm device */
    xSemaphore = xSemaphoreCreateMutex();

    /* Init GPIO */
    COMM_GPIO_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pin        = COMM_SCK_PIN | COMM_MISO_PIN | COMM_MOSI_PIN;
    GPIO_InitStructure.Mode       = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Alternate  = GPIO_AF5_SPI5;
    GPIO_InitStructure.Pull       = GPIO_NOPULL;
    GPIO_InitStructure.Speed      = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(COMM_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin        = COMM_CS_PIN;
    GPIO_InitStructure.Mode       = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(COMM_PORT, &GPIO_InitStructure);
    /* Deselect the COMM */
    prvSPI_COMM_CS_HIGH();

    /* Init SPI */
    COMM_SPI_CLK_ENABLE();
    HAL_SPI_Init(&SPI_Handle);

    /* Read COMM identification */
    prvDeviceId = SPI_COMM_ReadID();
  }
  return ERROR;
}

/**
 * @brief   Return the status of if the SPI COMM is intialized or not
 * @param   None
 * @retval  true: The device is initialized
 * @retval  false: The device is not initialized
 */
bool SPI_COMM_Initialized()
{
  return prvInitialized;
}

/**
 * @brief   Reads COMM identification
 * @param   None
 * @retval  COMM identification
 */
uint32_t SPI_COMM_ReadID()
{
  uint8_t byte[3];

  /* Try to take the semaphore in case some other process is using the device */
  if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
  {
    /* Select the COMM */
    prvSPI_COMM_CS_LOW();

    /* TODO: Read ID */

    /* Deselect the COMM */
    prvSPI_COMM_CS_HIGH();

    /* Give back the semaphore */
    xSemaphoreGive(xSemaphore);
  }
  return (byte[0] << 16) | (byte[1] << 8) | byte[2];
}

/**
 * @brief
 * @param   Command:
 * @param   pData:
 * @param   DataCount:
 * @retval  None
 */
void SPI_COMM_SendCommand(uint8_t Command, uint8_t* pData, uint32_t DataCount)
{
  /* Try to take the semaphore in case some other process is using the device */
  if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
  {
    /* Select the COMM */
    prvSPI_COMM_CS_LOW();

    /* Send command */
    HAL_SPI_Transmit(&SPI_Handle, &Command, 1, COMM_SPI_TIMEOUT);
    if (DataCount)
      HAL_SPI_Transmit(&SPI_Handle, pData, DataCount, COMM_SPI_TIMEOUT);

    /* Deselect the COMM */
    prvSPI_COMM_CS_HIGH();

    /* Give back the semaphore */
    xSemaphoreGive(xSemaphore);
  }
}

/**
 * @brief
 * @param   pDataBuffer:
 * @param   DataCount:
 * @retval  None
 */
void SPI_COMM_GetData(uint8_t* pDataBuffer, uint32_t DataCount)
{
  /* Try to take the semaphore in case some other process is using the device */
  if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
  {
    /* Select the COMM */
    prvSPI_COMM_CS_LOW();

    /* Get Data */
    if (DataCount)
      HAL_SPI_Receive(&SPI_Handle, pDataBuffer, DataCount, COMM_SPI_TIMEOUT);

    /* Deselect the COMM */
    prvSPI_COMM_CS_HIGH();

    /* Give back the semaphore */
    xSemaphoreGive(xSemaphore);
  }
}

/**
 * @brief   Enable the power to a channel
 * @param   Channel: The channel to use
 * @retval  None
 */
void SPI_COMM_EnablePowerForChannel(SPI_COMM_Channel Channel)
{
  if (Channel != 0 && Channel <= 6)
  {
    uint8_t data = 0x40 | (1 << (Channel-1));
    SPI_COMM_SendCommand(SPI_COMM_COMMAND_CHANNEL_POWER, &data, 1);
  }
  else if (Channel == SPI_COMM_Channel_All)
  {
    uint8_t data = 0x40 | 0x1F;
    SPI_COMM_SendCommand(SPI_COMM_COMMAND_CHANNEL_POWER, &data, 1);
  }
}

/**
 * @brief   Disable the power to a channel
 * @param   Channel: The channel to use
 * @retval  None
 */
void SPI_COMM_DisablePowerForChannel(SPI_COMM_Channel Channel)
{
  if (Channel != 0 && Channel <= 6)
  {
    uint8_t data = 0x80 | (1 << (Channel-1));
    SPI_COMM_SendCommand(SPI_COMM_COMMAND_CHANNEL_POWER, &data, 1);
  }
  else if (Channel == SPI_COMM_Channel_All)
  {
    uint8_t data = 0x80 | 0x1F;
    SPI_COMM_SendCommand(SPI_COMM_COMMAND_CHANNEL_POWER, &data, 1);
  }
}

/**
 * @brief   Enable the output for a channel
 * @param   Channel: The channel to use
 * @retval  None
 */
void SPI_COMM_EnableOutputForChannel(SPI_COMM_Channel Channel)
{
  if (Channel != 0 && Channel <= 6)
  {
    uint8_t data = 0x40 | (1 << (Channel-1));
    SPI_COMM_SendCommand(SPI_COMM_COMMAND_CHANNEL_OUTPUT, &data, 1);
  }
  else if (Channel == SPI_COMM_Channel_All)
  {
    uint8_t data = 0x40 | 0x1F;
    SPI_COMM_SendCommand(SPI_COMM_COMMAND_CHANNEL_OUTPUT, &data, 1);
  }
}

/**
 * @brief   Disable the output for a channel
 * @param   Channel: The channel to use
 * @retval  None
 */
void SPI_COMM_DisableOutputForChannel(SPI_COMM_Channel Channel)
{
  if (Channel != 0 && Channel <= 6)
  {
    uint8_t data = 0x80 | (1 << (Channel-1));
    SPI_COMM_SendCommand(SPI_COMM_COMMAND_CHANNEL_OUTPUT, &data, 1);
  }
  else if (Channel == SPI_COMM_Channel_All)
  {
    uint8_t data = 0x80 | 0x1F;
    SPI_COMM_SendCommand(SPI_COMM_COMMAND_CHANNEL_OUTPUT, &data, 1);
  }
}

/**
 * @brief   Enable the CAN termination for a channel
 * @param   Channel: The channel to use
 * @retval  None
 */
void SPI_COMM_EnableTerminationForChannel(SPI_COMM_Channel Channel)
{
  if (Channel != 0 && Channel <= 6)
  {
    uint8_t data = 0x40 | (1 << (Channel-1));
    SPI_COMM_SendCommand(SPI_COMM_COMMAND_CAN_CHANNEL_TERMINATION, &data, 1);
  }
  else if (Channel == SPI_COMM_Channel_All)
  {
    uint8_t data = 0x40 | 0x1F;
    SPI_COMM_SendCommand(SPI_COMM_COMMAND_CAN_CHANNEL_TERMINATION, &data, 1);
  }
}

/**
 * @brief   Disable the CAN termination for a channel
 * @param   Channel: The channel to use
 * @retval  None
 */
void SPI_COMM_DisableTerminationForChannel(SPI_COMM_Channel Channel)
{
  if (Channel != 0 && Channel <= 6)
  {
    uint8_t data = 0x80 | (1 << (Channel-1));
    SPI_COMM_SendCommand(SPI_COMM_COMMAND_CAN_CHANNEL_TERMINATION, &data, 1);
  }
  else if (Channel == SPI_COMM_Channel_All)
  {
    uint8_t data = 0x80 | 0x1F;
    SPI_COMM_SendCommand(SPI_COMM_COMMAND_CAN_CHANNEL_TERMINATION, &data, 1);
  }
}


/** Private functions .-------------------------------------------------------*/
/**
 * @brief   Pull the CS pin LOW
 * @param   None
 * @retval  None
 */
static inline void prvSPI_COMM_CS_LOW()
{
  HAL_GPIO_WritePin(COMM_PORT, COMM_CS_PIN, GPIO_PIN_RESET);
}

/**
 * @brief  Pull the CS pin HIGH
 * @param  None
 * @retval  None
 */
static inline void prvSPI_COMM_CS_HIGH()
{
  HAL_GPIO_WritePin(COMM_PORT, COMM_CS_PIN, GPIO_PIN_SET);
}

/** Interrupt Handlers -------------------------------------------------------*/
/**
  * @brief  TxRx Transfer completed callback.
  * @param  hspi: SPI handle
  * @retval None
  */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{

}

/**
  * @brief  SPI error callbacks
  * @param  hspi: SPI handle
  * @retval None
  */
 void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
   /* TODO: Manage errors */
}
