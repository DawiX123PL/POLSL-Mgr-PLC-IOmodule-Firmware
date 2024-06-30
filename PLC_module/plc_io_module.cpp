
#include <inttypes.h>
#include <math.h>
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"
#include "spi.h"
#include "crc.h"
#include "plc_module_registers.hpp"
#include "plc_adc.hpp"
#include "output_ports.hpp"
#include "device_state.hpp"
#include "raw_mutex.hpp"





HAL_StatusTypeDef status;

// ------------------------------------------------------------------------------------------
// modules current state

RawMutex current_state_lock;
DeviceState current_device_state;
bool current_state_updated = false;

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
// spi buffers

// RawMutex current_state_lock;

constexpr int32_t spi_buffer_size = 50;

volatile uint8_t spi_rx_buffer[spi_buffer_size] = {0};

volatile uint8_t spi_tx_buffer0[spi_buffer_size] = {0xAA, 0xBB, 0x01};
volatile uint8_t spi_tx_buffer1[spi_buffer_size] = {0xAA, 0xBB, 0x02};

volatile uint8_t spi_active_tx_buffer_nr = 0;

inline volatile uint8_t *GetActiveSpiBuffer()
{
	return (spi_active_tx_buffer_nr == 0) ? spi_tx_buffer0 : spi_tx_buffer1;
}

inline volatile uint8_t *GetFreeSpiBuffer()
{
	return (spi_active_tx_buffer_nr == 0) ? spi_tx_buffer1 : spi_tx_buffer0;
}

inline void TrySwapSpiTxBuffer()
{
	// TODO: protect this with mutex or disable interrupts
	spi_active_tx_buffer_nr = (spi_active_tx_buffer_nr + 1) % 2;
}

// ------------------------------------------------------------------------------------------
// ADC callbacks

extern "C" void
HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	ConversionTime = htim3.Instance->CNT;
	HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);

	const uint16_t supply_voltage = AdcToMiliVolt(adc_conversion_result.input_voltage);
	const uint16_t digital_threshold = supply_voltage >> 1; // supply_voltage / 2

	uint8_t digital_input = 0;

	{
		digital_input |= adc_conversion_result.analog_input_0 > digital_threshold ? (1 << 0) : 0;
		digital_input |= adc_conversion_result.analog_input_1 > digital_threshold ? (1 << 1) : 0;
		digital_input |= adc_conversion_result.analog_input_2 > digital_threshold ? (1 << 2) : 0;
		digital_input |= adc_conversion_result.analog_input_3 > digital_threshold ? (1 << 3) : 0;
		digital_input |= adc_conversion_result.analog_input_4 > digital_threshold ? (1 << 4) : 0;
		digital_input |= adc_conversion_result.analog_input_5 > digital_threshold ? (1 << 5) : 0;
		digital_input |= adc_conversion_result.analog_input_6 > digital_threshold ? (1 << 6) : 0;
		digital_input |= adc_conversion_result.analog_input_7 > digital_threshold ? (1 << 7) : 0;
	}

	if (current_state_lock.TryLock())
	{

		current_device_state.analog_input0 = adc_conversion_result.analog_input_0;
		current_device_state.analog_input1 = adc_conversion_result.analog_input_1;
		current_device_state.analog_input2 = adc_conversion_result.analog_input_2;
		current_device_state.analog_input3 = adc_conversion_result.analog_input_3;
		current_device_state.analog_input4 = adc_conversion_result.analog_input_4;
		current_device_state.analog_input5 = adc_conversion_result.analog_input_5;
		current_device_state.analog_input6 = adc_conversion_result.analog_input_6;
		current_device_state.analog_input7 = adc_conversion_result.analog_input_7;
		current_device_state.analog_input7 = adc_conversion_result.analog_input_7;
		current_device_state.supply_voltage = supply_voltage;
		current_device_state.digital_input = digital_input;

		// notify that content of buffer has changed
		current_state_updated = true;

		current_state_lock.Unlock();
	}
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

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (HAL_GPIO_ReadPin(SPI1_CS_GPIO_Port, SPI1_CS_Pin) == GPIO_PIN_SET)
	{
		// rising edge (master finished communication)

		// solution to tx fifo problem found there
		// https://community.st.com/t5/stm32-mcus-products/stm32f0-spi-txfifo-flush/m-p/351737

		// restart spi procedure
		// step 1) deinit spi module
		if (HAL_SPI_DeInit(&hspi1) != HAL_OK)
		{
			Error_Handler();
		}

		// step 2) reset spi via RCC
		__HAL_RCC_SPI1_FORCE_RESET();
		__HAL_RCC_SPI1_RELEASE_RESET();

		// step 3) init spi
		if (HAL_SPI_Init(&hspi1) != HAL_OK)
		{
			Error_Handler();
		}
	}
	else
	{
		// falling edge (master started communication)
		// setup spi transmision
		status = HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t *)GetActiveSpiBuffer(), (uint8_t *)spi_rx_buffer, spi_buffer_size);
	}
}

