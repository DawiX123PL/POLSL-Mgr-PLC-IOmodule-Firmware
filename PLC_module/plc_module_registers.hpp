/*
 * module_registers.hpp
 *
 *  Created on: May 31, 2024
 *      Author: Dawid
 */

#ifndef MODULE_REGISTERS_HPP_
#define MODULE_REGISTERS_HPP_

#include <inttypes.h>
#include <cstring>

namespace Module
{
	struct CommonDataLayout
	{
		uint8_t status_byte = 0;
		uint8_t error_byte = 0;
		uint8_t _reserved0 = 0;
		uint8_t crc8_1 = 0;
	};

	struct AnalogInputDataLayout
	{
		uint8_t _reserved0 = 0;
		uint8_t _reserved1 = 0;
		uint8_t digital_input = 0;
		uint8_t _reserved3 = 0;
		uint16_t supply_voltage = 0;
		uint16_t analog_input0 = 0;
		uint16_t analog_input1 = 0;
		uint16_t analog_input2 = 0;
		uint16_t analog_input3 = 0;
		uint16_t analog_input4 = 0;
		uint16_t analog_input5 = 0;
		uint16_t analog_input6 = 0;
		uint16_t analog_input7 = 0;
		uint8_t crc8_2 = 0;
	};


	union OuputData
	{
		OuputData(){
			memset(raw_data, 0, sizeof(raw_data));
		}

		uint8_t raw_data[100];
		CommonDataLayout common_data;
		AnalogInputDataLayout analog_input_data;
	};

}

#endif /* MODULE_REGISTERS_HPP_ */
