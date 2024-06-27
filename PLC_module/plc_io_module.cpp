
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

volatile int32_t spi_received_bytes = 0;

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
		//	detected risign edge (master finished communication)
		spi_received_bytes = spi_buffer_size - __HAL_DMA_GET_COUNTER(hspi1.hdmarx);
		status = HAL_SPI_Abort(&hspi1);
		status = HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t *)spi_tx_buffer0, (uint8_t *)spi_rx_buffer0, spi_buffer_size);

}

void HAL_SPI_AbortCpltCallback(SPI_HandleTypeDef *hspi)
{
	status = HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t *)spi_tx_buffer0, (uint8_t *)spi_rx_buffer0, spi_buffer_size);
}

// ------------------------------------------------------------------------------------------
// External interrupt on CS pin

extern "C" void EXTI4_15_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}

// ------------------------------------------------------------------------------------------
// Configure Interrupt on SPI1 CS pin

void ConfigureSPI_CS_Interrupt(void)
{
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/*Configure GPIO pin : PtPin */
	GPIO_InitStruct.Pin = SPI1_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(SPI1_CS_GPIO_Port, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}

// ------------------------------------------------------------------------------------------
// main

uint8_t output = 0;
OutputState state = OutputState::FLOATING;

int main(void)
{
	HAL_Init();

	SystemClock_Config();

	MX_GPIO_Init();
	MX_DMA_Init();
	MX_ADC_Init();
	MX_TIM3_Init();

	// External Interrupt MUST be initialized, before spi
	// Thanks DAnsp
	// https://community.st.com/t5/stm32-mcus-products/spi-slave-nss-interrupt/td-p/394501
	ConfigureSPI_CS_Interrupt();
	MX_SPI1_Init();

	HAL_SPI_GetState(&hspi1);

	// enable adc
	HAL_ADCEx_Calibration_Start(&hadc);
	while (HAL_ADC_GetState(&hadc) != HAL_ADC_STATE_READY)
	{
	} // todo: handle error case

	HAL_ADC_Start_DMA(&hadc, (uint32_t *)&adc_conversion_result, ADCConversionResult::length);
	HAL_TIM_Base_Start_IT(&htim3);

	// enable SPI interface
	HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t *)spi_tx_buffer0, (uint8_t *)spi_rx_buffer0, spi_buffer_size);



	while (1)
	{

		DQ_Write(output, state);
		CheckInternalShortcircuits();

//		DQ_Write(output, OutputState::LOW);
//		CheckInternalShortcircuits();
//		HAL_Delay(500);
//
//		DQ_Write(output, OutputState::HIGH);
//		CheckInternalShortcircuits();
//		HAL_Delay(500);
//
//		DQ_Write(output, OutputState::FLOATING);
//		CheckInternalShortcircuits();
//		HAL_Delay(500);
//
//		output = output < 7 ? output + 1 : 0;
	}
}
