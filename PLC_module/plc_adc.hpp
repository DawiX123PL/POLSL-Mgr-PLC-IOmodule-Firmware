/*
 * plc_adc.hpp
 *
 *  Created on: May 31, 2024
 *      Author: Dawid
 */

#ifndef PLC_ADC_HPP_
#define PLC_ADC_HPP_

#include <inttypes.h>
#include "math_utils.hpp"

constexpr uint32_t resistor_r1 = 62000; // [Ohm]
constexpr uint32_t resistor_r2 = 10000; // [Ohm]
constexpr double voltage_divider_gain = (double)resistor_r2 / (double)(resistor_r1 + resistor_r2); // [1]

constexpr uint32_t adc_bytes = 12;	   // [1]
constexpr double supply_voltage = 3.3; // [V]
constexpr double quant_voltage = supply_voltage / pow_i(2, adc_bytes); // [V]

constexpr uint32_t volt_to_mv = 1000;// [mV/V]

constexpr double adc_to_mv = quant_voltage * volt_to_mv / voltage_divider_gain; // [V]

// this magic number is used only to boost accuracy of calculations.
// adc_to_mv_const is multiplied by this magic number before conversion to int, and result is divided by 1024.
// this boost accuracy without necessity of using float or double.
//
// multiplying and dividing integer by 65536 = (1<<16) should be very fast (theoreticaly requires only bitshift operation)
constexpr uint32_t multiply_magic_number = (1 << 16);

constexpr uint32_t adc_to_mv_int = adc_to_mv * (double)multiply_magic_number;

//uint16_t AdcToMiliVolt(uint16_t measured_val);

static inline uint16_t AdcToMiliVolt(uint16_t measured_val)
{
	return measured_val * adc_to_mv_int / multiply_magic_number;
}



#endif /* PLC_ADC_HPP_ */
