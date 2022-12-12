/**
    @file Sensor_NTC.ino
    @brief Thermistors utils functions for SATLLA0.

    This file contains NTC sensors functionality for the SATLLA0.

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

#if NTC_ENABLE

#define ABS_ZERO_KELVIN 298.15    // Base temperature in Kelvin (default should be at 25 degree)
#define ABS_ZERO_FARENHITE 273.15 // Base temperature in Farenhite
#define R1 100000                 // Thermistor resistor rating at based temperature (25 degree celcius)
#define VOLTAGE_DIVIDER_R1 100000 // Resistor value in R1 for voltage devider method
#define B_VALUE 4300              // The B Value of the thermistor for the temperature measuring range
#define NAT_LOG_e 2.718281828     // The value of e use for calculation in Temperature


// NTC Globals
float ntc1 = 0.0, ntc2 = 0.0; //, ntc3 = 0.0;
#define NUM_OF_SAMPLES 5

/* ============ */
/* Setup        */
/* ============ */

void sensor_ntc_setup()
{
    PRINTLN("Func: sensor_ntc_setup()");
    // analogReference(DEFAULT); //TODO: Verify
    pinMode(NTC1_PIN, INPUT);
    pinMode(NTC2_PIN, INPUT);
    // pinMode(NTC3_PIN, INPUT);
}

/* ============ */
/* NTC          */
/* ============ */

float get_ntc_temp(uint8_t ntc_pin)
{
    float adc_val = 0;
    for (uint8_t i = 0; i < NUM_OF_SAMPLES; i++)
    {
        adc_val = adc_val + analogRead(ntc_pin);
        delay(10);
    }
    adc_val = adc_val / NUM_OF_SAMPLES; // Avarage

    float R2 = (VOLTAGE_DIVIDER_R1 * adc_val) / (1023.0 - adc_val); //convert the average analog value to resistance value
    float T2 = 1 / ((1 / ABS_ZERO_KELVIN) - ((log10(R1 / R2) / log10(NAT_LOG_e)) / B_VALUE));
    float T = T2 - ABS_ZERO_FARENHITE;

    return T;
}

void ntc_read()
{
    ntc1 = get_ntc_temp(NTC1_PIN); // Read temperature
    ntc2 = get_ntc_temp(NTC2_PIN); // Read temperature
    // ntc3 = get_ntc_temp(NTC3_PIN); // Read temperature

    sns_data_g.sns_ntc1 = (int8_t)ntc1;
    sns_data_g.sns_ntc2 = (int8_t)ntc2;
    // sns_data_g.sns_ntc3 = (int8_t)ntc3;

#ifdef PRINT_FUNC_DEBUG
    print_raw_sensor_ntc_data();
#endif
}

void print_raw_sensor_ntc_data()
{
    PRINTLN("Func: print_raw_sensor_ntc_data()");

    PRINT("ntc1:\t\t");
    PRINTLN(ntc1);
    PRINT("ntc2:\t\t");
    PRINTLN(ntc2);
    // PRINT("ntc3:\t\t");
    // PRINTLN(ntc3);
}

#endif