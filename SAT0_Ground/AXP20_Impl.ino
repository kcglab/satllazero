/**
    @file APX20_Impl.ino
    @brief APX20 Untils for SATLLA0 GS.

    This file contains APX20 utils for the SATLLA0 GS.

    Copyright (C) 2023 @author Rony Ronen

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#if AXP20_ENABLE

#include <axp20x.h>

AXP20X_Class axp20;

void axp20_setup()
{
    PRINTLN("Func: axp20_setup()");

    Wire.begin(21, 22); // SCL, SDA
    if (!axp20.begin(Wire, AXP192_SLAVE_ADDRESS))
    {
        Serial.println("AXP20 Begin PASS");
    }
    else
    {
        Serial.println("AXP20 Begin FAIL");
    }
    axp20.setPowerOutPut(AXP192_LDO2, AXP202_ON);
    axp20.setPowerOutPut(AXP192_LDO3, AXP202_ON);
    axp20.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
    axp20.setPowerOutPut(AXP192_EXTEN, AXP202_ON);
    axp20.setPowerOutPut(AXP192_DCDC1, AXP202_ON);

    /*Enable AXP ADC function*/
    axp20.adc1Enable(AXP202_VBUS_VOL_ADC1 |
                         AXP202_VBUS_CUR_ADC1 |
                         AXP202_BATT_CUR_ADC1 |
                         AXP202_BATT_VOL_ADC1,
                     true);
}

void axp20_status()
{
    PRINTLN("Func: axp20_status()");
    memset(battery_data_g.packet, 0x00, BATTERY_PACKET_LENGTH);
    // energy_data_g.apx20_vbusvoltage = axp20.getVbusVoltage() * 10;
    // energy_data_g.apx20_vbuscurrent_ma = axp20.getVbusCurrent() * 10;

    battery_data_g.battery_volts = axp20.getBattVoltage() * 10;
    if (axp20.isChargeing())
    {
        battery_data_g.battery_current = axp20.getBattChargeCurrent() * 10;
    }
    else
    {
        battery_data_g.battery_current = axp20.getBattDischargeCurrent() * -10;
    }

    // battery_data_g.apx20_temp = axp20.getTemp() * 100;
}

void axp20_raw_print()
{
    PRINTLN("Func: axp20_raw_print()");

    PRINT("getVbusVoltage:\t\t");
    PRINTLN(axp20.getVbusVoltage());
    PRINT("getVbusCurrent:\t\t");
    PRINTLN(axp20.getVbusCurrent());
    PRINT("getBattVoltage:\t\t");
    PRINTLN(axp20.getBattVoltage());
    if (axp20.isChargeing())
    {
        PRINT("getBattChargeCurrent:\t");
        PRINTLN(axp20.getBattChargeCurrent());
    }
    else
    {
        PRINT("getBattDischargeCurrent:\t");
        PRINTLN(axp20.getBattDischargeCurrent());
    }
    PRINT("getTemp:\t\t");
    PRINTLN(axp20.getTemp());
}
#endif

/* ============ */
/* Print        */
/* ============ */

void axp20_data_print(battery_data_t *battery_data)
{
    PRINTLN("Func: axp20_data_print()");

    PRINT("BAT Voltage:\t");
    PRINT((float)(battery_data->battery_volts) / 10);
    PRINTLN(" mV");
    PRINT("BAT Current:\t");
    PRINT((float)(battery_data->battery_current) / 10);
    PRINTLN(" mA");
}
