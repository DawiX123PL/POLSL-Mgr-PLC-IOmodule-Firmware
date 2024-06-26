/*
 * math_utils.hpp
 *
 *  Created on: May 31, 2024
 *      Author: Dawid
 */

#ifndef MATH_UTILS_HPP_
#define MATH_UTILS_HPP_

#include <inttypes.h>


// result := a ** b
constexpr uint32_t pow_i(uint32_t a, uint32_t b)
{
	uint32_t result = 1;
	for (uint32_t i = 0; i < b; i++)
	{
		result *= a;
	}
	return result;
}


#endif /* MATH_UTILS_HPP_ */
