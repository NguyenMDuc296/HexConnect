/**
 *******************************************************************************
 * @file  gpio1_task.c
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
#include "gpio1_task.h"

/* Private defines -----------------------------------------------------------*/
#define GPIO1_PORT				(GPIOB)
#define GPIO1_PIN				(GPIO_PIN_1)
#define GPIO1_DIRECTION_PORT	(GPIOC)
#define GPIO1_DIRECTION_PIN		(GPIO_PIN_5)


/* TODO: Use timer 8 instead */
//#define PWM_TIMER				(TIM3)
//#define PWM_TIMER_CLK_ENABLE	(__TIM3_CLK_ENABLE())
//#define PWM_AF_GPIO				(GPIO_AF2_TIM3)
//#define PWM_TIMER_CHANNEL		(TIM_CHANNEL_4)
//#define PWM_TIMER_CLOCK			(42000000)	/* 42 MHz, see datasheet page 31 */
//#define PWM_PERIOD				(255)		/* 256 step PWM */
//#define PWM_PRESCALER			(6)			/* Divide by 7 */
//#define PWM_FREQ				(PWM_TIMER_CLOCK/((PWM_PRESCALER+1) * (PWM_PERIOD+1)))

/* TODO: Don't use the negative output (TIM8_CH3N) next time */
#define PWM_TIMER				(TIM8)
#define PWM_TIMER_CLK_ENABLE	(__TIM8_CLK_ENABLE())
#define PWM_TIMER_GET_CLOCK()	(HAL_RCC_GetPCLK2Freq())
#define PWM_AF_GPIO				(GPIO_AF3_TIM8)
#define PWM_TIMER_CHANNEL		(TIM_CHANNEL_3)
#define PWM_CCR_REGISTER		CCR3
#define PWM_TIMER_CLOCK			(84000000)	/* 84 MHz, see datasheet page 31 */
#define PWM_MAX_FREQUENCY		18000//(PWM_TIMER_CLOCK/(PWM_PERIOD+1))

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Default settings that can be overwritten if valid settings are read from the SPI FLASH */
static GPIOSettings prvCurrentSettings = {
		.direction		= GPIODirection_Output,
		.pwmFrequency	= 10000,
		.pwmDuty		= 50.0,
		.pwmPeriod		= 255,
};

static TIM_HandleTypeDef prvTimerHandle = {
		.Instance 			= PWM_TIMER,
		.Init.Period		= 255,
		.Init.Prescaler		= 0,
		.Init.ClockDivision	= TIM_CLOCKDIVISION_DIV1,
		.Init.CounterMode	= TIM_COUNTERMODE_UP,
};

static bool prvIsEnabled = false;
static uint32_t prvTimerClock = 0;

/* Private function prototypes -----------------------------------------------*/
static void prvHardwareInit();
static void prvActivatePwmFunctionality();
static void prvDeactivatePwmFunctionality();
static uint32_t prvGetPrescalerForFrequency(uint32_t Frequency);
static uint32_t prvGetActualFrequency();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	The main task for the GPIO1 channel
 * @param	pvParameters:
 * @retval	None
 */
void gpio1Task(void *pvParameters)
{
	prvHardwareInit();

	gpio1SetDirection(GPIODirection_Output);

	/* The parameter in vTaskDelayUntil is the absolute time
	 * in ticks at which you want to be woken calculated as
	 * an increment from the time you were last woken. */
	TickType_t xNextWakeTime;
	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	while (1)
	{
		vTaskDelayUntil(&xNextWakeTime, 1000 / portTICK_PERIOD_MS);
	}
}

/**
 * @brief	Sets the direction of the GPIO
 * @param	Direction: The direction to use, can be any value of GPIODirection
 * @retval	None
 */
void gpio1SetDirection(GPIODirection Direction)
{
	prvCurrentSettings.direction = Direction;
}

/**
 * @brief	Get the curren direction of the channel
 * @param	None
 * @retval	The direction as a GPIODirection
 */
GPIODirection gpio1GetDirection()
{
	return prvCurrentSettings.direction;
}

/**
 * @brief	Read the value of the pin
 * @param	None
 * @retval	None
 */
GPIO_PinState gpio1ReadPin()
{
	return HAL_GPIO_ReadPin(GPIO1_PORT, GPIO1_PIN);
}

/**
 * @brief	Write a value to the pin
 * @param	None
 * @retval	None
 */
void gpio1WritePin(GPIO_PinState PinState)
{
	/* TODO: Make sure it's set as an output, otherwise return error */
	HAL_GPIO_WritePin(GPIO1_PORT, GPIO1_PIN, PinState);
}

/**
 * @brief	Toggle the value of the pin
 * @param	None
 * @retval	None
 */
void gpio1TogglePin()
{
	/* TODO: Make sure it's set as an output, otherwise return error */
	HAL_GPIO_TogglePin(GPIO1_PORT, GPIO1_PIN);
}

