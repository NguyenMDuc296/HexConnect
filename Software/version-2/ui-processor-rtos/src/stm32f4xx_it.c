/**
 *******************************************************************************
 * @file    stm32f4xx_it.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-15
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
#include "stm32f4xx_it.h"

#include "ft5206.h"
#include "lcd.h"
#include "uart1.h"
#include "uart_comm.h"

/** Private defines ----------------------------------------------------------*/
/** Private typedefs ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
/** Private function prototypes ----------------------------------------------*/
/** Functions ----------------------------------------------------------------*/

/** Private functions --------------------------------------------------------*/

/** Cortex-M4 Processor Exceptions Handlers  ---------------------------------*/

/** STM32F4xx Peripherals Interrupt Handlers   -------------------------------*/
/**
  * @brief  This function handles External line 1 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI1_IRQHandler(void)
{
  /* Check CTP_INT Interrupt */
  if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_1) != RESET)
  {
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
    CTP_INT_Callback();
  }
}

/**
  * @brief  This function handles DMA2D IRQ
  * @param  None
  * @retval None
  */
void DMA2D_IRQHandler(void)
{
  HAL_DMA2D_IRQHandler(&DMA2DHandle);
}

/**
  * @brief  This function handles LTDC IRQ
  * @param  None
  * @retval None
  */
void LTDC_IRQHandler(void)
{
  HAL_LTDC_IRQHandler(&LTDCHandle);
}

/**
  * @brief  This function handles UART interrupt request.
  * @param  None
  * @retval None
  */
void USART1_IRQHandler(void)
{
  /* Check if it's a RX interrupt */
  uint32_t tmp_flag = 0, tmp_it_source = 0;
  tmp_flag = __HAL_UART_GET_FLAG(&UART_Handle, UART_FLAG_RXNE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(&UART_Handle, UART_IT_RXNE);
  if ((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
//    UART1_DataReceivedHandler();
    uint8_t temp = (uint8_t)(UART_Handle.Instance->DR & (uint8_t)0x00FF);
    UART_COMM_HandleReceivedByte(temp);
  }
  /* Otherwise call the HAL IRQ handler */
  else
    HAL_UART_IRQHandler(&UART_Handle);
}

/** HAL Callback functions ---------------------------------------------------*/
