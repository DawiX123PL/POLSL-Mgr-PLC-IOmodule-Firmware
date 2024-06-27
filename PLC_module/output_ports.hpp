#pragma once

#include "main.h"

enum class OutputState
{
	FLOATING,
	HIGH,
	LOW,
};


void DQ_Write(uint8_t output_number, OutputState out_state);

GPIO_TypeDef *DQHigh_GetPort(uint8_t output_number);
GPIO_TypeDef *DQLow_GetPort(uint8_t output_number);

uint16_t DQHigh_GetPin(uint8_t output_number);
uint16_t DQLow_GetPin(uint8_t output_number);

void CheckInternalShortcircuits(void);

