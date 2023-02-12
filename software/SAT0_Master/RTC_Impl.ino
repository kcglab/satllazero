/**
    @file RTC_impl.ino
    @brief RTC set date and time for SATLLA0.

    This file contains functions to maintain RTC time for the SATLLA0.

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

tmElements_t tm;
const char *monthName[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void rtc_setup()
{
    if (getDate(__DATE__) && getTime(__TIME__))
    {
        PRINTLN("AVR Macro strings converted to tmElements.");
    }
    Teensy3Clock.set(makeTime(tm));
    setTime(makeTime(tm)); // set Ardino system clock to compiled time
    PRINTLN("System millis clock referenced to tmElements.");
}

time_t getTeensy3Time()
{
	return Teensy3Clock.get();
}

bool getTime(const char *str)
{
    int Hour, Min, Sec;

    if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3)
    {
        return false;
    }
    tm.Hour = Hour;
    tm.Minute = Min;
    tm.Second = Sec;
    return true;
}

bool getDate(const char *str)
{
    char Month[12];
    int Day, Year;
    uint8_t monthIndex;

    if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3)
    {
        return false;
    }
    for (monthIndex = 0; monthIndex < 12; monthIndex++)
    {
        if (strcmp(Month, monthName[monthIndex]) == 0)
            break;
    }
    if (monthIndex >= 12)
    {
        return false;
    }
    tm.Day = Day;
    tm.Month = monthIndex + 1;
    tm.Year = CalendarYrToTm(Year);
    return true;
}

void digitalClockDisplay()
{
    // digital clock display of the time
    PRINT(hour());
    printDigits(minute());
    printDigits(second());
    PRINT(" ");
    PRINT(month());
    PRINT('/');
    PRINT(day());
    PRINT('/');
    PRINT(year());
    PRINTLN();
}

void printDigits(int digits)
{
    // utility function for digital clock display: prints preceding colon and leading 0
    PRINT(":");
    if (digits < 10)
    {
        PRINT('0');
    }
    PRINT(digits);
}