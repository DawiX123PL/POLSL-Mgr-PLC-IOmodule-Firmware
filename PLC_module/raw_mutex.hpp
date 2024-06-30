
#include "stm32f0xx_hal.h"

class RawMutex
{
    // locked = true
    // unlocked = false
    bool mutex;

public:
    RawMutex() : mutex(0){};

    // returns true on successful acquisition
    // returns false if already locked
    bool TryLock()
    {
        bool is_lock_successfull = false;
        __disable_irq();
        if (!mutex)
        {
            mutex = true;
            is_lock_successfull = true;
        }
        __enable_irq();
        return is_lock_successfull;
    }

    void Unlock()
    {
        __disable_irq();
        mutex = false;
        __enable_irq();
    }
};
