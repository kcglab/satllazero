/**
    @file Telegram_Impl.ino
    @brief Telegram utils file function for SATLLA0 GS.

    Telegram implementation to send and receive messages from Telegram bot.

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


#if TELEGRAM_ENABLE && WIFI_ENABLE

#include <HTTPClient.h>
#include "CTBot.h"

CTBot myBot;

// Your Domain name with URL path or IP address with path
String webhook_url = "https://maker.ifttt.com/trigger/message_received/with/key/ctvRPL-fUvk81l3DiUEwu0";

#define CMD_NUM 16
char commands[CMD_NUM][11] = {"nextsat", "satlla", "norby", "modem1", "modem2", "pub2chn", "predict", "station", "restart", "test", "gs_loc", "firmware", "version", "tle_fb", "batt", "tle"};
lorafreq_e freq;

/* =============== */
/* Setup           */
/* =============== */

void tlg_setup()
{
    // connect the ESP8266 to the desired access point
    // myBot.wifiConnect(WFSSID2, WFPASS2);

    // set the telegram bot token
    myBot.setTelegramToken(token);

    // check if all things are ok
    if (myBot.testConnection())
    {
        Serial.println("\ntestConnection OK");
    }
    else
    {
        Serial.println("\ntestConnection NOK");
    }

    randomSeed(analogRead(0));
}

/* =============== */
/* Read            */
/* =============== */

