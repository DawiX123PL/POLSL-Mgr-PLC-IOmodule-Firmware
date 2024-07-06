#pragma once

#include "main.h"



#define CHECK_INTERNAL_SHORT_CIRCUITS


void DQ_Write(uint8_t output_number, bool out_level, bool enable);
void DQ_WriteRegister(uint8_t level_reg, uint8_t enable_reg);

GPIO_TypeDef *DQHigh_GetPort(uint8_t output_number);
GPIO_TypeDef *DQLow_GetPort(uint8_t output_number);

uint16_t DQHigh_GetPin(uint8_t output_number);
uint16_t DQLow_GetPin(uint8_t output_number);

void CheckInternalShortcircuits(void);

