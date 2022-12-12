/**
    @file Laser_Blink.ino
    @brief Laser Blink functions for SATLLA0.

    This file contains functionality to blink Laser on several frequency for the SATLLA0.

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

#if LASER_ENABLE
IntervalTimer main_laser_blink_timer;

bool main_status_laser = false;
bool first_time_laser = true;
bool terminate_function_laser = false;

uint32_t blink_counter_laser = 0;
uint8_t blink_sequence_laser = 0;
uint32_t blink_delay_laser = 0;
uint32_t blink_delay_start_laser = 0;

/* ============ */
/* Setup        */
/* ============ */

void laser_setup()
{
    PRINTLN("Func: laser_setup()");
    pinMode(FAT_LASER_PIN, OUTPUT);
    digitalWrite(FAT_LASER_PIN, LOW);
}

/* ================ */
/* BlinkLASER Loop  */
/* ================ */

void blink_LASER_task()
{
    PRINTLN("Func: blink_LASER_task()");
    // If should blink is on, then blink.
    if (should_blink_laser)
    {
        // Blink LED Functionality
        BlinkLASER();
    }

    // If should blink with delay is on, then wait for blink.
    if (should_blink_laser_delay)
    {
        if (millis() - start_blink_laser_delay > blink_laser_delay_msec)
        {
            // Blink LED Functionality
            BlinkLASER();
        }
    }
}

/* ============ */
/* BlinkLASER     */
/* ============ */

void BlinkLASER()
{
    PRINTLN("Func: BlinkLASER()");
    if (first_time_laser)
    {
        laser_first_time_func();
    }

    if (blink_counter_laser >= cycle_sec_laser)
    {
        first_time_laser = true;
        blink_counter_laser = 0;
        blink_sequence_laser++;
        if (blink_sequence_laser > 3)
        {
            //blinqSequence = 0;
            PRINTLN("BlinkLASER: Terminate Blink");
            terminate_function_laser = true;
            first_time_laser = false;
        }
        else
        {
            main_laser_blink_timer.end();
            //delay(TEN_MS);
            //main_laser_blink_timer.delete1();
            delay(TEN_MS);
        }

        if (first_time_laser)
        {
            laser_first_time_func();
        }
    }

    if (terminate_function_laser)
    {
        laser_terminate();
    }
}

void laser_first_time_func()
{
    PRINTLN("Func: laser_first_time_func()");
    cycle_sec_laser = cycle_secs_laser[blink_sequence_laser];
    did_blinked_laser = true;

    PRINT("BlinkLASER: Blink Params:\t");
    PRINT(time_periods_laser[blink_sequence_laser]);
    PRINT(period_units_laser[blink_sequence_laser]);
    PRINT(", ");
    PRINTLN(cycle_sec_laser);

    BlinkLASERPeriod(time_periods_laser[blink_sequence_laser], period_units_laser[blink_sequence_laser]);

    blink_counter_laser = 0;
    first_time_laser = false;

    mission_laser_blink_flag = true;
}

void laser_terminate()
{
    PRINTLN("Func: laser_terminate()");
    main_laser_blink_timer.end();
    delay(TEN_MS);
    digitalWrite(FAT_LASER_PIN, 0);
    //delay(TEN_MS);
    //main_laser_blink_timer.delete1();
    //delay(TEN_MS);

    blink_sequence_laser = 0;
    blink_counter_laser = 0;
    first_time_laser = true;
    did_blinked_laser = false;
    should_blink_laser = false;

    // reset the parameters
    should_blink_laser_delay = false;
    start_blink_laser_delay = 0;
    blink_laser_delay_msec = 0;

    terminate_function_laser = false;

    mission_laser_blink_flag = false;

    delay(TEN_MS);
}

void BlinkLASERPeriod(uint16_t timePeriod, uint32_t unit)
{
    PRINTLN("Func: BlinkLASERPeriod()");
    int interval = timePeriod * unit;
    PRINT("BlinkLASERPeriod: Interval:\t");
    PRINTLN(interval);
    //Begin calling the function. The interval is specified in microseconds, which may be an integer or floating point number,
    main_laser_blink_timer.begin(BlinkLASERTimerFunction, interval);
    //main_laser_blink_timer.start();
}

void BlinkLASERTimerFunction()
{
    //PRINTLN("Func: BlinkLASERTimerFunction()");
    blink_counter_laser++;
    main_status_laser = !main_status_laser;
    digitalWrite(FAT_LASER_PIN, main_status_laser);
}

#endif