void tlg_read()
{
    // PRINTLN("Func: tlg_read()");
    wifi_connect();

    // a variable to store telegram message data
    TBMessage msg;
    // check if there is a new incoming message
    if (myBot.getNewMessage(msg))
    {
        // check if the message is a text message
        if (msg.messageType == CTBotMessageText)
        {
            Serial.println(
                "\nFirstName: " + msg.sender.firstName +
                "\nID: " + msg.sender.id +
                "\nMessage: " + msg.text +
                "\nLength: " + msg.text.length());

            uint8_t message_length = msg.text.length();

            if (message_length > 0)
            {
                msg.text = msg.text.substring(1, message_length);

                int16_t idx = msg.text.indexOf('@');
                // PRINTLN(idx);
                uint8_t address = 0;
                // Is this message is for a specific station?
                if (idx > 0)
                {
                    String hex = msg.text.substring(idx + 1, idx + 3);
                    address = (uint8_t)strtol(hex.c_str(), NULL, 16);
                    if (address != local_address)
                    {
                        PRINTLN("TLG: This message is not for me.");
                        return;
                    }
                    msg.text = msg.text.substring(0, idx); // remove the address
                }

                /* ======================== */
                if (msg.text == commands[0]) // nextsat
                {
#if TLE_ENABLE
                    publish_sat_to_tlg(-1, msg.sender.id);
#endif
                }
                else if (msg.text == commands[1]) // satlla
                {
#if TLE_ENABLE
                    publish_sat_to_tlg(0, msg.sender.id);
#endif
                }
                else if (msg.text == commands[2]) // norbi
                {
#if TLE_ENABLE
                    publish_sat_to_tlg(1, msg.sender.id);
#endif
                }
                // else if (msg.text == commands[3]) // modem1
                // {
                //     switch_modem(0);
                //     switch_modem_override = true;
                //     command_received_message(msg.sender.id, msg.text);
                // }
                // else if (msg.text == commands[4]) // modem2
                // {
                //     switch_modem(1);
                //     switch_modem_override = true;
                //     command_received_message(msg.sender.id, msg.text);
                // }
                else if (msg.text == commands[5] && idx > 0) // pub2chn
                {
                    tlg_ch_enable = !tlg_ch_enable;
                    Serial.print("Publish to Channel:\t");
                    Serial.println(tlg_ch_enable);
                    command_received_message(msg.sender.id, msg.text);
                }
                else if (msg.text == commands[6]) // predict
                {
// Get TLEs
#if TLE_ENABLE
                    update_TLE();
                    switch_next_sat();
#endif
                    command_received_message(msg.sender.id, msg.text);
                }
                else if (msg.text == commands[7]) // station
                {
                    String text = "Station " + String(local_address, HEX) + " is alive.";
                    publish_to_tlg_message(msg.sender.id, text, 0);
                }
                else if (msg.text == commands[8] && idx > 0) // restart
                {
                    restart_requested_flag = millis();
                    command_received_message(msg.sender.id, msg.text);
                }
                else if (msg.text == commands[9]) // test
                {
#if TLE_ENABLE
                    sat_current_info sat_current;
                    get_current(-1, &sat_current);
                    print_current(&sat_current);
                    get_pass(-1, &sat_current, 0);
                    print_current(&sat_current);
                    get_pass(-1, &sat_current, 1);
                    print_current(&sat_current);
                    get_pass(-1, &sat_current, 2);
                    print_current(&sat_current);
#endif
                }
                else if (msg.text == commands[10]) // gs_location
                {
                    String text = msg.text + "\nLat/Lng: " + String(gs_lng, 6) + "," + String(gs_lat, 6);
                    command_received_message(msg.sender.id, text);
                }
                else if (msg.text == commands[11] && idx > 0) // firmware
                {
                    String text = msg.text;
#if FIREBASE_ENABLE
                    text = text + "\nFirmware: " + String(firmware_version);
                    update_firmware();
#endif
                    command_received_message(msg.sender.id, text);
                }
                else if (msg.text == commands[12]) // version
                {
                    String text = msg.text + "\nFirmware Version: " + String(firmware_version);
                    command_received_message(msg.sender.id, text);
                }
                else if (msg.text == commands[13]) // tle_fb
                {
#if FIREBASE_ENABLE && TLE_ENABLE
                    update_TLE_from_TLG();
                    switch_next_sat();
#endif
                    command_received_message(msg.sender.id, msg.text);
                }
                else if (msg.text == commands[14]) // batt axp20
                {
                    String text = msg.text;
#if AXP20_ENABLE
                    axp20_status();
                    axp20_raw_print();
                    text = text + "\nBattery Volt: " + String((float)(battery_data_g.battery_volts) / 10) + "\nBattery Current: " + String((float)(battery_data_g.battery_current) / 10);
#endif
                    command_received_message(msg.sender.id, text);
                }
                else if (msg.text == commands[15]) // TLE
                {
                    String text = msg.text;
#if TLE_ENABLE
                    get_tle_for_sat(0, buffer_g);
                    char tle1[71] = {0x00};
                    memcpy(tle1, buffer_g, 70);
                    char tle2[71] = {0x00};
                    memcpy(tle2, buffer_g + 70, 70);

                    text = text + "\nTLE:\n" + String(tle1) + "\n" + String(tle2);
#endif
                    command_received_message(msg.sender.id, text);
                }

                /* ======================== */
                else if (msg.text.c_str()[0] == 'r' && idx > 0) // receive
                {
                    freq = lora_none;
                    parse_message(msg.sender.id, msg.text, freq);
                }
                else if (msg.text.c_str()[0] == 's' && idx > 0) // send 24
                {
                    freq = lora_24;
                    parse_message(msg.sender.id, msg.text, freq);
                }
                else if (msg.text.c_str()[0] == 'u' && idx > 0) // send uhf
                {
                    freq = lora_433;
                    parse_message(msg.sender.id, msg.text, freq);
                }
                else if (msg.text.c_str()[0] == 'p' && idx > 0) // pending uhf command
                {
                    freq = lora_433;
                    parse_message(msg.sender.id, msg.text, freq, true);
                }
                else if (msg.text.c_str()[0] == 'g' && idx > 0) // send tinyGS
                {
                    freq = lora_gs;
                    parse_message(msg.sender.id, msg.text, freq);
                }
                else if (msg.text.c_str()[0] == 'l' && idx > 0) // pending s-band command
                {
                    freq = lora_24;
                    parse_message(msg.sender.id, msg.text, freq, true);
                }
                else if (msg.text.c_str()[0] == 't' && idx > 0) // setting only per station. this required idx > 0
                {
                    // setting
                    parse_setting(msg.sender.id, msg.text);
                }
            }
        }
    }
}

/* =============== */
/* Parse           */
/* =============== */

void parse_message(int64_t target_id, String message)
{
    return parse_message(target_id, message, lora_none, false);
}

void parse_message(int64_t target_id, String message, lorafreq_e freq)
{
    return parse_message(target_id, message, freq, false);
}

