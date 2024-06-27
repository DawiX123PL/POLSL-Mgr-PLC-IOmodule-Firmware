
#include <inttypes.h>
#include <math.h>
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"
#include "spi.h"
#include "plc_module_registers.hpp"
#include "plc_adc.hpp"
#include "output_ports.hpp"

// ------------------------------------------------------------------------------------------
// ADC buffers

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

// ------------------------------------------------------------------------------------------
// SPI buffers

constexpr int32_t spi_buffer_size = 20;

volatile uint8_t spi_rx_buffer0[spi_buffer_size] = {0};
volatile uint8_t spi_tx_buffer0[spi_buffer_size] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

// ------------------------------------------------------------------------------------------
// ADC callbacks

extern "C" void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	ConversionTime = htim3.Instance->CNT;
	HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
}

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	HAL_GPIO_TogglePin(LED_ORANGE_GPIO_Port, LED_ORANGE_Pin);
}

// ------------------------------------------------------------------------------------------
// SPI callbacks

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	//	called when all data is transfered
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	// called when rx buffer is full (rx data overflow)
}

HAL_StatusTypeDef status;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	// this is called whenever nss pin changes state
	if (HAL_GPIO_ReadPin(SPI1_CS_GPIO_Port, SPI1_CS_Pin))
	{
		//	detected risign edge (master finished communication)
		status = HAL_SPI_Abort(&hspi1);
		status = HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t *)spi_tx_buffer0, (uint8_t *)spi_rx_buffer0, spi_buffer_size);
	}
	else
	{
		//	detected falling edge (master started communication)
	}
}

void HAL_SPI_AbortCpltCallback(SPI_HandleTypeDef *hspi)
{
	status = HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t *)spi_tx_buffer0, (uint8_t *)spi_rx_buffer0, spi_buffer_size);
}

// ------------------------------------------------------------------------------------------
// Configure Interrupt on SPI1 CS pin



// ------------------------------------------------------------------------------------------
// main

int main(void)
{
	HAL_Init();

	SystemClock_Config();

	MX_GPIO_Init();
	MX_DMA_Init();
	MX_ADC_Init();
	MX_TIM3_Init();
//	MX_SPI1_Init();
	ConfigureSPI_CS_Interrupt();

	// enable adc
	HAL_ADCEx_Calibration_Start(&hadc);
	while (HAL_ADC_GetState(&hadc) != HAL_ADC_STATE_READY)
	{
	} // todo: handle error case

	HAL_ADC_Start_DMA(&hadc, (uint32_t *)&adc_conversion_result, ADCConversionResult::length);
	HAL_TIM_Base_Start_IT(&htim3);

	// enable SPI interface
//	HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t *)spi_tx_buffer0, (uint8_t *)spi_rx_buffer0, spi_buffer_size);

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
