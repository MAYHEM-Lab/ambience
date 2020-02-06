//
// Created by fatih on 11/8/18.
//

#pragma once

#include <cstdint>
#include <array>
#include <common/i2c.hpp>
#include <tos/span.hpp>
#include "detail/ina219_constants.hpp"

namespace tos
{
template <class TwimT>
class ina219
{
public:
    template <class TwimU = TwimT>
    ina219(twi_addr_t addr, TwimU&& twim) : _addr{addr}, _twim{std::forward<TwimU>(twim)} {
        setCalibration_32V_2A();
    }

    void setCalibration_16V_400mA();
    void setCalibration_32V_2A();

    uint16_t get_power_raw() {
        uint16_t val;
        wireReadRegister(INA219_REG_POWER, &val);
        return val;
    }

    int16_t getBusVoltage_raw() {
        uint16_t value;
        wireReadRegister(INA219_REG_BUSVOLTAGE, &value);

        // Shift to the right 3 to drop CNVR and OVF and multiply by LSB
        return (int16_t)((value >> 3) * 4);
    }

    float getBusVoltage_V() {
        int16_t value = getBusVoltage_raw();
        return value * 0.001;
    }

    int16_t getCurrent_raw() {
        uint16_t value;

        // Sometimes a sharp load will reset the INA219, which will
        // reset the cal register, meaning CURRENT and POWER will
        // not be available ... avoid this by always setting a cal
        // value even if it's an unfortunate extra step
        wireWriteRegister(INA219_REG_CALIBRATION, ina219_calValue);

        // Now we can safely read the CURRENT register!
        wireReadRegister(INA219_REG_CURRENT, &value);

        return (int16_t)value;
    }

    float getCurrent_mA() {
        float valueDec = getCurrent_raw();
        valueDec /= ina219_currentDivider_mA;
        return valueDec;
    }

private:
    void wireWriteRegister (uint8_t reg, uint16_t value);
    void wireReadRegister(uint8_t reg, uint16_t *value);

    twi_addr_t _addr;
    TwimT _twim;

    uint32_t ina219_calValue;
    // The following multipliers are used to convert raw current and power
    // values to mA and mW, taking into account the current config settings
    uint32_t ina219_currentDivider_mA;
    float    ina219_powerMultiplier_mW;
};
} // namespace tos

// impl