/**
 * @brief	Set the duty of the PWM
 * @param	Duty: The duty as a percentage (0.0 - 100.0%)
 * @retval	None
 */
void gpio1SetPwmDuty(float Duty)
{
	if (Duty >= 0.0 && Duty <= 100.0)
	{
		PWM_TIMER->PWM_CCR_REGISTER = (uint16_t)(Duty/100.0 * prvCurrentSettings.pwmPeriod);
		prvCurrentSettings.pwmDuty = Duty;
	}
}

/**
 * @brief	Get the duty of the PWM
 * @param	None
 * @retval	The duty as a percentage (0.0 - 100.0%)
 */
float gpio1GetPwmDuty()
{
	return prvCurrentSettings.pwmDuty;
}

/**
 * @brief	Set the frequency
 * @param	Frequency: The frequency to set
 * @retval	None
 */
void gpio1SetFrequency(uint32_t Frequency)
{
	if (Frequency <= PWM_MAX_FREQUENCY)
	{
		prvCurrentSettings.pwmFrequency = Frequency;
		prvDeactivatePwmFunctionality();
		prvActivatePwmFunctionality();
	}
}

/**
 * @brief	Get the frequency
 * @param	None
 * @retval	The current frequency
 */
uint32_t gpio1GetFrequency()
{
	return prvCurrentSettings.pwmFrequency;
}

/**
 * @brief	Get the maximum frequency
 * @param	None
 * @retval	The maximum frequency
 */
uint32_t gpio1GetMaxFrequency()
{
	return PWM_MAX_FREQUENCY;
}

/**
 * @brief	Enable the GPIO with the currently set settings
 * @param	None
 * @retval	None
 */
void gpio1Enable()
{
	/* If it's PWM we have to activate the timer and such */
	if (prvCurrentSettings.direction == GPIODirection_OutputPWM)
	{
		prvActivatePwmFunctionality();
	}
	else
	{
		/* Init the GPIO */
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.Pin  	= GPIO1_PIN;
		GPIO_InitStructure.Speed 	= GPIO_SPEED_HIGH;
		GPIO_InitStructure.Pull		= GPIO_NOPULL;
		if (prvCurrentSettings.direction == GPIODirection_Output)
			GPIO_InitStructure.Mode  	= GPIO_MODE_OUTPUT_PP;
		else
			GPIO_InitStructure.Mode  	= GPIO_MODE_INPUT;
		HAL_GPIO_Init(GPIO1_PORT, &GPIO_InitStructure);
	}

	/* Set the direction pin */
	if (prvCurrentSettings.direction == GPIODirection_Input)
		HAL_GPIO_WritePin(GPIO1_DIRECTION_PORT, GPIO1_DIRECTION_PIN, GPIO_PIN_SET);
	else if (prvCurrentSettings.direction == GPIODirection_Output || GPIODirection_OutputPWM)
		HAL_GPIO_WritePin(GPIO1_DIRECTION_PORT, GPIO1_DIRECTION_PIN, GPIO_PIN_RESET);

	prvIsEnabled = true;
}

/**
 * @brief	Disable the GPIO
 * @param	None
 * @retval	None
 */
void gpio1Disable()
{
	/* If it's PWM we have to deactivate the timer and such */
	if (prvCurrentSettings.direction == GPIODirection_OutputPWM)
		prvDeactivatePwmFunctionality();

	/* Deinit the GPIO */
	HAL_GPIO_DeInit(GPIO1_PORT, GPIO1_PIN);

	/* Set the direction pin as input */
	HAL_GPIO_WritePin(GPIO1_DIRECTION_PORT, GPIO1_DIRECTION_PIN, GPIO_PIN_SET);

	prvIsEnabled = false;
}

/**
 * @brief	Check if the GPIO is enabled or not
 * @param	None
 * @retval	true if it is
 * @retval	false if it is not
 */
bool gpio1IsEnabled()
{
	return prvIsEnabled;
}

/**
 * @brief	Get the current settings of the CAN1 channel
 * @param	None
 * @retval	A pointer to the current settings
 */
GPIOSettings* gpio1GetSettings()
{
	return &prvCurrentSettings;
}

/* Private functions .--------------------------------------------------------*/
/**
 * @brief	Initializes the hardware
 * @param	None
 * @retval	None
 */
static void prvHardwareInit()
{
	__GPIOB_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();

	/* Direction pin */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin  	= GPIO1_DIRECTION_PIN;
	GPIO_InitStructure.Mode  	= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull		= GPIO_NOPULL;
	GPIO_InitStructure.Speed 	= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIO1_DIRECTION_PORT, &GPIO_InitStructure);

	/* Set as input as default */
	gpio1SetDirection(GPIODirection_Input);
}