void parse_message(int64_t target_id, String message, lorafreq_e freq, bool pending_flag)
{
    PRINTLN("Func: parse_message()");
    // PRINTLN(message);
    uint16_t message_length = message.length();
    if (message_length < 2)
    {
        return;
    }

    uint8_t val = 0;
    uint8_t idx = 0;
    String hex;

    for (int i = 1; i < message_length; i += 2) // avoid eol
    {
        hex = message.substring(i, i + 2);
        // PRINTLN(hex);
        val = (int)strtol(hex.c_str(), NULL, 16);
        buffer_g[idx++] = val;
    }

    memset(command_data_g.packet, 0x00, COMMAND_PACKET_LENGTH);
    destination_g = buffer_g[0];
    command_data_g.cmd_code = buffer_g[1];
    command_data_g.cmd_type = cmd_type_param;
    command_data_g.cmd_size = idx - 2;
    memcpy(command_data_g.cmd_payload, buffer_g + 2, idx - 2);
    // print_command(&command_data_g);

    command_pending_flag = false; // shutdown pending on any other command.
    if (freq < lora_none)
    {
        if (pending_flag)
        {
            command_pending_flag = true;
        }
        else
        {
            cmd_send(&command_data_g, destination_g, freq);
        }
    }
    else
    {
        cmd_receive(&command_data_g, destination_g);
    }

    command_received_message(target_id, message);
}

void parse_setting(int64_t target_id, String message)
{
    PRINTLN("Func: parse_setting()");
    // PRINTLN(message);
    uint16_t message_length = message.length();
    if (message_length < 2)
    {
        return;
    }

    uint8_t val = 0;
    uint8_t idx = 0;
    String hex;

    for (int i = 2; i < message_length; i += 2) // avoid eol
    {
        hex = message.substring(i, i + 2);
        // PRINTLN(hex);
        val = (int)strtol(hex.c_str(), NULL, 16);
        buffer_g[idx++] = val;
    }

    if (message.c_str()[1] == 'u')
    {
// set_lora_433(int bw, int sf, int cr, int pm)
#if RF_433_ENABLE
        modem_lora_433_g.modem_bw = buffer_g[0];
        modem_lora_433_g.modem_sf = buffer_g[1];
        modem_lora_433_g.modem_cr = buffer_g[2];
        modem_lora_433_g.modem_power = buffer_g[3];
        setup_lora_433(&modem_lora_433_g);
        change_radio_lora_433_setting_timer = millis();
#endif
    }
    else if (message.c_str()[1] == 's')
    {
#if RF_24_ENABLE
        modem_lora_24_g.modem_bw = buffer_g[0];
        modem_lora_24_g.modem_sf = buffer_g[1];
        modem_lora_24_g.modem_cr = buffer_g[2];
        modem_lora_24_g.modem_power = buffer_g[3];
        setup_lora_24(&modem_lora_24_g);
        change_radio_lora_24_setting_timer = millis();
#endif
    }
    else if (message.c_str()[1] == 'l')
    {
        PRINTLN("Set GS Location");

        b2f lat;
        b2f lng;
        for (size_t i = 0; i < 4; i++)
        {
            lat.b[i] = buffer_g[3 - i];
            lng.b[i] = buffer_g[7 - i];
        }
        PRINTLN(lat.fval, 6);
        PRINTLN(lng.fval, 6);
        set_gs_location(lat.fval, lng.fval);
    }
    else if (message.c_str()[1] == 'a')
    {
        PRINTLN("Set GS Address");
        local_address = buffer_g[0];
#if EEPROM_ENABLE
        update_gs_address(local_address); //, destination);
#endif
    }
    else if (message.c_str()[1] == 'r')
    {
        PRINTLN("Reset Settings");
#if EEPROM_ENABLE
        reset_settings();
#endif
    }
    else if (message.c_str()[1] == 'g')
    {
        PRINTLN("update gps enable");
#if EEPROM_ENABLE
        uint8_t enable = buffer_g[0];
        update_gps_enable(enable);
#endif
    }
    else if (message.c_str()[1] == 'd')
    {
        PRINTLN("update drive enable");
#if EEPROM_ENABLE
        uint8_t enable = buffer_g[0];
        update_drive_enable(enable);
#endif
    }
    else if (message.c_str()[1] == 'p')
    {
        PRINTLN("update doppler enable");
#if EEPROM_ENABLE
        uint8_t enable = buffer_g[0];
        update_doppler_enable(enable);
#endif
    }
    else if (message.c_str()[1] == 'e')
    {
        PRINTLN("Get Settings");
#if EEPROM_ENABLE
        uint16_t length = str_settings(buffer_g);
        message += "\n" + String(buffer_g);
#endif
    }
    else
    {
        return;
    }

    print_settings();
#if EEPROM_ENABLE
    print_eeprom();
#endif
    command_received_message(target_id, message);
}

/* ================ */
/* Command Received */
/* ================ */

