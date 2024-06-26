
#include <inttypes.h>
#include <math.h>
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"
#include "plc_module_registers.hpp"
#include "plc_adc.hpp"

void CheckInternalShortcircuits(void);

enum class OutputState;

void DQ_Write(uint8_t output_number, OutputState out_state);

GPIO_TypeDef *DQHigh_GetPort(uint8_t output_number);
GPIO_TypeDef *DQLow_GetPort(uint8_t output_number);

uint16_t DQHigh_GetPin(uint8_t output_number);
uint16_t DQLow_GetPin(uint8_t output_number);

struct ADCConversionResult
{
	uint16_t analog_input_7;
	uint16_t analog_input_6;
	uint16_t analog_input_5;
	uint16_t analog_input_4;
	uint16_t analog_input_3;
	uint16_t analog_input_2;
	uint16_t analog_input_1;
	uint16_t analog_input_0;
	uint16_t input_voltage;

	static constexpr uint32_t length = 9;
};

volatile ADCConversionResult adc_conversion_result;
volatile uint16_t ConversionTime = 0; // [us]

extern "C" void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{

	ConversionTime = htim3.Instance->CNT;
	HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
}

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	HAL_GPIO_TogglePin(LED_ORANGE_GPIO_Port, LED_ORANGE_Pin);
}

enum class OutputState
{
	FLOATING,
	HIGH,
	LOW,
};

GPIO_TypeDef *DQHigh_GetPort(uint8_t output_number)
{
	assert_param(output_number <= 7);
	switch (output_number)
	{
	case 0:
		return DQ0_HIGH_GPIO_Port;
	case 1:
		return DQ1_HIGH_GPIO_Port;
	case 2:
		return DQ2_HIGH_GPIO_Port;
	case 3:
		return DQ3_HIGH_GPIO_Port;
	case 4:
		return DQ4_HIGH_GPIO_Port;
	case 5:
		return DQ5_HIGH_GPIO_Port;
	case 6:
		return DQ6_HIGH_GPIO_Port;
	case 7:
		return DQ7_HIGH_GPIO_Port;
	}
	assert_param(0);
	return 0;
}

uint16_t DQHigh_GetPin(uint8_t output_number)
{
	assert_param(output_number <= 7);
	switch (output_number)
	{
	case 0:
		return DQ0_HIGH_Pin;
	case 1:
		return DQ1_HIGH_Pin;
	case 2:
		return DQ2_HIGH_Pin;
	case 3:
		return DQ3_HIGH_Pin;
	case 4:
		return DQ4_HIGH_Pin;
	case 5:
		return DQ5_HIGH_Pin;
	case 6:
		return DQ6_HIGH_Pin;
	case 7:
		return DQ7_HIGH_Pin;
	}
	assert_param(0);
	return 0;
}

GPIO_TypeDef *DQLow_GetPort(uint8_t output_number)
{
	assert_param(output_number <= 7);
	switch (output_number)
	{
	case 0:
		return DQ0_LOW_GPIO_Port;
	case 1:
		return DQ1_LOW_GPIO_Port;
	case 2:
		return DQ2_LOW_GPIO_Port;
	case 3:
		return DQ3_LOW_GPIO_Port;
	case 4:
		return DQ4_LOW_GPIO_Port;
	case 5:
		return DQ5_LOW_GPIO_Port;
	case 6:
		return DQ6_LOW_GPIO_Port;
	case 7:
		return DQ7_LOW_GPIO_Port;
	}
	assert_param(0);
	return 0;
}

uint16_t DQLow_GetPin(uint8_t output_number)
{
	assert_param(output_number <= 7);
	switch (output_number)
	{
	case 0:
		return DQ0_LOW_Pin;
	case 1:
		return DQ1_LOW_Pin;
	case 2:
		return DQ2_LOW_Pin;
	case 3:
		return DQ3_LOW_Pin;
	case 4:
		return DQ4_LOW_Pin;
	case 5:
		return DQ5_LOW_Pin;
	case 6:
		return DQ6_LOW_Pin;
	case 7:
		return DQ7_LOW_Pin;
	}
	assert_param(0);
	return 0;
}

