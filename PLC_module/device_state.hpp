#pragma once

#include <inttypes.h>

extern "C"
{

    enum class DeviceID : uint8_t
    {
        UNNOWN = 0,
        ANALOG_INPUT = 1,
        DIGITAL_OUTPUT = 2,
    };

    // error_byte flags
    namespace ErrorFlags{
        uint8_t internal_device_error = 0x00000001;
        uint8_t supply_voltage_to_low = 0x00000010;
    };

    struct DeviceState
    {
        DeviceID device_id = DeviceID::UNNOWN;
        uint8_t error_byte;
        uint8_t digital_input = 0;
        uint8_t digital_output_level;
        uint8_t digital_output_enable;
        uint16_t analog_input0;
        uint16_t analog_input1;
        uint16_t analog_input2;
        uint16_t analog_input3;
        uint16_t analog_input4;
        uint16_t analog_input5;
        uint16_t analog_input6;
        uint16_t analog_input7;
        uint16_t supply_voltage;
    };

    struct DeviceStateBuffer
    {
        DeviceState device_state;
        uint8_t crc;
    };
}