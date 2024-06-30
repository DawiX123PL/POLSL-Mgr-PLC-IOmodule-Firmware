#pragma once

#include <inttypes.h>
#include "stm32f0xx_hal.h"

template <uint32_t _capacity>
class DoubleBuffer
{
public:
	static constexpr uint32_t capacity = _capacity;

private:
	volatile uint32_t size_0 = 0;
	volatile uint32_t size_1 = 0;

	volatile uint8_t buffer0[capacity] = {0xAA, 0xBB, 0x01};
	volatile uint8_t buffer1[capacity] = {0xAA, 0xBB, 0x02};

	// indicates which buffer is used for active data tranmision
	volatile uint8_t active_buffer_nr = 0;

	// prevents from swaping buffer during active data transmision
	// false = buffers are allowed to swap
	// true = buffers cannot be swapped
	bool swap_lock = false;

public:
	void PreventSwap()
	{
		__disable_irq();
		swap_lock = true;
		__enable_irq();
	}

	void AllowSwap()
	{
		__disable_irq();
		swap_lock = false;
		__enable_irq();
	}

	inline void TrySwap()
	{
		__disable_irq();
		if (!swap_lock)
		{
			active_buffer_nr = (active_buffer_nr + 1) % 2;
		}
		__enable_irq();
	}

	// returns pointer to active buffer
	// active = connected to communication interface
	inline volatile uint8_t *GetActive()
	{
		return (active_buffer_nr == 0) ? buffer0 : buffer1;
	}

	// returns pointer to active buffer
	// free = NOT connected to communication interface
	// it is allowed to write to this buffer
	inline volatile uint8_t *GetFree()
	{
		return (active_buffer_nr == 0) ? buffer1 : buffer0;
	}

	// buffer size
	// active = connected to communication interface
	inline volatile uint32_t &ActiveSize()
	{
		return (active_buffer_nr == 0) ? size_0 : size_1;
	}

	// buffer size
	// free = NOT connected to communication interface
	// it is allowed to write to this buffer
	inline volatile uint32_t &FreeSize()
	{
		return (active_buffer_nr == 0) ? size_1 : size_0;
	}
};