void command_received_message(int64_t target_id, String message)
{
    PRINTLN("Func: command_received_message()");
    String text = "Station " + String(local_address, HEX) + " executed command: " + message;
    PRINTLN(text);
    myBot.setParseMode(CTBotParseModeHTML);
    myBot.sendMessage(target_id, text);
    delay(500); // wait 500 milliseconds
}

/* =============== */
/* Publish Lng Bcn */
/* =============== */

void publish_to_tlg_long_becon(int64_t target_id, uint8_t *message, uint8_t message_length)
{
    PRINTLN("Func: publish_to_tlg_bot()");
    if (message_length <= 0)
    {
        return;
    }

    memset(prnt_msg_g.packet, 0, LORA_PACKET_LENGTH);
    memset(gps_data_g.packet, 0, GPS_PACKET_LENGTH);
    memset(sns_data_g.packet, 0, SNS_PACKET_LENGTH);
    memset(energy_data_g.packet, 0, ENERGY_PACKET_LENGTH);
    memset(sd_data_g.packet, 0, SD_DATA_LENGTH);
    memset(radio_data_g.packet, 0, RADIO_DATA_LENGTH);
    memset(info_data_g.packet, 0, INFO_DATA_LENGTH);
    memset(ld_meta_data_g.packet, 0, LD_META_DATA_LENGTH);

    memcpy(prnt_msg_g.packet, message, message_length);
    memcpy(gps_data_g.packet, prnt_msg_g.msg_payload, GPS_PACKET_LENGTH);
    memcpy(sns_data_g.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH, SNS_PACKET_LENGTH);
    memcpy(battery_data_g.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH + SNS_PACKET_LENGTH, BATTERY_PACKET_LENGTH);
    memcpy(sd_data_g.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH, SD_DATA_LENGTH);
    memcpy(info_data_g.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH + SD_DATA_LENGTH, INFO_DATA_LENGTH);

    uint8_t ld_size = prnt_msg_g.msg_size - (GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH + SD_DATA_LENGTH + INFO_DATA_LENGTH);
    if (ld_size > 0)
    {
        memcpy(ld_meta_data_g.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH + SD_DATA_LENGTH + INFO_DATA_LENGTH, LD_META_HEADER_LENGTH + ld_size);
    }

    uint8_t h = (prnt_msg_g.msg_time / 3600);
    uint8_t m = (prnt_msg_g.msg_time / 60) % 60;

    String text = "Station: " + String(local_address, HEX) +
                  ", rssi: " + String(rssi_g) +
                  ", snr: " + String(snr_g) +
                  ", doppler: " + String(doppler_g) +
                  ", rv: " + String(rv_g) +
                  "\n\U0001F4ED 0x" + String(prnt_msg_g.local_address, HEX) +
                  "\n\U000000A9 0x" + String(prnt_msg_g.msg_type, HEX) +
                  "\n\U0001F53A 0x" + String(prnt_msg_g.destination, HEX) +
                  "\n\U0001F194 " + String(prnt_msg_g.msg_index) +
                  "\n\U0001F4E6 " + String(prnt_msg_g.msg_size) +
                  "\nAck " + String(prnt_msg_g.msg_ack_req) +
                  "\n\U000023F0 " + String(h) + ":" + ((m < 10) ? "0" + String(m) : String(m));

    text += "\nGPS Time: " + String(gps_data_g.gps_date_time.gps_hour) + ":" + String(gps_data_g.gps_date_time.gps_min) +
            "\nGPS Date: " + String(gps_data_g.gps_date_time.gps_day) + ":" + String(gps_data_g.gps_date_time.gps_month) +
            "\nSats: " + String(gps_data_g.gps_sat) +
            "\nLat/Lng: " + String(gps_data_g.gps_lat, 6) + "," + String(gps_data_g.gps_lng, 6) +
            "\n\U0001F5FA <a href='https://www.google.com/maps/place/" + String(gps_data_g.gps_lat, 6) + "," + String(gps_data_g.gps_lng, 6) + "'>Google Maps</a>" +
            "\nAlt: " + String((float)(gps_data_g.gps_alt) / 1000) +
            "\nSpeed: " + String(gps_data_g.gps_speed) +
            "\nCourse: " + String(gps_data_g.gps_course);

    text += "\nsns_bmp_temp_c=" + String((float)(sns_data_g.sns_bmp_temp_c) / CONST_100) + " deg C" +
            "\nsns_gx=" + String((float)(sns_data_g.sns_gx) / CONST_100) + " radians/s" +
            "\nsns_gy=" + String((float)(sns_data_g.sns_gy) / CONST_100) + " radians/s" +
            "\nsns_gz=" + String((float)(sns_data_g.sns_gz) / CONST_100) + " radians/s" +
            "\nsns_mx=" + String((float)(sns_data_g.sns_mx) / CONST_100) + " uTesla" +
            "\nsns_my=" + String((float)(sns_data_g.sns_my) / CONST_100) + " uTesla" +
            "\nsns_mz=" + String((float)(sns_data_g.sns_mz) / CONST_100) + " uTesla" +
            "\nntc1=" + String(sns_data_g.sns_ntc1) + " deg C" +
            "\nntc2=" + String(sns_data_g.sns_ntc2) + " deg C";

    text += "\nbattery_soc=" + String(battery_data_g.battery_soc) +
            "\nbattery_volts=" + String(battery_data_g.battery_volts) +
            "\nbattery_current=" + String(battery_data_g.battery_current) +
            "\nbattery_capacity=" + String(battery_data_g.battery_capacity) +
            "\nbattery_full_capacity=" + String(battery_data_g.battery_full_capacity) +
            "\nbattery_power=" + String(battery_data_g.battery_power) +
            "\nbattery_health=" + String(battery_data_g.battery_health);

    text += "\noutbox=" + String(sd_data_g.sd_outbox) +
            "\nsent=" + String(sd_data_g.sd_sent) +
            "\nrpi=" + String(sd_data_g.sd_rpi) +
            "\nfiles=" + String(sd_data_g.sd_files);

    text += "\nreset_index=" + String(info_data_g.reset_index);

    if (ld_size > 0)
    {
        text += "\nld_type:=" + String(ld_meta_data_g.ld_type) +
                "\nld_size:=" + String(ld_meta_data_g.ld_size) +
                "\nPayload: " + message2hex(ld_meta_data_g.packet, ld_meta_data_g.ld_size);
    }

    PRINT("Text: ");
    PRINTLN(text);

    myBot.setParseMode(CTBotParseModeHTML);
    myBot.sendMessage(target_id, text);
    delay(500); // wait 500 milliseconds

    // post the message on google docs
    if (tlg_ch_enable)
    {
        myBot.sendMessage(satlla0_channel, text);
        delay(500); // wait 500 milliseconds
    }
}

