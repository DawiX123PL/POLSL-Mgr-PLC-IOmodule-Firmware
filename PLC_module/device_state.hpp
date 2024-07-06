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
    namespace StatusFlags{
        const uint8_t crc_error                  = 0b00000001;
        const uint8_t invalid_command            = 0b00000010;
    };

    namespace ErrorFlags{
        const uint8_t internal_device_error      = 0b00000001;
        const uint8_t supply_voltage_to_low      = 0b00000010;
        const uint8_t power_on_reset             = 0b00000100;
        const uint8_t watchdog_triggered         = 0b00001000;
        const uint8_t hardware_or_software_reset = 0b00010000;
        const uint8_t unnown_reset_source        = 0b00100000;
        const uint8_t communication_timeout      = 0b01000000;
    };

    struct DeviceState
    {
        DeviceID device_id = DeviceID::UNNOWN;
        uint8_t status_byte;
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