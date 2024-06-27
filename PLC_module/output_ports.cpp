#include "output_ports.hpp"


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
