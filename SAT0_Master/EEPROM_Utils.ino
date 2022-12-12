/**
    @file EEPROM_Utils.ino
    @brief EEPROM utils functions for SATLLA0.

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

#define CRC_ADDR 0x02
const uint8_t eeprom_system_addr = 0; // 0: Init Boot Complete
const uint8_t eeprom_setting_addr = 10;

/* ================= */
/* EEPROM System Mem */
/* ================= */

void eeprom_system_clear()
{
    PRINTLN("Func: eeprom_system_clear()");
    for (int i = 0; i < eeprom_setting_addr; i++)
    {
        EEPROM.update(i, 0x00);
    }
}

void eeprom_system_update(uint16_t addr, uint8_t value)
{
    PRINTLN("Func: eeprom_system_update()");
    // PRINT("addr:\t");
    // PRINTLN(addr, HEX);
    // PRINT("value:\t");
    // PRINTLN(value, HEX);
    if (addr < eeprom_setting_addr)
    {
        EEPROM.update(addr, value);
    }
}

uint8_t eeprom_system_read(uint16_t addr)
{
    PRINTLN("Func: eeprom_system_read()");
    uint8_t value = 0x00;

    if (addr < eeprom_setting_addr)
    {
        value = EEPROM.read(addr);
        // PRINT("addr:\t");
        // PRINTLN(addr, HEX);
        // PRINT("value:\t");
        // PRINTLN(value, HEX);
    }

    return value;
}

/* ================= */
/* EEPROM Setting    */
/* ================= */
//EEPROM.length():	4096

void eeprom_clear_settings()
{
    PRINTLN("Func: eeprom_clear_settings()");
    PRINT("EEPROM.length():\t");
    PRINTLN(EEPROM.length());

    for (int i = 0; i < SETTING_PACKET_LENGTH; i++)
    {
        EEPROM.update(i + eeprom_setting_addr, 0x00);
    }
}

void eeprom_put_settings(setting_data_t *setting_data)
{
    PRINTLN("Func: eeprom_put_settings()");
    if (SETTING_PACKET_LENGTH + eeprom_setting_addr > EEPROM.length())
    {
        return;
    }

    for (int i = 0; i < SETTING_PACKET_LENGTH; i++)
    {
        EEPROM.update(i + eeprom_setting_addr, setting_data->packet[i]);
    }

    uint8_t crc_eeprom = calc_CRC8(setting_data->packet, SETTING_PACKET_LENGTH);
    PRINT("EEPROM CRC:\t");
    PRINTLN(crc_eeprom, HEX);
    eeprom_system_update(CRC_ADDR, crc_eeprom);
}

bool eeprom_get_settings(setting_data_t *setting_data)
{
    PRINTLN("Func: eeprom_get_settings()");
    bool ret_value = false;
    if (SETTING_PACKET_LENGTH + eeprom_setting_addr > EEPROM.length())
    {
        return ret_value;
    }

    EEPROM.get(eeprom_setting_addr, setting_data->packet);
#ifdef PRINT_FUNC_DEBUG
    print_eeprom(setting_data);
#endif
    uint8_t crc = calc_CRC8(setting_data->packet, SETTING_PACKET_LENGTH);
    PRINT("CRC:\t\t");
    PRINTLN(crc, HEX);
    uint8_t crc_eeprom = eeprom_system_read(CRC_ADDR);
    PRINT("EEPROM CRC:\t");
    PRINTLN(crc_eeprom, HEX);
    ret_value = (crc_eeprom != 0 && crc == crc_eeprom);

    return ret_value;
}

/* ================= */
/* EEPROM Setting    */
/* ================= */

// get global setting from eeprom
void get_global_settings()
{
    PRINTLN("Func: get_global_settings()");

    bool is_valid = eeprom_get_settings(&setting_data_g);

    // if correct then load to global vars, else save and restore to default
    if (is_valid)
    {
        load_global_settings();
    }
    else
    {
        save_global_settings();
    }
}