namespace tos
{
template<class TwimT>
void ina219<TwimT>::setCalibration_16V_400mA() {

    // Calibration which uses the highest precision for
    // current measurement (0.1mA), at the expense of
    // only supporting 16V at 400mA max.

    // VBUS_MAX = 16V
    // VSHUNT_MAX = 0.04          (Assumes Gain 1, 40mV)
    // RSHUNT = 0.1               (Resistor value in ohms)

    // 1. Determine max possible current
    // MaxPossible_I = VSHUNT_MAX / RSHUNT
    // MaxPossible_I = 0.4A

    // 2. Determine max expected current
    // MaxExpected_I = 0.4A

    // 3. Calculate possible range of LSBs (Min = 15-bit, Max = 12-bit)
    // MinimumLSB = MaxExpected_I/32767
    // MinimumLSB = 0.0000122              (12uA per bit)
    // MaximumLSB = MaxExpected_I/4096
    // MaximumLSB = 0.0000977              (98uA per bit)

    // 4. Choose an LSB between the min and max values
    //    (Preferrably a roundish number close to MinLSB)
    // CurrentLSB = 0.00005 (50uA per bit)

    // 5. Compute the calibration register
    // Cal = trunc (0.04096 / (Current_LSB * RSHUNT))
    // Cal = 8192 (0x2000)

    ina219_calValue = 8192;

    // 6. Calculate the power LSB
    // PowerLSB = 20 * CurrentLSB
    // PowerLSB = 0.001 (1mW per bit)

    // 7. Compute the maximum current and shunt voltage values before overflow
    //
    // Max_Current = Current_LSB * 32767
    // Max_Current = 1.63835A before overflow
    //
    // If Max_Current > Max_Possible_I then
    //    Max_Current_Before_Overflow = MaxPossible_I
    // Else
    //    Max_Current_Before_Overflow = Max_Current
    // End If
    //
    // Max_Current_Before_Overflow = MaxPossible_I
    // Max_Current_Before_Overflow = 0.4
    //
    // Max_ShuntVoltage = Max_Current_Before_Overflow * RSHUNT
    // Max_ShuntVoltage = 0.04V
    //
    // If Max_ShuntVoltage >= VSHUNT_MAX
    //    Max_ShuntVoltage_Before_Overflow = VSHUNT_MAX
    // Else
    //    Max_ShuntVoltage_Before_Overflow = Max_ShuntVoltage
    // End If
    //
    // Max_ShuntVoltage_Before_Overflow = VSHUNT_MAX
    // Max_ShuntVoltage_Before_Overflow = 0.04V

    // 8. Compute the Maximum Power
    // MaximumPower = Max_Current_Before_Overflow * VBUS_MAX
    // MaximumPower = 0.4 * 16V
    // MaximumPower = 6.4W

    // Set multipliers to convert raw current/power values
    ina219_currentDivider_mA = 20;    // Current LSB = 50uA per bit (1000/50 = 20)
    ina219_powerMultiplier_mW = 1.0f; // Power LSB = 1mW per bit

    // Set Calibration register to 'Cal' calculated above
    wireWriteRegister(INA219_REG_CALIBRATION, ina219_calValue);

    // Set Config register to take into account the settings above
    uint16_t config = INA219_CONFIG_BVOLTAGERANGE_16V |
                      INA219_CONFIG_GAIN_1_40MV | INA219_CONFIG_BADCRES_12BIT |
                      INA219_CONFIG_SADCRES_12BIT_1S_532US |
                      INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
    wireWriteRegister(INA219_REG_CONFIG, config);
}
    template<class TwimT>
    void ina219<TwimT>::setCalibration_32V_2A() {
        // By default we use a pretty huge range for the input voltage,
        // which probably isn't the most appropriate choice for system
        // that don't use a lot of power.  But all of the calculations
        // are shown below if you want to change the settings.  You will
        // also need to change any relevant register settings, such as
        // setting the VBUS_MAX to 16V instead of 32V, etc.

        // VBUS_MAX = 32V             (Assumes 32V, can also be set to 16V)
        // VSHUNT_MAX = 0.32          (Assumes Gain 8, 320mV, can also be 0.16, 0.08, 0.04)
        // RSHUNT = 0.1               (Resistor value in ohms)

        // 1. Determine max possible current
        // MaxPossible_I = VSHUNT_MAX / RSHUNT
        // MaxPossible_I = 3.2A

        // 2. Determine max expected current
        // MaxExpected_I = 2.0A

        // 3. Calculate possible range of LSBs (Min = 15-bit, Max = 12-bit)
        // MinimumLSB = MaxExpected_I/32767
        // MinimumLSB = 0.000061              (61uA per bit)
        // MaximumLSB = MaxExpected_I/4096
        // MaximumLSB = 0,000488              (488uA per bit)

        // 4. Choose an LSB between the min and max values
        //    (Preferrably a roundish number close to MinLSB)
        // CurrentLSB = 0.0001 (100uA per bit)

        // 5. Compute the calibration register
        // Cal = trunc (0.04096 / (Current_LSB * RSHUNT))
        // Cal = 4096 (0x1000)

        ina219_calValue = 4096;

        // 6. Calculate the power LSB
        // PowerLSB = 20 * CurrentLSB
        // PowerLSB = 0.002 (2mW per bit)

        // 7. Compute the maximum current and shunt voltage values before overflow
        //
        // Max_Current = Current_LSB * 32767
        // Max_Current = 3.2767A before overflow
        //
        // If Max_Current > Max_Possible_I then
        //    Max_Current_Before_Overflow = MaxPossible_I
        // Else
        //    Max_Current_Before_Overflow = Max_Current
        // End If
        //
        // Max_ShuntVoltage = Max_Current_Before_Overflow * RSHUNT
        // Max_ShuntVoltage = 0.32V
        //
        // If Max_ShuntVoltage >= VSHUNT_MAX
        //    Max_ShuntVoltage_Before_Overflow = VSHUNT_MAX
        // Else
        //    Max_ShuntVoltage_Before_Overflow = Max_ShuntVoltage
        // End If

        // 8. Compute the Maximum Power
        // MaximumPower = Max_Current_Before_Overflow * VBUS_MAX
        // MaximumPower = 3.2 * 32V
        // MaximumPower = 102.4W

        // Set multipliers to convert raw current/power values
        ina219_currentDivider_mA = 10;  // Current LSB = 100uA per bit (1000/100 = 10)
        ina219_powerMultiplier_mW = 2;     // Power LSB = 1mW per bit (2/1)

        // Set Calibration register to 'Cal' calculated above
        wireWriteRegister(INA219_REG_CALIBRATION, ina219_calValue);

        // Set Config register to take into account the settings above
        uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
                          INA219_CONFIG_GAIN_8_320MV |
                          INA219_CONFIG_BADCRES_12BIT |
                          INA219_CONFIG_SADCRES_12BIT_1S_532US |
                          INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
        wireWriteRegister(INA219_REG_CONFIG, config);
    }

    template<class TwimT>
    void ina219<TwimT>::wireWriteRegister(uint8_t reg, uint16_t value) {
        std::array<uint8_t, 3> buf {reg, uint8_t ((value >> 8) & 0xFF), uint8_t (value & 0xFF)};
        auto res = _twim->transmit(_addr, tos::span<uint8_t>(buf));
        if (res != tos::twi_tx_res::ok) {
            tos::debug::panic("nope");
        }
    }

    template<class TwimT>
    void ina219<TwimT>::wireReadRegister(uint8_t reg, uint16_t *value) {
        _twim->transmit(_addr, span<uint8_t>{&reg, 1});

        // should there be a delay here?

        std::array<uint8_t, 2> buf{};
        auto res = _twim->receive(_addr, tos::span<uint8_t>(buf));
        if (res != tos::twi_rx_res::ok) {
            tos::debug::panic("nope");
        }
        *value = (buf[0] << 8 | buf[1]);
    }
} // namespace tos