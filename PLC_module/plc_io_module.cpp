
#include <inttypes.h>
#include <math.h>
#include <cstring>
#include "main.h"
#include "iwdg.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"
#include "spi.h"
#include "crc.h"
#include "plc_adc.hpp"
#include "output_ports.hpp"
#include "device_state.hpp"
#include "raw_mutex.hpp"
#include "double_buffer.hpp"

HAL_StatusTypeDef status;

// ------------------------------------------------------------------------------------------
// performance measurements

struct Performance
{

	static inline uint16_t GetTick()
	{
		return __HAL_TIM_GET_COUNTER(&htim_performance_measure);
	}

	static inline uint16_t CalculateDuration(uint16_t start_tick)
	{
		return GetTick() - start_tick;
	}

	uint16_t adc_result_parse;
	uint16_t init_spi_communication;
	uint16_t finish_spi_communication;
	uint16_t update_output;
	uint16_t update_spi_tx_buffer;
	uint16_t main_loop;
};

volatile Performance performance;

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

constexpr uint32_t spi_buffer_size = 50;

DoubleBuffer<spi_buffer_size> spi_tx_buffer;
DoubleBuffer<spi_buffer_size> spi_rx_buffer;

// ------------------------------------------------------------------------------------------
// ADC callbacks

extern "C" void
HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	ConversionTime = htim3.Instance->CNT;

	uint16_t start_tick = Performance::GetTick(); // get timer tick before parsing adc data

	const uint16_t supply_voltage_raw = adc_conversion_result.input_voltage;
	const uint16_t supply_voltage = AdcToMiliVolt(supply_voltage_raw);
	const uint16_t digital_threshold = supply_voltage_raw >> 1; // supply_voltage / 2

	uint8_t digital_input = 0;

	{
		digital_input |= (adc_conversion_result.analog_input_0 > digital_threshold) ? (1 << 0) : 0;
		digital_input |= (adc_conversion_result.analog_input_1 > digital_threshold) ? (1 << 1) : 0;
		digital_input |= (adc_conversion_result.analog_input_2 > digital_threshold) ? (1 << 2) : 0;
		digital_input |= (adc_conversion_result.analog_input_3 > digital_threshold) ? (1 << 3) : 0;
		digital_input |= (adc_conversion_result.analog_input_4 > digital_threshold) ? (1 << 4) : 0;
		digital_input |= (adc_conversion_result.analog_input_5 > digital_threshold) ? (1 << 5) : 0;
		digital_input |= (adc_conversion_result.analog_input_6 > digital_threshold) ? (1 << 6) : 0;
		digital_input |= (adc_conversion_result.analog_input_7 > digital_threshold) ? (1 << 7) : 0;
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

	performance.adc_result_parse = Performance::CalculateDuration(start_tick); // store calculation performance
}

// ------------------------------------------------------------------------------------------
// Parse command

void ParseCommand(const uint8_t *const data, uint32_t size)
{
	constexpr uint8_t command_nop = 0;
	constexpr uint8_t command_clear_error = 1;
	constexpr uint8_t command_write_dq = 2;

	// command must contain command_byte and crc_byte
	if (size < 2)
	{
		return;
	}

	// step 1 - verify crc

	uint8_t crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)data, size - 1);
	if (data[size - 1] != crc)
	{
		// crc not match
		if (current_state_lock.TryLock())
		{
			current_device_state.status_byte |= StatusFlags::crc_error;
			current_state_lock.Unlock();
		}
		// return;
	}

	// reset spi timeout timer
	__HAL_TIM_SetCounter(&htim_spi_timeout, 0);

	// step 2 - check command and excecude
	uint8_t command = data[0];

	// [command; crc]
	if (command == command_nop)
	{
		return;
	}

	// [command; crc]
	if (command == command_clear_error)
	{
		if (current_state_lock.TryLock())
		{
			current_device_state.error_byte = 0;
			current_state_lock.Unlock();
		}
		return;
	}

	// [command; digital_output_level; digital_output_enable; crc]
	if (command == command_write_dq)
	{
		if (size < 4)
		{
			if (current_state_lock.TryLock())
			{
				current_device_state.status_byte |= StatusFlags::invalid_command;
				current_state_lock.Unlock();
			}
			return;
		}

		if (current_state_lock.TryLock())
		{
			current_device_state.digital_output_level = data[1];
			current_device_state.digital_output_enable = data[2];
			current_state_lock.Unlock();
		}
		return;
	}

	// invalid command
	if (current_state_lock.TryLock())
	{
		current_device_state.status_byte |= StatusFlags::invalid_command;
		current_state_lock.Unlock();
	}
}