void DQ_Write(uint8_t output_number, OutputState out_state)
{
	assert_param(output_number <= 7);

	//	disable all transistors
	HAL_GPIO_WritePin(DQLow_GetPort(output_number), DQLow_GetPin(output_number), GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DQHigh_GetPort(output_number), DQHigh_GetPin(output_number), GPIO_PIN_RESET);

	//	enable lower or uppper
	if (out_state == OutputState::HIGH)
	{
		HAL_GPIO_WritePin(DQHigh_GetPort(output_number), DQHigh_GetPin(output_number), GPIO_PIN_SET);
	}
	else if (out_state == OutputState::LOW)
	{
		HAL_GPIO_WritePin(DQLow_GetPort(output_number), DQLow_GetPin(output_number), GPIO_PIN_SET);
	}

	// sanity check
	CheckInternalShortcircuits();
}

void CheckInternalShortcircuits(void)
{
	assert_param((HAL_GPIO_ReadPin(DQ0_HIGH_GPIO_Port, DQ0_HIGH_Pin) != 1) || (HAL_GPIO_ReadPin(DQ0_LOW_GPIO_Port, DQ0_LOW_Pin) != 1));
	assert_param((HAL_GPIO_ReadPin(DQ1_HIGH_GPIO_Port, DQ1_HIGH_Pin) != 1) || (HAL_GPIO_ReadPin(DQ1_LOW_GPIO_Port, DQ1_LOW_Pin) != 1));
	assert_param((HAL_GPIO_ReadPin(DQ2_HIGH_GPIO_Port, DQ2_HIGH_Pin) != 1) || (HAL_GPIO_ReadPin(DQ2_LOW_GPIO_Port, DQ2_LOW_Pin) != 1));
	assert_param((HAL_GPIO_ReadPin(DQ3_HIGH_GPIO_Port, DQ3_HIGH_Pin) != 1) || (HAL_GPIO_ReadPin(DQ3_LOW_GPIO_Port, DQ3_LOW_Pin) != 1));
	assert_param((HAL_GPIO_ReadPin(DQ4_HIGH_GPIO_Port, DQ4_HIGH_Pin) != 1) || (HAL_GPIO_ReadPin(DQ4_LOW_GPIO_Port, DQ4_LOW_Pin) != 1));
	assert_param((HAL_GPIO_ReadPin(DQ5_HIGH_GPIO_Port, DQ5_HIGH_Pin) != 1) || (HAL_GPIO_ReadPin(DQ5_LOW_GPIO_Port, DQ5_LOW_Pin) != 1));
	assert_param((HAL_GPIO_ReadPin(DQ6_HIGH_GPIO_Port, DQ6_HIGH_Pin) != 1) || (HAL_GPIO_ReadPin(DQ6_LOW_GPIO_Port, DQ6_LOW_Pin) != 1));
	assert_param((HAL_GPIO_ReadPin(DQ7_HIGH_GPIO_Port, DQ7_HIGH_Pin) != 1) || (HAL_GPIO_ReadPin(DQ7_LOW_GPIO_Port, DQ7_LOW_Pin) != 1));
}

int main(void)
{
	HAL_Init();

	SystemClock_Config();

	MX_GPIO_Init();
	MX_DMA_Init();
	MX_ADC_Init();
	MX_TIM3_Init();

	HAL_ADCEx_Calibration_Start(&hadc);
	while (HAL_ADC_GetState(&hadc) != HAL_ADC_STATE_READY)
	{
	} // todo: handle error case

	HAL_ADC_Start_DMA(&hadc, (uint32_t *)&adc_conversion_result, ADCConversionResult::length);
	HAL_TIM_Base_Start_IT(&htim3);

	uint8_t output = 0;
	while (1)
	{

		DQ_Write(output, OutputState::LOW);
		CheckInternalShortcircuits();
		HAL_Delay(500);

		DQ_Write(output, OutputState::HIGH);
		CheckInternalShortcircuits();
		HAL_Delay(500);

		DQ_Write(output, OutputState::FLOATING);
		CheckInternalShortcircuits();
		HAL_Delay(500);

		output = output < 7 ? output + 1 : 0;
	}
}