void HAL_SPI_AbortCpltCallback(SPI_HandleTypeDef *hspi)
{
	status = HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t *)GetActiveSpiBuffer(), (uint8_t *)spi_rx_buffer, spi_buffer_size);
}

// ------------------------------------------------------------------------------------------
// External interrupt on CS pin

// extern "C" void EXTI4_15_IRQHandler(void)
// {
// 	// HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);

// 	// spi_active_tx_buffer_nr = (spi_active_tx_buffer_nr + 1) % 2;
// }

// ------------------------------------------------------------------------------------------
// Configure Interrupt on SPI1 CS pin

void ConfigureSPI_CS_Interrupt(void)
{
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/*Configure GPIO pin : PtPin */
	GPIO_InitStruct.Pin = SPI1_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(SPI1_CS_GPIO_Port, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}

// ------------------------------------------------------------------------------------------
// Configure SPI1
// STM32 Cube randomly changes this configuracion so I had to copy STm32Cube generated function

void ConfigureSPI1(void)
{
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_SLAVE;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_HARD_INPUT;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_ENABLE;
	hspi1.Init.CRCPolynomial = 7;
	hspi1.Init.CRCLength = SPI_CRC_LENGTH_8BIT;
	hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
	if (HAL_SPI_Init(&hspi1) != HAL_OK)
	{
		Error_Handler();
	}
}

// ------------------------------------------------------------------------------------------
// LEDS

static void LedRed(uint8_t enable)
{
	GPIO_PinState state = enable ? GPIO_PIN_SET : GPIO_PIN_RESET;
	HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, state);
}

static void LedOrange(uint8_t enable)
{
	GPIO_PinState state = enable ? GPIO_PIN_SET : GPIO_PIN_RESET;
	HAL_GPIO_WritePin(LED_ORANGE_GPIO_Port, LED_ORANGE_Pin, state);
}

static void LedGreen(uint8_t enable)
{
	GPIO_PinState state = enable ? GPIO_PIN_SET : GPIO_PIN_RESET;
	HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, state);
}

static void LedRedToggle()
{
	HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
}

static void LedOrangeToggle()
{
	HAL_GPIO_TogglePin(LED_ORANGE_GPIO_Port, LED_ORANGE_Pin);
}

static void LedGreenToggle()
{
	HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
}

// ------------------------------------------------------------------------------------------
// get reset reason

enum class ResetReason
{
	UNNOWN = 0,
	LOW_POWER_MANAGEMENT,
	WINDOW_WATCHDOG,
	INDEPENDENT_WATCHDOG,
	SOFTWARE_RESET,
	POWER_ON_RESET,
	POWER_ON_1_8_DOMAIN_RESET,
	NRST_PIN_RESET,
	OPTION_BYTE_LOADER,
};

ResetReason GetResetReason()
{
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_OBLRST))
		return ResetReason::OPTION_BYTE_LOADER;
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))
		return ResetReason::NRST_PIN_RESET;
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))
		return ResetReason::POWER_ON_RESET;
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
		return ResetReason::SOFTWARE_RESET;
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
		return ResetReason::INDEPENDENT_WATCHDOG;
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST))
		return ResetReason::WINDOW_WATCHDOG;
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST))
		return ResetReason::LOW_POWER_MANAGEMENT;
	if (__HAL_RCC_GET_FLAG(RCC_CSR_V18PWRRSTF))
		return ResetReason::POWER_ON_1_8_DOMAIN_RESET;

	return ResetReason::UNNOWN;
}

