/**
    @file WDT.ino
    @brief Watchdog utils functions for SATLLA0.

    This file contains Teansy 4.x watchdog for the SATLLA0.

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

#if WDT_ENABLE

/* ============ */
/* Setup        */
/* ============ */

#include "Watchdog_t4.h"
WDT_T4<WDT1> wdt;

void wd_setup()
{
    PRINTLN("Func: wd_setup()");

    WDT_timings_t config;
    config.window = 3;   /* in seconds, 32ms to 522.232s, must be smaller than timeout */
    config.timeout = 60; /* in seconds, 0->128 */
    wdt.begin(config);
}

/* ============== */
/* Is Alive       */
/* ============== */

void wd_heartbeat()
{
    // PRINTLN("Func: wd_heartbeat()");

    wdt.feed();
}

#endif