/**
 * @brief	Activate the PWM functionality for the channel
 * @param	None
 * @retval	None
 */
static void prvActivatePwmFunctionality()
{
	/* Get the frequency of the clock the timer is connected to */
	prvTimerClock = PWM_TIMER_GET_CLOCK();

	/* Enable TIMER Clock */
	PWM_TIMER_CLK_ENABLE;

	/* Configure the GPIO as alternate function */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin  		= GPIO1_PIN;
	GPIO_InitStructure.Mode  		= GPIO_MODE_AF_PP;
	GPIO_InitStructure.Alternate	= PWM_AF_GPIO;
	GPIO_InitStructure.Pull			= GPIO_NOPULL;
	GPIO_InitStructure.Speed 		= GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIO1_PORT, &GPIO_InitStructure);

	/* Timer init */
	prvTimerHandle.Instance 			= PWM_TIMER;
	prvTimerHandle.Init.Prescaler		= prvGetPrescalerForFrequency(prvCurrentSettings.pwmFrequency);
	prvTimerHandle.Init.Period			= prvCurrentSettings.pwmPeriod;
	prvTimerHandle.Init.ClockDivision	= TIM_CLOCKDIVISION_DIV1;
	prvTimerHandle.Init.CounterMode		= TIM_COUNTERMODE_UP;
	HAL_TIM_PWM_Init(&prvTimerHandle);

	/* Save the actual frequency */
	prvCurrentSettings.pwmFrequency = prvGetActualFrequency();

	/* Output compare init */
	TIM_OC_InitTypeDef timerOutputCompare;
	timerOutputCompare.OCMode 		= TIM_OCMODE_PWM1;
	timerOutputCompare.Pulse		= prvCurrentSettings.pwmPeriod / 2;
	timerOutputCompare.OCPolarity	= TIM_OCPOLARITY_HIGH;
	timerOutputCompare.OCNPolarity	= TIM_OCNPOLARITY_HIGH;
	timerOutputCompare.OCFastMode	= TIM_OCFAST_DISABLE;
	timerOutputCompare.OCIdleState	= TIM_OCIDLESTATE_SET;
	timerOutputCompare.OCNIdleState	= TIM_OCNIDLESTATE_SET;
	HAL_TIM_PWM_ConfigChannel(&prvTimerHandle, &timerOutputCompare, PWM_TIMER_CHANNEL);

	/* We don't need to configure this */
//	TIM_BreakDeadTimeConfigTypeDef breakDeadTimeConfig;
//	breakDeadTimeConfig.AutomaticOutput 	= TIM_AUTOMATICOUTPUT_ENABLE;
//	breakDeadTimeConfig.BreakPolarity 		= TIM_BREAKPOLARITY_LOW;
//	breakDeadTimeConfig.BreakState 			= TIM_BREAK_DISABLE;
//	breakDeadTimeConfig.DeadTime 			= 0;
//	breakDeadTimeConfig.LockLevel 			= TIM_LOCKLEVEL_OFF;
//	breakDeadTimeConfig.OffStateIDLEMode	= TIM_OSSI_ENABLE;
//	breakDeadTimeConfig.OffStateRunMode 	= TIM_OSSR_ENABLE;
//	HAL_TIMEx_ConfigBreakDeadTime(&timerHandle, &breakDeadTimeConfig);

	/* Start the PWM <- Not needed because we use the complementary, see below */
//	HAL_TIM_PWM_Start(&timerHandle, PWM_TIMER_CHANNEL);

	/* Start the complementary output as that is what we use */
	HAL_TIMEx_PWMN_Start(&prvTimerHandle, PWM_TIMER_CHANNEL);
}

/**
 * @brief	Deactivate the PWM functionality for the channel
 * @param	None
 * @retval	None
 */
static void prvDeactivatePwmFunctionality()
{
	HAL_GPIO_DeInit(GPIO1_PORT, GPIO1_PIN);
	HAL_TIMEx_PWMN_Stop(&prvTimerHandle, PWM_TIMER_CHANNEL);
	HAL_TIM_PWM_DeInit(&prvTimerHandle);
}

/**
 * @brief	Text
 * @param	None
 * @retval	None
 */
static uint32_t prvGetPrescalerForFrequency(uint32_t Frequency)
{
	uint32_t prescaler = (prvTimerClock / (Frequency/2 * (prvCurrentSettings.pwmPeriod + 1))) - 1;
	/* TODO: Change the period as well */
	return prescaler;
}

/**
 * @brief	Text
 * @param	None
 * @retval	None
 */
static uint32_t prvGetActualFrequency()
{
	uint32_t prescaler = prvTimerHandle.Init.Prescaler + 1;
	return (prvTimerClock / (prescaler * (prvCurrentSettings.pwmPeriod + 1))) * 2;
}

/* Interrupt Handlers --------------------------------------------------------*/
