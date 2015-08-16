/**
 *******************************************************************************
 * @file    i2c_eeprom.h
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
#ifndef I2C_EEPROM_H_
#define I2C_EEPROM_H_

/** Includes -----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include <stdbool.h>

/** Defines ------------------------------------------------------------------*/
/** Typedefs -----------------------------------------------------------------*/
/** Function prototypes ------------------------------------------------------*/
ErrorStatus I2C_EEPROM_Init();
bool I2C_EEPROM_Initialized();
void I2C_EEPROM_EnableWriteProtection();
void I2C_EEPROM_DisableWriteProtection();

void I2C_EEPROM_WriteByte(uint32_t WriteAddress, uint8_t Byte);
void I2C_EEPROM_WriteByteFromISR(uint32_t WriteAddress, uint8_t Byte);

uint8_t I2C_EEPROM_ReadByte(uint32_t ReadAddress);
uint8_t I2C_EEPROM_ReadByteFromISR(uint32_t ReadAddress);

#endif /* I2C_EEPROM_H_ */