// load setting_data_g to global varuables
void load_global_settings()
{
    PRINTLN("Func: load_global_settings()");
    ts_loop_runtime = setting_data_g.set_ts_loop_runtime * SEC_1;
    save_to_log_threshold = setting_data_g.set_save_to_log_threshold * SEC_1;
    send_beacon_threshold = setting_data_g.set_send_beacon_threshold * SEC_1;
    sleep_delay_panic = setting_data_g.set_sleep_delay_panic * SEC_1;
    sleep_delay_non_panic = setting_data_g.set_sleep_delay_non_panic * SEC_1;
    delay_between_beacons = setting_data_g.set_delay_between_beacons * SEC_1;
    save_to_flash = setting_data_g.set_save_to_flash;
    size_last_meta = setting_data_g.set_size_last_meta;

    memcpy(key, setting_data_g.set_key, KEY_LENGTH);
}

// save setting_data_g to eeprom
void save_global_settings()
{
    PRINTLN("Func: save_global_settings()");

    setting_data_g.packet[SETTING_PACKET_LENGTH] = {0};

    setting_data_g.set_ts_loop_runtime = ts_loop_runtime / SEC_1;
    setting_data_g.set_save_to_log_threshold = save_to_log_threshold / SEC_1;
    setting_data_g.set_send_beacon_threshold = send_beacon_threshold / SEC_1;
    setting_data_g.set_sleep_delay_panic = sleep_delay_panic / SEC_1;
    setting_data_g.set_sleep_delay_non_panic = sleep_delay_non_panic / SEC_1;
    setting_data_g.set_delay_between_beacons = delay_between_beacons / SEC_1;
    setting_data_g.set_save_to_flash = save_to_flash;
    setting_data_g.set_size_last_meta = size_last_meta;
    
    memcpy(setting_data_g.set_key, key, KEY_LENGTH);

    // Save to eeprom
    eeprom_put_settings(&setting_data_g);
}

// reset setting_data_g and save to eeprom
void reset_global_setting()
{
    PRINTLN("Func: reset_global_setting()");

    eeprom_clear_settings();

    setting_data_g.packet[SETTING_PACKET_LENGTH] = {0};

    setting_data_g.set_ts_loop_runtime = TS_LOOP_RUNTIME / SEC_1;
    setting_data_g.set_save_to_log_threshold = LOG_THRESHOLD / SEC_1;
    setting_data_g.set_send_beacon_threshold = SEND_BEACON_THRESHOLD / SEC_1;
    setting_data_g.set_sleep_delay_panic = SECS_45 / SEC_1;
    setting_data_g.set_sleep_delay_non_panic = SECS_15 / SEC_1;;
    setting_data_g.set_delay_between_beacons = SEND_BEACON_THRESHOLD / SEC_1;
    setting_data_g.set_save_to_flash = FLASH_ON;
    setting_data_g.set_size_last_meta = CONST_10;
    memcpy(setting_data_g.set_key, KEY, KEY_LENGTH);
    
    // Save to eeprom
    eeprom_put_settings(&setting_data_g);
    // Load into global vars
    load_global_settings();
}

void set_global_settings(setting_data_t *setting_data)
{
    PRINTLN("Func: set_global_settings()");
    memcpy(&setting_data_g, setting_data->packet, SETTING_PACKET_LENGTH);

    // Save to eeprom
    eeprom_put_settings(&setting_data_g);
    // Load into global vars
    load_global_settings();
}

/* ================= */
/* Var Setting       */
/* ================= */

// save global var based on its index
void save_global_variable(uint8_t index, uint8_t value)
{
    PRINTLN("Func: save_global_variable()");

    PRINT("Index: ");
    PRINT(index);
    PRINT(", Value: ");
    PRINTLN(value);

    setting_data_g.packet[index] = value;

    // Save to eeprom
    eeprom_put_settings(&setting_data_g);
    // Load into global vars
    load_global_settings();
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

void print_eeprom(setting_data_t *setting_data)
{
    PRINTLN("Func: print_eeprom()");

    for (int i = 0; i < SETTING_PACKET_LENGTH; i++)
    {
        if (setting_data->packet[i] < 16)
        {
            PRINT('0');
        }
        PRINT(setting_data->packet[i], HEX);
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