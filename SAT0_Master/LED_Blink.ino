/**
    @file LED_Blink.ino
    @brief LED Blink functions for SATLLA0.

    This file contains functionality to blink LEDs on several frequency for the SATLLA0.

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

#if MAINLED_ENABLE
IntervalTimer mainLEDBlinkTimer;

bool mainLEDStatus = false;
bool first_time = true;
bool terminate_function = false;

uint32_t blink_counter_led = 0;
uint8_t blink_sequence_led = 0;
uint32_t blink_delay_led = 0;
uint32_t blink_delay_start_led = 0;

/* ============ */
/* Setup        */
/* ============ */

void LED_setup()
{
    PRINTLN("Func: LED_setup()");
    pinMode(FAT_MAIN_LED_PIN, OUTPUT);
    digitalWrite(FAT_MAIN_LED_PIN, LOW);
}

/* ============== */
/* BlinkLED Loop  */
/* ============== */

void blink_LED_task()
{
    PRINTLN("Func: blink_LED_task()");
    // If should blink is on, then blink.
    if (should_blink_LED)
    {
        // Blink LED Functionality
        BlinkLED();
    }

    // If should blink with delay is on, then wait for blink.
    if (should_blink_LED_delay)
    {
        if (millis() - start_blink_delay > blink_delay_msec)
        {
            // Blink LED Functionality
            BlinkLED();
        }
    }
}

/* ============ */
/* BlinkLED     */
/* ============ */

void BlinkLED()
{
    PRINTLN("Func: BlinkLED()");
    if (first_time)
    {
        led_first_time_func();
    }

    if (blink_counter_led > cycleSec)
    {
        first_time = true;
        blink_counter_led = 0;
        blink_sequence_led++;
        if (blink_sequence_led > 3)
        {
            //blinqSequence = 0;
            PRINTLN("BlinkLED: Terminate Blink");
            terminate_function = true;
            first_time = false;
        }
        else
        {
            mainLEDBlinkTimer.end();
            //delay(TEN_MS);
            //mainLEDBlinkTimer.delete1();
            delay(TEN_MS);
        }

        if (first_time)
        {
            led_first_time_func();
        }
    }

    if (terminate_function)
    {
        led_terminate();
    }
}

void led_first_time_func()
{
    PRINTLN("Func: led_first_time_func()");
    cycleSec = CycleSecs[blink_sequence_led];
    didBlinked = true;

    PRINT("BlinkLED: Blink Params:\t");
    PRINT(TimePeriods[blink_sequence_led]);
    PRINT(PeriodUnits[blink_sequence_led]);
    PRINT(", ");
    PRINTLN(cycleSec);

    BlinkLEDPeriod(TimePeriods[blink_sequence_led], PeriodUnits[blink_sequence_led]);

    blink_counter_led = 0;
    first_time = false;

    mission_led_blink_flag = true;
}

void led_terminate()
{
    PRINTLN("Func: led_terminate()");
    mainLEDBlinkTimer.end();
    delay(TEN_MS);
    digitalWrite(FAT_MAIN_LED_PIN, 0);
    //delay(TEN_MS);
    //mainLEDBlinkTimer.delete1();
    //delay(TEN_MS);

    blink_sequence_led = 0;
    blink_counter_led = 0;
    first_time = true;
    didBlinked = false;
    should_blink_LED = false;

    // reset the parameters
    should_blink_LED_delay = false;
    start_blink_delay = 0;
    blink_delay_msec = 0;

    terminate_function = false;

    mission_led_blink_flag = false;
}

void BlinkLEDPeriod(uint16_t timePeriod, uint32_t unit)
{
    PRINTLN("Func: BlinkLEDPeriod()");
    int interval = timePeriod * unit;
    PRINT("BlinkLEDPeriod: Interval:\t");
    PRINTLN(interval);
    //Begin calling the function. The interval is specified in microseconds, which may be an integer or floating point number,
    mainLEDBlinkTimer.begin(BlinkLEDTimerFunction, interval);
    //mainLEDBlinkTimer.start();
}

void BlinkLEDTimerFunction()
{
    // PRINTLN("Func: BlinkLEDTimerFunction()");
    blink_counter_led++;
    mainLEDStatus = !mainLEDStatus;
    digitalWrite(FAT_MAIN_LED_PIN, mainLEDStatus);
}

#endif