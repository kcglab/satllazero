/**
    @file debug.h
    @brief Debug for SATLLA0.

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

// Comment the below macros before last flash to eliminate println

#define DEBUG
#define PRINT_FUNC_DEBUG

#ifndef DEBUG
#define PRINTLN(fmt, ...)
#define PRINT(fmt, ...)
void print_buffer(uint8_t *buffer, int buffer_len){}

#else

#define PRINTLN(fmt, ...)  Serial.println(fmt, ##__VA_ARGS__)
#define PRINT(fmt, ...)    Serial.print(fmt, ##__VA_ARGS__)

void print_buffer(uint8_t *buffer, int buffer_len)
{
    for (int i = 0; i < buffer_len; i++)
    {
        if (buffer[i] < 16)
        {
            Serial.print(0);
        }
        Serial.print(buffer[i], HEX);
        if (!((i + 1) % 16))
        {
            Serial.println();
        }
        else
        {
            Serial.print(" ");
        }
    }
    Serial.println();
}

#endif