/* =============== */
/* Publish Srt Bcn */
/* =============== */

void publish_to_tlg_short_becon(int64_t target_id, uint8_t *message, uint8_t message_length)
{
    PRINTLN("Func: publish_to_tlg_short_becon()");
    if (message_length <= 0)
    {
        return;
    }

    memcpy(prnt_short_beacon_g.packet, message, message_length);

    String text = "Station: " + String(local_address, HEX) +
                  ", rssi: " + String(rssi_g) +
                  ", snr: " + String(snr_g) +
                  ", doppler: " + String(doppler_g) +
                  ", rv: " + String(rv_g) +
                  "\n\U0001F4ED 0x" + String(prnt_short_beacon_g.local_address, HEX) +
                  "\n\U000000A9 0x" + String(prnt_short_beacon_g.msg_type, HEX) +
                  //   "\n\U0001F53A 0x" + String(prnt_short_beacon_g.destination, HEX) +
                  "\n\U0001F194 " + String(prnt_short_beacon_g.msg_index) +
                  "\nVolts: " + String(prnt_short_beacon_g.battery_volts) +
                  "\nCurren:  " + String(prnt_short_beacon_g.battery_current) +
                  "\nSD Out: " + String(prnt_short_beacon_g.sd_outbox) +
                  "\nMsg Received: " + String(prnt_short_beacon_g.msg_received) +
                  "\nNtc1: " + String(prnt_short_beacon_g.sns_ntc1) +
                  "\nNtc2: " + String(prnt_short_beacon_g.sns_ntc2);

    PRINT("Text: ");
    PRINTLN(text);

    myBot.setParseMode(CTBotParseModeMarkdown);
    myBot.sendMessage(target_id, text);
    delay(500); // wait 500 milliseconds

    if (tlg_ch_enable)
    {
        myBot.sendMessage(satlla0_channel, text);
        delay(500); // wait 500 milliseconds
    }
}

/* =============== */
/* Publish Message */
/* =============== */