void ParseSpiCommand()
{
	spi_rx_buffer.TrySwap();

	if (spi_rx_buffer.FreeSize())
	{
		ParseCommand((uint8_t *)spi_rx_buffer.GetFree(), spi_rx_buffer.FreeSize());
	}

	spi_rx_buffer.FreeSize() = 0;
	current_state_lock.Unlock();
}

// ------------------------------------------------------------------------------------------
// SPI callbacks

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &htim_spi_timeout)
	{
		// spi communication timeout
		if (current_state_lock.TryLock())
		{
			current_device_state.error_byte |= ErrorFlags::communication_timeout;
			current_state_lock.Unlock();
		}
	}
}

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
	uint16_t start_tick = Performance::GetTick();

	if (HAL_GPIO_ReadPin(SPI1_CS_GPIO_Port, SPI1_CS_Pin) == GPIO_PIN_SET)
	{
		uint32_t received_bytes = spi_rx_buffer.capacity - __HAL_DMA_GET_COUNTER(hspi1.hdmarx);
		spi_rx_buffer.ActiveSize() = received_bytes;

		spi_tx_buffer.AllowSwap();
		spi_rx_buffer.AllowSwap();
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

		performance.finish_spi_communication = Performance::CalculateDuration(start_tick);

		// TODO: parse received data from SPI
		ParseSpiCommand();
	}
	else
	{
		// falling edge (master started communication)
		// setup spi transmision
		spi_tx_buffer.PreventSwap();
		spi_rx_buffer.PreventSwap();

		status = HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t *)spi_tx_buffer.GetActive(), (uint8_t *)spi_rx_buffer.GetActive(), spi_buffer_size);

		performance.init_spi_communication = Performance::CalculateDuration(start_tick);
	}
}

void HAL_SPI_AbortCpltCallback(SPI_HandleTypeDef *hspi)
{
}

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

static void LedRed(bool enable)
{
	GPIO_PinState state = enable ? GPIO_PIN_SET : GPIO_PIN_RESET;
	HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, state);
}

// static void LedOrange(bool enable)
//{
//	GPIO_PinState state = enable ? GPIO_PIN_SET : GPIO_PIN_RESET;
//	HAL_GPIO_WritePin(LED_ORANGE_GPIO_Port, LED_ORANGE_Pin, state);
// }

static void LedGreen(bool enable)
{
	GPIO_PinState state = enable ? GPIO_PIN_SET : GPIO_PIN_RESET;
	HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, state);
}

// ------------------------------------------------------------------------------------------
// get reset reason

void SetInitialErrorFlag()
{
	uint8_t errorbyte = 0;

	if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST)) // POWER_ON_RESET
	{
		errorbyte |= ErrorFlags::power_on_reset;
	}

	if (__HAL_RCC_GET_FLAG(RCC_CSR_V18PWRRSTF)) // POWER_ON_1_8_DOMAIN_RESET
	{
		errorbyte |= ErrorFlags::power_on_reset;
	}

	if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)) // INDEPENDENT_WATCHDOG
	{
		errorbyte |= ErrorFlags::watchdog_triggered;
	}

	if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST)) // WINDOW_WATCHDOG
	{
		errorbyte |= ErrorFlags::watchdog_triggered;
	}

	if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST)) // NRST_PIN_RESET
	{
		errorbyte |= ErrorFlags::hardware_or_software_reset;
	}

	if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST)) // SOFTWARE_RESET
	{
		errorbyte |= ErrorFlags::hardware_or_software_reset;
	}

	if (__HAL_RCC_GET_FLAG(RCC_FLAG_OBLRST)) // OPTION_BYTE_LOADER
	{
		errorbyte |= ErrorFlags::unnown_reset_source;
	}

	if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST)) // LOW_POWER_MANAGEMENT
	{
		errorbyte |= ErrorFlags::unnown_reset_source;
	}

	current_device_state.error_byte = errorbyte;
}

// ------------------------------------------------------------------------------------------
// update Digital Output and leds routine

