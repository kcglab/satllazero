/**
    @file EEPROM_Utils.ino
    @brief EEPROM utils functions for SATLLA0 GS.

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

#if EEPROM_ENABLE

#include <EEPROM.h>

#if (DEVCIE == BOARD_HELTEC)
#define EEPROM_SIZE 64
#else
#define EEPROM_SIZE 512
#endif

void eeprom_setup()
{
    PRINTLN("Func: eeprom_setup()");
    EEPROM.begin(EEPROM_SIZE);
}

uint16_t eeprom_clear()
{
    PRINTLN("Func: eeprom_clear()");
    // write a 0 to all 512 bytes of the EEPROM
    uint16_t i = 0;
    for (i = 0; i < EEPROM_SIZE; i++)
    {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    return i;
}

uint16_t eeprom_set(uint8_t *buffer, uint16_t buffer_size)
{
    PRINTLN("Func: eeprom_set()");
    // write the value to the appropriate byte of the EEPROM.
    // these values will remain there when the board is
    // turned off.
    uint16_t i = 0;
    for (i = 0; i < buffer_size; i++)
    {
        // PRINT(buffer[i], HEX);
        EEPROM.write(i, buffer[i]);
    }

    EEPROM.commit();
    return i;
}

uint16_t eeprom_read(uint8_t *buffer, uint16_t buffer_size)
{
    PRINTLN("Func: eeprom_read()");
    // read a byte from the current address of the EEPROM
    uint16_t i = 0;
    for (i = 0; i < buffer_size; i++)
    {
        buffer[i] = EEPROM.read(i);
    }
    return i;
}

/* ================= */
/* EEPROM CRC        */
/* ================= */

uint8_t calc_CRC8(const uint8_t *buffer, uint8_t len)
{
    PRINTLN("Func: calc_CRC8()");
    uint8_t crc = 0x00;
    while (len--)
    {
        uint8_t extract = *buffer++;
        for (uint8_t i = 8; i > 0; i--)
        {
            uint8_t sum = (crc ^ extract) & 0x01;
            crc >>= 1;
            if (sum)
            {
                crc ^= 0x8C;
            }
            extract >>= 1;
        }
    }
    return crc;
}

/* ================= */
/* Print EEPROM      */
/* ================= */
void print_eeprom()
{
    PRINTLN("Func: print_eeprom()");

    uint8_t value = 0;
    for (uint16_t i = 0; i < /*EEPROM_SIZE*/ GS_SETTING_PACKET_LENGTH; i++)
    {
        value = EEPROM.read(i);
        if (value < 16)
        {
            PRINT('0');
        }
        PRINT(value, HEX);
        if ((i + 1) % 16 == 0)
        {
            PRINTLN();
        }
        else
        {
            PRINT(" ");
        }
    }
    PRINTLN();
}

#endif