/**
    @file Energy.ino
    @brief Energy utils function for SATLLA0.

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

#if BBSTR_BQ27441_ENABLE

#include <SparkFunBQ27441.h>

// Set BATTERY_CAPACITY to the design capacity of your battery.
const unsigned int BATTERY_CAPACITY = 1200; // e.g. 850mAh battery

unsigned int soc = 0;           // Read state-of-charge (%)
unsigned int volts = 0;         // Read battery voltage (mV)
int current_avg = 0;            // Read average current (mA)
int current_stby = 0;           // Read average current (mA)
int current_max = 0;            // Read average current (mA)
unsigned int full_capacity = 0; // Read full capacity (mAh)
unsigned int capacity = 0;      // Read remaining capacity (mAh)
int power = 0;                  // Read average power draw (mW)
int health = 0;                 // Read state-of-health (%)

void setup_BQ27441()
{
    PRINTLN("Func: setup_BQ27441()");
    // Use lipo.begin() to initialize the BQ27441-G1A and confirm that it's
    // connected and communicating.
    if (!lipo.begin()) // begin() will return true if communication is successful
    {
        // If communication fails, print an error message and loop forever.
        PRINTLN("BQ27441: Init Failed!.");
    }
    else
    {
        PRINTLN("BQ27441: Setup Succeed.");
        // Uset lipo.setCapacity(BATTERY_CAPACITY) to set the design capacity
        // of your battery.
        lipo.setCapacity(BATTERY_CAPACITY);
    }
}

void battery_read()
{
    PRINTLN("Func: battery_read()");
    // Read battery stats from the BQ27441-G1A
    soc = lipo.soc();                    // Read state-of-charge (%)
    volts = lipo.voltage();              // Read battery voltage (mV)
    current_avg = lipo.current(AVG);     // Read average current (mA)
    current_stby = lipo.current(STBY);   // Read average current (mA)
    current_max = lipo.current(MAX);     // Read average current (mA)
    full_capacity = lipo.capacity(FULL); // Read full capacity (mAh)
    capacity = lipo.capacity(REMAIN);    // Read remaining capacity (mAh)
    power = lipo.power();                // Read average power draw (mW)
    health = lipo.soh();                 // Read state-of-health (%)

    battery_data_g.battery_soc = (int8_t)(soc);
    if (battery_data_g.battery_soc > 100)
    {
        battery_data_g.battery_soc = 0;
    }
    battery_data_g.battery_health = (int8_t)health;
    battery_data_g.battery_volts = (int16_t)volts;

    if (battery_data_g.battery_volts > 6000)
    {
        battery_data_g.battery_volts = 0;
    }
    battery_data_g.battery_current = (int16_t)current_avg;

    battery_data_g.battery_full_capacity = (uint16_t)full_capacity;
    if (battery_data_g.battery_full_capacity > BATTERY_CAPACITY)
    {
        battery_data_g.battery_full_capacity = 0;
    }
    battery_data_g.battery_capacity = (uint16_t)capacity;
    if (battery_data_g.battery_capacity > BATTERY_CAPACITY)
    {
        battery_data_g.battery_capacity = 0;
    }
    battery_data_g.battery_power = (int16_t)power;

#ifdef PRINT_FUNC_DEBUG
    print_battery_stats(&battery_data_g);
    PRINT("current_avg:\t");
    PRINTLN(current_avg);
    PRINT("current_stby:\t");
    PRINTLN(current_stby);
    PRINT("current_max:\t");
    PRINTLN(current_max);
    // print_energy_raw_data();
#endif
}

void check_battery_status()
{
    PRINTLN("Func: check_battery_status()");
    int volts = battery_data_g.battery_volts;
    if (volts > 6000 || volts < 0)
    {
        volts = 0;
    }
    else
    {
        if (volts > 0 && volts <= LOW_POWER_CRITERIA) // 1 - 3400
        {
            sat_state = sat_panic_power;
            PRINTLN("SAT state: PANIC");
        }
        else if (volts > LOW_POWER_CRITERIA + 100 && volts <= HIGH_POWER_CRITERIA) // 3500 - 3900
        {
            sat_state = sat_med_power;
            PRINTLN("SAT state: MED");
        }
        else if (volts > HIGH_POWER_CRITERIA + 50) // 3950
        {
            sat_state = sat_high_power;
            PRINTLN("SAT state: HIGH");
        }
    }
}
#endif