void UpdateOutputRoutine()
{
	// check if state buffer has been updated
	uint8_t error_byte = 0;

	// this holds digital output levels (1-Hi or 0-Low)
	uint8_t output_level = 0;
	// this tells if output is enabled or not (1-Enabled, 0-Hi impedance)
	uint8_t output_enable = 0;

	if (current_state_lock.TryLock())
	{
		error_byte = current_device_state.error_byte;
		if (error_byte)
		{
			current_device_state.digital_output_enable = 0;
			current_device_state.digital_output_level = 0;
		}
		output_enable = current_device_state.digital_output_enable;
		output_level = current_device_state.digital_output_level;

		current_state_lock.Unlock();
	}

	// update digital outputs
	if (error_byte)
	{
		DQ_WriteRegister(0, 0);
		CheckInternalShortcircuits();
	}
	else
	{
		DQ_WriteRegister(output_level, output_enable);
		CheckInternalShortcircuits();
	}

	// blink red led in case of error;
	const uint32_t blink_time = 1000;
	const uint32_t blink_enable_time = 200;

	bool pwm_level = (HAL_GetTick() % blink_time) < blink_enable_time;
	bool led_red_level = (error_byte != 0) && pwm_level;
	LedRed(led_red_level);
}

// ------------------------------------------------------------------------------------------
// update spi buffer

void UpdateSpiTxBufferRoutine()
{
	if (current_state_lock.TryLock())
	{
		// if (current_state_updated)
		{
			// copy current state to spi buffer
			DeviceStateBuffer *spi_buf = (DeviceStateBuffer *)spi_tx_buffer.GetFree();
			DeviceState *spi_state_buf = &(spi_buf->device_state);

			// move data to spi
			memcpy(spi_state_buf, &current_device_state, sizeof(DeviceState));

			// callculate crc
			spi_buf->crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)&current_device_state, sizeof(DeviceState));

			// notify that content of buffer has been moved to spi_buffer
			current_state_updated = false;

			spi_tx_buffer.TrySwap();
		}
		current_state_lock.Unlock();
	}
}

// ------------------------------------------------------------------------------------------
// configure device

void ConfigureDevice()
{
	HAL_Init();

	// immediately enable WatchDog
	MX_IWDG_Init();

	// setup device error flags
	SetInitialErrorFlag();

	// clear reset flags
	__HAL_RCC_CLEAR_RESET_FLAGS();

	// initialize other peripherals
	SystemClock_Config();
	MX_CRC_Init();
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_ADC_Init();
	MX_TIM3_Init();
	MX_TIM17_Init();
	MX_TIM16_Init();

	{
		// External Interrupt MUST be initialized, before spi
		// Thanks DAnsp
		// https://community.st.com/t5/stm32-mcus-products/spi-slave-nss-interrupt/td-p/394501
		ConfigureSPI_CS_Interrupt();
		ConfigureSPI1();
		// MX_SPI1_Init(); // this line can be usefull in future

		HAL_TIM_Base_Start_IT(&htim_spi_timeout);
	}

	// enable adc
	{
		HAL_ADCEx_Calibration_Start(&hadc);
		while (HAL_ADC_GetState(&hadc) != HAL_ADC_STATE_READY)
		{
		} // todo: handle error case

		HAL_ADC_Start_DMA(&hadc, (uint32_t *)&adc_conversion_result, ADCConversionResult::length);
		HAL_TIM_Base_Start_IT(&htim_adc_trigger);
	}

	// enable clock for performance measurements
	HAL_TIM_Base_Start(&htim_performance_measure);

	// signal that device is working correctly
	LedGreen(true);
}

// ------------------------------------------------------------------------------------------
// main

int main(void)
{

	assert_param(spi_buffer_size > sizeof(DeviceStateBuffer));
	assert_param(spi_tx_buffer.capacity > sizeof(DeviceStateBuffer));

	ConfigureDevice();

	while (1)
	{
		// get current time to measure software performance
		uint16_t start_tick = Performance::GetTick();

		// reset watchdog
		__HAL_IWDG_RELOAD_COUNTER(&hiwdg);

		{
			// get current time to measure software performance
			uint16_t start_tick = Performance::GetTick();

			// update digital outputs and leds
			UpdateOutputRoutine();

			// calculate software performance
			performance.update_output = Performance::CalculateDuration(start_tick);
		}

		{
			// get current time to measure software performance
			uint16_t start_tick = Performance::GetTick();

			// update spi tx register
			UpdateSpiTxBufferRoutine();

			// calculate software performance
			performance.update_spi_tx_buffer = Performance::CalculateDuration(start_tick);
		}

		// calculate software performance
		performance.main_loop = Performance::CalculateDuration(start_tick);
	}
}