void publish_to_tlg_other_message(int64_t target_id, uint8_t *message, uint8_t message_length)
{
    PRINTLN("Func: publish_to_tlg_other_message()");
    if (message_length <= 0)
    {
        return;
    }

    memset(prnt_msg_g.packet, 0, LORA_PACKET_LENGTH);
    memcpy(prnt_msg_g.packet, message, message_length);
    uint8_t h = (prnt_msg_g.msg_time / 3600);
    uint8_t m = (prnt_msg_g.msg_time / 60) % 60;

    String text = "Station: " + String(local_address, HEX) +
                  ", rssi: " + String(rssi_g) +
                  ", snr: " + String(snr_g) +
                  ", doppler: " + String(doppler_g) +
                  ", rv: " + String(rv_g) +
                  "\n\U0001F4ED\t0x" + String(prnt_msg_g.local_address, HEX) +
                  ", \U000000A9\t0x" + String(prnt_msg_g.msg_type, HEX) +
                  ", \U0001F6F0\t0x" + String(prnt_msg_g.destination, HEX) +
                  "\n\U0001F194\t" + String(prnt_msg_g.msg_index) +
                  ", \U0001F4E6:\t" + String(prnt_msg_g.msg_size) +
                  ", Ack:\t" + String(prnt_msg_g.msg_ack_req) +
                  "\n\U000023F0\t" + String(h) + ":" + ((m < 10) ? "0" + String(m) : String(m));

    if (prnt_msg_g.msg_type == type_command)
    {
        command_data_t command_data;
        memcpy(command_data.packet, prnt_msg_g.msg_payload, prnt_msg_g.msg_size);
        encrypt_decrypt(&command_data);
        text += "\nCommand:\nCode: 0x" + String(command_data.cmd_code, HEX);
        text += ", Type: 0x" + String(command_data.cmd_type, HEX);
        text += ", Size: " + String(command_data.cmd_size);
        if (command_data.cmd_size > 0 && command_data.cmd_size < 20)
        {
            text += "\nPayload:\t" + message2hex(command_data.cmd_payload, command_data.cmd_size);
        }
    }

    text += "\nHeader: " + message2hex(prnt_msg_g.packet, LORA_HEADER_LENGTH);
    if (prnt_msg_g.msg_size > 0)
    {
        text += "\nPayload: " + message2hex(prnt_msg_g.msg_payload, prnt_msg_g.msg_size);
    }

    PRINT("Text: ");
    PRINTLN(text);

    myBot.setParseMode(CTBotParseModeMarkdown);
    myBot.sendMessage(target_id, text);
    delay(500); // wait 500 milliseconds

    if (tlg_ch_enable)
    {
        myBot.sendMessage(satlla0_channel, text);
        delay(500); // wait 500 milliseconds
    }
}

String message2hex(uint8_t *message, uint8_t message_length)
{
    String message_hex;
    for (uint8_t i = 0; i < message_length; i++)
    {
        if (message[i] < 16)
        {
            message_hex += "0";
        }
        message_hex += String(message[i], HEX);
    }
    return message_hex;
}
/* =============== */
/* Publish Otr Bcn */
/* =============== */

void publish_to_tlg_message(String message)
{
    return publish_to_tlg_message(satlla0_bot, message, 0);
}

void publish_to_tlg_message(String message, uint8_t parse_mode)
{
    return publish_to_tlg_message(satlla0_bot, message, parse_mode);
}

void publish_to_tlg_message(int64_t target_id, String message, uint8_t parse_mode)
{
    PRINTLN("Func: publish_to_tlg_message()");
    if (message.length() <= 0)
    {
        return;
    }

    if (parse_mode)
    {
        myBot.setParseMode(CTBotParseModeHTML);
    }
    else
    {
        myBot.setParseMode(CTBotParseModeMarkdown);
    }

    if (abs(rssi_g) || abs(snr_g) || abs(doppler_g) || abs(rv_g))
    {
        message = "Station " + String(local_address, HEX) +
                  ", rssi: " + String(rssi_g) +
                  ", snr:" + String(snr_g) +
                  ", doppler: " + String(doppler_g) +
                  ", rv: " + String(rv_g) +
                  "\n" + message;
    }
    else
    {
        message = "Station " + String(local_address, HEX) +
                  "\n" + message;
    }

    myBot.sendMessage(target_id, message);
    delay(500); // wait 500 milliseconds

    if (tlg_ch_enable)
    {
        myBot.sendMessage(satlla0_channel, message);
        delay(500); // wait 500 milliseconds
    }
}

#endif