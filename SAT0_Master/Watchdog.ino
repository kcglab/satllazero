/**
    @file Watchdog.ino
    @brief Watchdog utils functions for SATLLA0.

    This file contains MAX6369 watchdog functions for the SATLLA0.

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

/* 
    The MAX6369–MAX6374 are pin-selectable watchdog timers that supervise microprocessor (uP)  activity and signal when a system is operating improperly. During normal operation, the microprocessor should repeatedly toggle the watchdog input (WDI) before the selected watchdog timeout period elapses to demonstrate that the system is processing code properly. If the uP does not provide a valid watchdog input transition before the timeout period expires, the supervisor asserts a watchdog (WDO) output to signal that the system is not executing the desired instructions within the expected time frame.
*/
#if WD_MAX6369_ENABLE
/* ============ */
/* Setup        */
/* ============ */

bool wd_pulse = true;

void wd_setup()
{
    PRINTLN("Func: wd_setup()");
    pinMode(WDI_PIN, OUTPUT);

    wd_heartbeat();
}

/* ============== */
/* Is Alive       */
/* ============== */

void wd_heartbeat()
{
    // PRINTLN("Func: wd_heartbeat()");

    // During normal operation, the internal timer is cleared each time the
    // uP toggles the WDI with a valid logic transition 
    // (low to high or high to low) within the selected timeout period 
    wd_pulse = !wd_pulse;
    // PRINT("WD_PULSE: ");
    // PRINTLN(wd_pulse);
    digitalWrite(WDI_PIN, wd_pulse);
}

#endif