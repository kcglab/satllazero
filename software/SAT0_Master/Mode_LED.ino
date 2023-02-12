/**
    @file Mode_LED.ino
    @brief Mode LED Blinking for SATLLA0.

    This file contains functionality to blink the mode LED for the SATLLA0.

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

#if MODELED_ENABLE

#include "TeensyThreads.h"

volatile bool run = true;
void modeled_setup()
{
    PRINTLN("Func: modeled_setup()");
    pinMode(MODE_LED_PIN, OUTPUT);

    start_modeled_thread();
}

void modeled_thread()
{
    while (run)
    {
        if (modeled_blink_count)
        {
            for (int i = 0; i < modeled_blink_count; i++)
            {
                digitalWrite(MODE_LED_PIN, HIGH);
                threads.delay(modeled_blink_period);
                digitalWrite(MODE_LED_PIN, LOW);
                threads.delay(modeled_blink_period);
            }
            modeled_blink_count = 0;
            modeled_blink_period = MODELED_SHORT;
        }
        threads.yield();
    }
}

void stop_modeled_thread()
{
    run = false;
}

void start_modeled_thread()
{
    run = true;
    threads.addThread(modeled_thread);
}

#endif