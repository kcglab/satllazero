/**
    @file Settings_Utils.ino
    @brief Setting Utils function for SATLLA0 GS.

    This file contains the settings functionality for the SATLLA0 GS.

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

#define CRC_KEY 0xA0

void load_settings()
{
    PRINTLN("Func: load_settings()");
    uint16_t len = eeprom_read(gs_setting_data_g.packet, GS_SETTING_PACKET_LENGTH);
    uint8_t gs_crc = gs_setting_data_g.set_gs_crc;
    uint8_t crc_eeprom = calc_CRC8(gs_setting_data_g.packet + 1, GS_SETTING_PACKET_LENGTH - 1);

    if (crc_eeprom != gs_crc)
    {
        reset_settings();
    }
    else
    {
        local_address = gs_setting_data_g.set_local_address;
        gs_gps_enable = gs_setting_data_g.set_gs_gps_enable;
        gs_drive_enable = gs_setting_data_g.set_gs_drive_enable;
        doppler_enable = gs_setting_data_g.set_doppler_enable;
        gs_lat = gs_setting_data_g.set_gs_lat;
        gs_lng = gs_setting_data_g.set_gs_lon;
    }
}

// Reset the setting to default
uint16_t reset_settings()
{
    PRINTLN("Func: reset_settings()");
    gs_setting_data_g.set_gs_lat = MYLOC_LAT;
    gs_setting_data_g.set_gs_lon = MYLOC_LNG;
    gs_setting_data_g.set_local_address = LOCAL_ADDRESS;
    gs_setting_data_g.set_gs_gps_enable = GPS_ENABLE;
    gs_setting_data_g.set_gs_drive_enable = DRIVE_ENABLE;
    gs_setting_data_g.set_doppler_enable = 1;
    // calc crc w/out current crc. crc always first.
    gs_setting_data_g.set_gs_crc = calc_CRC8(gs_setting_data_g.packet + 1, GS_SETTING_PACKET_LENGTH - 1);
    ;

    // eeprom_clear();
    uint16_t len = eeprom_set(gs_setting_data_g.packet, GS_SETTING_PACKET_LENGTH);
    print_eeprom();
    print_settings();
    return len;
}

// Save setting packet
uint16_t save_settings(gs_setting_data_t *gs_setting_data)
{
    PRINTLN("Func: save_settings()");

    memcpy(gs_setting_data_g.packet, gs_setting_data->packet, GS_SETTING_PACKET_LENGTH);
    // calc crc w/out current crc. crc always first.
    uint8_t crc_eeprom = calc_CRC8(gs_setting_data_g.packet + 1, GS_SETTING_PACKET_LENGTH - 1);
    gs_setting_data_g.set_gs_crc = crc_eeprom;

    uint16_t len = eeprom_set(gs_setting_data_g.packet, GS_SETTING_PACKET_LENGTH);
    print_settings();

    return len;
}

uint8_t update_gs_loc(float lat, float lng)
{
    PRINTLN("Func: update_gs_loc()");
    bool flag = eeprom_read(gs_setting_data_g.packet, GS_SETTING_PACKET_LENGTH);
    if (!flag)
    {
        reset_settings();
    }

    // Read the current setting and update the relevants.
    gs_setting_data_g.set_gs_lat = lat;
    gs_setting_data_g.set_gs_lon = lng;

    // calc crc w/out current crc. crc always first.
    uint8_t crc_eeprom = calc_CRC8(gs_setting_data_g.packet + 1, GS_SETTING_PACKET_LENGTH - 1);
    gs_setting_data_g.set_gs_crc = crc_eeprom;

    uint8_t error = eeprom_set(gs_setting_data_g.packet, GS_SETTING_PACKET_LENGTH);
    print_settings();

    return error;
}

uint8_t update_gs_address(uint8_t address) //, uint8_t destination)
{
    PRINTLN("Func: update_gs_address()");
    bool flag = eeprom_read(gs_setting_data_g.packet, GS_SETTING_PACKET_LENGTH);
    if (!flag)
    {
        reset_settings();
    }

    // Read the current setting and update the relevants.
    gs_setting_data_g.set_local_address = address;

    // calc crc w/out current crc. crc always first.
    uint8_t crc_eeprom = calc_CRC8(gs_setting_data_g.packet + 1, GS_SETTING_PACKET_LENGTH - 1);
    gs_setting_data_g.set_gs_crc = crc_eeprom;

    uint8_t error = eeprom_set(gs_setting_data_g.packet, GS_SETTING_PACKET_LENGTH);
    print_settings();

    return error;
}

uint8_t update_gps_enable(uint8_t enable)
{
    PRINTLN("Func: update_gps_enable()");
    bool flag = eeprom_read(gs_setting_data_g.packet, GS_SETTING_PACKET_LENGTH);
    if (!flag)
    {
        reset_settings();
    }

    // Read the current setting and update the relevants.
    gs_setting_data_g.set_gs_gps_enable = enable;

    // calc crc w/out current crc. crc always first.
    uint8_t crc_eeprom = calc_CRC8(gs_setting_data_g.packet + 1, GS_SETTING_PACKET_LENGTH - 1);
    gs_setting_data_g.set_gs_crc = crc_eeprom;

    uint8_t error = eeprom_set(gs_setting_data_g.packet, GS_SETTING_PACKET_LENGTH);
    print_settings();

    return error;
}

uint8_t update_drive_enable(uint8_t enable)
{
    PRINTLN("Func: update_drive_enable()");
    bool flag = eeprom_read(gs_setting_data_g.packet, GS_SETTING_PACKET_LENGTH);
    if (!flag)
    {
        reset_settings();
    }

    // Read the current setting and update the relevants.
    gs_setting_data_g.set_gs_drive_enable = enable;

    // calc crc w/out current crc. crc always first.
    uint8_t crc_eeprom = calc_CRC8(gs_setting_data_g.packet + 1, GS_SETTING_PACKET_LENGTH - 1);
    gs_setting_data_g.set_gs_crc = crc_eeprom;

    uint8_t error = eeprom_set(gs_setting_data_g.packet, GS_SETTING_PACKET_LENGTH);
    print_settings();

    return error;
}

uint8_t update_doppler_enable(uint8_t enable)
{
    PRINTLN("Func: update_drive_enable()");
    bool flag = eeprom_read(gs_setting_data_g.packet, GS_SETTING_PACKET_LENGTH);
    if (!flag)
    {
        reset_settings();
    }

    // Read the current setting and update the relevants.
    gs_setting_data_g.set_doppler_enable = enable;

    // calc crc w/out current crc. crc always first.
    uint8_t crc_eeprom = calc_CRC8(gs_setting_data_g.packet + 1, GS_SETTING_PACKET_LENGTH - 1);
    gs_setting_data_g.set_gs_crc = crc_eeprom;

    uint8_t error = eeprom_set(gs_setting_data_g.packet, GS_SETTING_PACKET_LENGTH);
    print_settings();

    return error;
}

uint16_t str_settings(char *settings)
{
    PRINTLN("Func: str_settings()");
    uint16_t length = sprintf(settings, "Local Address: %x\nGPS: %x\nDrive: %x\nDoppler: %x\nLat: %.6f\nLon: %.6f",
                              gs_setting_data_g.set_local_address,
                              gs_setting_data_g.set_gs_gps_enable,
                              gs_setting_data_g.set_gs_drive_enable,
                              gs_setting_data_g.set_doppler_enable,
                              gs_setting_data_g.set_gs_lat,
                              gs_setting_data_g.set_gs_lon);
    return length;
}

void parse_setting(char opt, char *buffer, uint16_t buffer_len)
{
    PRINTLN("Func: parse_setting()");
    PRINT("opt:\t");
    PRINTLN(opt);
    // PRINTLN(buffer_len);
    // if (buffer_len < 2)
    // {
    //     return;
    // }

    uint8_t val = 0;
    uint8_t idx = 0;

    if (opt == 'u')
    {
#if RF_433_ENABLE
        modem_lora_433_g.modem_bw = buffer[0];
        modem_lora_433_g.modem_sf = buffer[1];
        modem_lora_433_g.modem_cr = buffer[2];
        modem_lora_433_g.modem_power = buffer[3];
        setup_lora_433(&modem_lora_433_g);
        change_radio_lora_433_setting_timer = millis();
#endif
    }
    else if (opt == 's')
    {
#if RF_24_ENABLE
        modem_lora_24_g.modem_bw = buffer[0];
        modem_lora_24_g.modem_sf = buffer[1];
        modem_lora_24_g.modem_cr = buffer[2];
        modem_lora_24_g.modem_power = buffer[3];
        setup_lora_24(&modem_lora_24_g);
        change_radio_lora_24_setting_timer = millis();
#endif
    }
    else if (opt == 'l')
    {
        PRINTLN("Set GS Location");

        b2f lat;
        b2f lng;
        for (size_t i = 0; i < 4; i++)
        {
            lat.b[i] = buffer[3 - i];
            lng.b[i] = buffer[7 - i];
        }
        PRINTLN(lat.fval, 6);
        PRINTLN(lng.fval, 6);
        set_gs_location(lat.fval, lng.fval);
    }
    else if (opt == 'a')
    {
        PRINTLN("Set GS Address");
        local_address = buffer[0];
#if EEPROM_ENABLE
        update_gs_address(local_address); //, destination);
#endif
    }
    else if (opt == 'r')
    {
        PRINTLN("Reset Settings");
#if EEPROM_ENABLE
        reset_settings();
#endif
    }
    else if (opt == 'g')
    {
        PRINTLN("update gps enable");
#if EEPROM_ENABLE
        uint8_t enable = buffer[0];
        update_gps_enable(enable);
#endif
    }
    else if (opt == 'd')
    {
        PRINTLN("update drive enable");
#if EEPROM_ENABLE
        uint8_t enable = buffer[0];
        update_drive_enable(enable);
#endif
    }
    else if (opt == 'p')
    {
        PRINTLN("update doppler enable");
#if EEPROM_ENABLE
        uint8_t enable = buffer[0];
        update_doppler_enable(enable);
#endif
    }
    else if (opt == 'e')
    {
        PRINTLN("Get Settings");
        // #if EEPROM_ENABLE
        //         uint16_t length = str_settings(buffer);
        //         String message += "\n" + String(buffer);
        // #endif
    }
    else
    {
        return;
    }

    print_settings();
#if EEPROM_ENABLE
    print_eeprom();
#endif
}

void print_settings()
{
    PRINTLN("Func: print_settings()");

    PRINT("gs_setting_data_g.set_gs_crc:\t");
    PRINTLN(gs_setting_data_g.set_gs_crc, HEX);

    PRINT("gs_setting_data_g.set_local_address:\t");
    PRINTLN(gs_setting_data_g.set_local_address, HEX);

    PRINT("gs_setting_data_g.set_gs_gps_enable:\t");
    PRINTLN(gs_setting_data_g.set_gs_gps_enable);
    PRINT("gs_setting_data_g.set_gs_drive_enable:\t");
    PRINTLN(gs_setting_data_g.set_gs_drive_enable);
    PRINT("gs_setting_data_g.set_doppler_enable:\t");
    PRINTLN(gs_setting_data_g.set_doppler_enable);

    PRINT("gs_setting_data_g.set_gs_lat:\t");
    PRINTLN(gs_setting_data_g.set_gs_lat, 6);
    PRINT("gs_setting_data_g.set_gs_lon:\t");
    PRINTLN(gs_setting_data_g.set_gs_lon, 6);

    PRINTLN();
}

#endif