void SetInitialErrorFlag()
{
	ResetReason reset_reason = GetResetReason();

	switch (reset_reason)
	{
	case ResetReason::UNNOWN:
		current_device_state.error_byte |= ErrorFlags::unnown_reset_source;
		break;

	case ResetReason::NRST_PIN_RESET:
		current_device_state.error_byte |= ErrorFlags::unnown_reset_source;
		break;

	case ResetReason::OPTION_BYTE_LOADER:
		current_device_state.error_byte |= ErrorFlags::unnown_reset_source;
		break;

	case ResetReason::LOW_POWER_MANAGEMENT:
		current_device_state.error_byte |= ErrorFlags::hardware_or_software_reset;
		break;

	case ResetReason::SOFTWARE_RESET:
		current_device_state.error_byte |= ErrorFlags::hardware_or_software_reset;
		break;

	case ResetReason::WINDOW_WATCHDOG:
		current_device_state.error_byte |= ErrorFlags::watchdog_triggered;
		break;

	case ResetReason::INDEPENDENT_WATCHDOG:
		current_device_state.error_byte |= ErrorFlags::watchdog_triggered;
		break;

	case ResetReason::POWER_ON_RESET:
		current_device_state.error_byte |= ErrorFlags::power_on_reset;
		break;

	case ResetReason::POWER_ON_1_8_DOMAIN_RESET:
		current_device_state.error_byte |= ErrorFlags::power_on_reset;
		break;

	default:
		current_device_state.error_byte |= ErrorFlags::unnown_reset_source;
	}
}

// ------------------------------------------------------------------------------------------
// main

int main(void)
{

	assert_param(spi_buffer_size > sizeof(DeviceStateBuffer));

	SetInitialErrorFlag();

	// clear reset flags
	__HAL_RCC_CLEAR_RESET_FLAGS();

	// initialize other peripherals

	HAL_Init();

	SystemClock_Config();
	MX_CRC_Init();
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_ADC_Init();
	MX_TIM3_Init();

	// External Interrupt MUST be initialized, before spi
	// Thanks DAnsp
	// https://community.st.com/t5/stm32-mcus-products/spi-slave-nss-interrupt/td-p/394501
	ConfigureSPI_CS_Interrupt();
	ConfigureSPI1();
	// MX_SPI1_Init(); // this line can be usefull in future

	HAL_SPI_GetState(&hspi1);

	// enable adc
	HAL_ADCEx_Calibration_Start(&hadc);
	while (HAL_ADC_GetState(&hadc) != HAL_ADC_STATE_READY)
	{
	} // todo: handle error case

	HAL_ADC_Start_DMA(&hadc, (uint32_t *)&adc_conversion_result, ADCConversionResult::length);
	HAL_TIM_Base_Start_IT(&htim_adc_trigger);

	// enable SPI interface
	status = HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t *)GetActiveSpiBuffer(), (uint8_t *)spi_rx_buffer, spi_buffer_size);

	while (1)
	{

		// update digital outputs and leds
		{
			// check if state buffer has been updated
			uint8_t error_byte = 0;

			// this holds digital output levels (1-Hi or 0-Low)
			uint8_t output_level = 0;
			// this tells if output is enabled or not (1-Enabled, 0-Hi impedance)
			uint8_t output_enable = 0;

			__disable_irq();
			output_enable = current_device_state.digital_output_enable;
			output_level = current_device_state.digital_output_level;
			error_byte = current_device_state.error_byte;
			__enable_irq();

			// update digital outputs
			DQ_WriteRegister(output_level, output_enable);
			CheckInternalShortcircuits();

			// blink red led in case of error;
			const uint32_t blink_time = 1000;
			const uint32_t blink_enable_time = 200;

			bool pwm_level = (HAL_GetTick() % blink_time) < blink_enable_time;
			bool led_red_level = (error_byte != 0) && pwm_level;
			LedRed(led_red_level);
		}

		// update spi tx register
		{
			__disable_irq();
			if (current_state_lock.TryLock())
			{

				if (current_state_updated)
				{
					// copy current state to spi buffer
					DeviceStateBuffer *spi_buf = (DeviceStateBuffer *)GetFreeSpiBuffer();
					DeviceState *spi_state_buf = &(spi_buf->device_state);

					// move data to spi
					memcpy(spi_state_buf, &current_device_state, sizeof(DeviceState));

					// callculate crc
					spi_buf->crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)&current_device_state, sizeof(DeviceState));

					// notify that content of buffer has been moved to spi_buffer
					current_state_updated = false;

					TrySwapSpiTxBuffer();
				}
				current_state_lock.Unlock();
			}
		}
	}
}
