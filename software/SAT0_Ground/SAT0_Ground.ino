/**
    @file SAT0_Ground.ino
    @brief Master file for SATLLA0 Ground Station.
    Board: Teensy/Heltec32

    This file contains the main and setup functions for the SATLLA0 GS.

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

#define programversion "SATLLA0-GS"
#define Serial_Monitor_Baud 115200

#include "settings.h"

/* ============ */
/* Logo         */
/* ============ */

void logo()
{
#if LCD_SSD
    lcd.clear();
    lcd.drawXbm(0, 5, logo_width, logo_height, logo_bits);
    lcd.display();
    delay(SECS_2);
#endif
}

/* ============ */
/* Setup        */
/* ============ */

void setup()
{
    Serial.begin(Serial_Monitor_Baud);
    delay(HALF_SEC);
    PRINTLN("Setup Started");

    PRINTLN();
    PRINT(__TIME__);
    PRINT(F(" "));
    PRINTLN(__DATE__);
    PRINTLN(programversion);
    PRINTLN();

    get_date(DATE_, buffer_g);
    memcpy(firmware_version, buffer_g, 8);
    get_time(TIME_, buffer_g);
    memcpy(firmware_version + 8, buffer_g, 6);
    PRINT("FIRMWARE_VERSION:\t");
    PRINTLN(firmware_version);

    Wire.begin(OLED_SDA, OLED_SCL);

#if (DEVCIE == 1)   // Teensy
    SPI.setSCK(14); // For teensy
    SPI.begin();
#else
    SPI.begin(SCK, MISO, MOSI, NSS);
#endif

#if LCD_SSD
    // OLED
    pinMode(OLED_R, OUTPUT);
    digitalWrite(OLED_R, LOW); // set GPIO16 low to reset OLED
    delay(50);
    digitalWrite(OLED_R, HIGH); // while OLED is running, must set GPIO16 in high

    lcd.init();
    lcd.flipScreenVertically();
    lcd.clear();
    lcd.display();

    logo();

    lcd.clear();
    lcd.setTextAlignment(TEXT_ALIGN_CENTER);
    lcd.setFont(ArialMT_Plain_10);
    lcd.drawString(64, 0, "SATLLA0 GS Start");
    lcd.drawString(64, 10, String(local_address, HEX));
    lcd.drawString(64, 20, firmware_version);
    lcd.display();

    lcd.setTextAlignment(TEXT_ALIGN_LEFT);
    // lcd.setFont(DejaVu_Sans_10);
#endif

#if AXP20_ENABLE
    axp20_setup();
    axp20_raw_print();
#endif

#if EEPROM_ENABLE
    eeprom_setup();
    load_settings();
    print_settings();
#endif

#if WIFI_ENABLE
    wifi_setup();

#if OTA_ENABLE
    arduinoota_setup();
#endif

#if TELEGRAM_ENABLE
    tlg_setup();

    publish_to_tlg_message("Station was restarted!.");
#endif

#if TELEGRAM_BOT_ENABLE
    tlgrelay_setup();
#endif

#if FIREBASE_ENABLE
    firebase_setup();
#endif

#if TLE_ENABLE
    tle_setup();
#endif

#endif

    // LoRa 433 Setup
#if RF_433_ENABLE
    lora_433_setup();
    // modem_fsk_433_setup(); // Need to set FSK as well
#endif

#if RF_24_ENABLE
    // LoRa 2.4 Setup
    lora_24_setup();
#endif

#if GPS_ENABLE
    gps_setup();
#endif

    button_setup();

#if TRK_ENABLE
    trk_setup();
    find_north();
#endif

#if LCD_SSD
    lcd.drawString(0, 20, "Heltec.LoRa 433 success!");
    lcd.drawString(0, 30, "Heltec.LoRa 2.4 success!");
    lcd.drawString(0, 40, "Setup Completed");
    lcd.display();
#endif

    PRINTLN("Setup Completed");
}

/* ============ */
/* Loop         */
/* ============ */

void loop()
{
    // if restart requested wait 5 secs and restart
    if (restart_requested_flag && millis() - restart_requested_flag > SECS_10)
    {
        CPU_RESTART;
    }

    if (Serial.available() > 0)
    {
        serial_event();
    }

    if (button_pressed_flag)
    {
        PRINTLN("Flag: button_pressed_flag");
        if (digitalRead(BTN_PRG_PIN) == HIGH)
        {
            // PRINTLN("Flag: HIGH");
            button_setup();
        }
        else
        {
            // PRINTLN("Flag: LOW");
        }
    }

#if GPS_ENABLE
    if (gs_gps_enable)
    {
        gps_read();
        gps_info();
    }
#endif

#if RF_433_ENABLE
    if (received_433_flag)
    {
        received_433_flag = false;
        delay(10);
#if LORA_SX127X_ENABLE
        on_receive_433();
#endif
        handle_433_message();
    }
#endif

#if RF_24_ENABLE
    if (received_24_flag)
    {
        received_24_flag = false;
        delay(10);
        on_receive_24();
        handle_24_message();
        setup_lora_24_rx();
    }
#endif

#if WIFI_ENABLE

#if TELEGRAM_BOT_ENABLE
    if (millis() - tlgrelay_read_millis > tlg_read_threshold)
    {
        tlgrelay_read();
        tlgrelay_read_millis = millis();
    }
#endif

#if TELEGRAM_ENABLE
    if (millis() - tlg_read_millis > tlg_read_threshold)
    {
        tlg_read();
        tlg_read_millis = millis();
    }
#endif

#if TLE_ENABLE
    if (millis() - tle_read_millis > tle_read_threshold)
    {
        tle_loop();
        tle_read_millis = millis();
    }

    // update TLE each day
    if (millis() - tle_updated_millis > tle_updated_threshold)
    {
        tle_updated_millis = millis();
        update_TLE();
        switch_next_sat();
    }
#if TRK_ENABLE
    trk_loop();
#endif
#endif

#if OTA_ENABLE
    arduinoota_handle();
#endif

#endif

    if (millis() - change_radio_setting_timer > MINUTES_5 && change_radio_setting_timer)
    {
        change_radio_setting_timer = 0;
        setup_lora_433_default();
    }

    // sat_current_info sat_current;
    // get_start_pass(0, &sat_current);
    // print_current(&sat_current);

    //     if (!RXbuffer.isEmpty())
    //     {
    //         if (ENABLE_GOOGLE_DRIVE)
    //         {
    //             if (recieved_beacon || millis() - last_post_to_google > post_to_google_threshold)
    //             {
    //                 while (!RXbuffer.isEmpty())
    //                 {
    //                     PRINT("RXbuffer Element:\t");
    //                     PRINTLN(RXbuffer.numElements());
    //                     RXbuffer.pull(&ringbuffer_data_b);
    //                     print_message(ringbuffer_data_b.lora_data.packet, ringbuffer_data_b.lora_data.msg_size + LORA_HEADER_LENGTH, ringbuffer_data_b.freq);
    // #if (DEVCIE == 0)
    //                     //post the message on google docs
    //                     postToGoogleDoc(ringbuffer_data_b.lora_data.packet, ringbuffer_data_b.lora_data.msg_size + LORA_HEADER_LENGTH, ringbuffer_data_b.rssi, ringbuffer_data_b.snr, ringbuffer_data_b.freq);
    // #endif
    //                 }
    //                 last_post_to_google = millis();
    //             }
    //         }
    //         else
    //         {
    //             while (!RXbuffer.isEmpty())
    //             {
    //                 PRINT("RXbuffer Element:\t");
    //                 PRINTLN(RXbuffer.numElements());
    //                 RXbuffer.pull(&ringbuffer_data_b);
    //                 print_message(ringbuffer_data_b.lora_data.packet, ringbuffer_data_b.lora_data.msg_size + LORA_HEADER_LENGTH, ringbuffer_data_b.freq);
    //             }
    //         }
    //     }

    // delay(1000);
}

/* ================== */
/* Print Short Beacon */
/* ================== */
void handle_messgae()
{
    // // check if there is files in outbox
    // if (sd_outbox_g > 0)
    // {
    //     ld_get_metadata(lora_433);
    //     sd_outbox_g = 0;
    // }
}

/* =================== */
/* Print Other Message */
/* =================== */

void print_other_message(uint8_t *message, uint8_t message_length, lorafreq_e freq, int satid)
{
    PRINTLN("Func: print_other_message()");

    // json_message(message, message_length);

    PRINTLN("***********************");
    PRINTLN("Message: ");
    if (message_length)
    {
        PRINTLN("Payload: ");
        print_buffer(message, message_length);
    }
    PRINTLN();
    PRINTLN("***********************");

#if TLE_ENABLE && WIFI_ENABLE
    sat_current_info sat_current;
    get_current(satid, &sat_current);
    print_current(&sat_current);
#endif

#if TELEGRAM_ENABLE && TLE_ENABLE && WIFI_ENABLE
    String text = "\n\U0001F551 " +
                  String(sat_current.day) + ":" +
                  String(sat_current.mon) + ":" +
                  String(sat_current.year) + ", " +
                  String(sat_current.hr) + ":" +
                  String(sat_current.mnt) + ":" +
                  String(sat_current.sec);

    text += "\nCurrent: \U0001F4D0 " + String(sat_current.az) + "°, " +
            "\U0001F4CF " + String(sat_current.ele) + "°, " +
            "\U00002194 " + String(sat_current.dist) + " km";

    text += "\n\U0001F5FA <a href='https://www.google.com/maps/place/" +
            String(sat_current.lat) + "," +
            String(sat_current.lon) + "'>" +
            String(sat_current.lat) + "," +
            String(sat_current.lon) + "</a>, Alt:" +
            sat_current.alt + " km";

    publish_to_tlg_message(text, 2);
#endif

#if LCD_SSD
    lcd.clear();
    lcd.drawString(0, 0, "Other Message");

    lcd.display();
#endif
}

/* ============= */
/* Print Current */
/* ============= */

void print_current(sat_current_info *sat_current)
{
    PRINT("Date:\t");
    PRINTLN(String(sat_current->day) + ":" + String(sat_current->mon) + ":" + String(sat_current->year));
    PRINT("Time:\t");
    PRINTLN(String(sat_current->hr) + ":" + String(sat_current->mnt) + ":" + String(sat_current->sec));
    PRINT("SAT Az: ");
    PRINT(sat_current->az);
    PRINT(", Ele: ");
    PRINT(sat_current->ele);
    PRINT(", Distance: ");
    PRINTLN(sat_current->dist);
    PRINT("SAT Lat: ");
    PRINT(sat_current->lat);
    PRINT(", Lon: ");
    PRINT(sat_current->lon);
    PRINT(", Alt:");
    PRINTLN(sat_current->alt);

    PRINTLN();
}

/* ================== */
/* Print Short Beacon */
/* ================== */

void print_short_beacon(uint8_t *message, uint8_t message_length)
{
    PRINTLN("Func: print_short_beacon()");

    memcpy(prnt_short_beacon_g.packet, message, message_length);

    PRINTLN("***********************");
    PRINT("L.Addr:\t0x");
    PRINTLN(prnt_short_beacon_g.local_address, HEX);
    PRINT("Type:\t0x");
    PRINTLN(prnt_short_beacon_g.msg_type, HEX);
    PRINT("Index:\t");
    PRINTLN(prnt_short_beacon_g.msg_index);
    PRINT("Voltage:\t");
    PRINTLN(prnt_short_beacon_g.battery_volts);
    PRINT("Current:\t");
    PRINTLN(prnt_short_beacon_g.battery_current);
    PRINT("SD Outbox:\t");
    PRINTLN(prnt_short_beacon_g.sd_outbox);
    PRINT("Msg Received:\t");
    PRINTLN(prnt_short_beacon_g.msg_received);
    PRINT("NTC1:\t");
    PRINTLN(prnt_short_beacon_g.sns_ntc1);
    PRINT("NTC2:\t");
    PRINTLN(prnt_short_beacon_g.sns_ntc2);

    PRINTLN("Message: ");
    print_buffer(prnt_short_beacon_g.packet, SHRT_BCN_PACKET_LENGTH);
    PRINTLN();
    PRINTLN("***********************");

#if LCD_SSD
    lcd.clear();
    lcd.drawString(0, 0, "ID: " + String(prnt_short_beacon_g.local_address, HEX) + ". TYP: " + String(prnt_short_beacon_g.msg_type, HEX) + ". IDX: " + String(prnt_short_beacon_g.msg_index));
    lcd.drawString(0, 9, "BatV: " + String(prnt_short_beacon_g.battery_volts) + ". CUR: " + String(prnt_short_beacon_g.battery_current));
    lcd.drawString(0, 18, "SD: " + String(prnt_short_beacon_g.sd_outbox) + ". MsgR: " + String(prnt_short_beacon_g.msg_received));
    lcd.drawString(0, 27, "T1: " + String(prnt_short_beacon_g.sns_ntc1) + ". T2: " + String(prnt_short_beacon_g.sns_ntc2));
    lcd.display();
#endif

#if TELEGRAM_ENABLE && WIFI_ENABLE
    // post the message on google docs
    publish_to_tlg_short_becon(satlla0_bot, message, message_length);
#endif
#if FIREBASE_ENABLE && WIFI_ENABLE
    unsigned long timestamp = 0L;
#if TLE_ENABLE
    timestamp = getTime();
#endif
    fb_store_data(message, message_length, timestamp);
#endif
}

/* ============= */
/* Print Message */
/* ============= */
void print_message(uint8_t *message, uint8_t message_length){
    print_message(message, message_length, lora_none);
}

void print_message(uint8_t *message, uint8_t message_length, lorafreq_e freq)
{
    PRINTLN("***********************");
    PRINTLN("Func: print_message()");

    //     memset(prnt_msg_g.packet, 0x00, LORA_PACKET_LENGTH);
    memcpy(prnt_msg_g.packet, message, message_length);

    // if short beacon then use short message.
    if (prnt_msg_g.msg_type == type_sbeacon)
    {
        json_short_beacon(message, message_length);
        print_short_beacon(message, message_length);
    }
    else
    {
        json_message(message, message_length);

        PRINTLN("***********************");
        PRINT("L.Addr:\t0x");
        PRINTLN(prnt_msg_g.local_address, HEX);
        PRINT("Dest:\t0x");
        PRINTLN(prnt_msg_g.destination, HEX);
        PRINT("Type:\t0x");
        PRINTLN(prnt_msg_g.msg_type, HEX);
        PRINT("Size:\t");
        PRINTLN(prnt_msg_g.msg_size);
        PRINT("Total Size:\t");
        PRINTLN(prnt_msg_g.msg_size + LORA_HEADER_LENGTH);
        PRINT("Index:\t");
        PRINTLN(prnt_msg_g.msg_index);
        PRINT("Time:\t");
        PRINTLN(prnt_msg_g.msg_time);
        PRINT("Ack Req:\t");
        PRINTLN(prnt_msg_g.msg_ack_req);

        PRINTLN("Header:");
        print_buffer(prnt_msg_g.packet, LORA_HEADER_LENGTH);
        if (prnt_msg_g.msg_size)
        {
            PRINTLN("Payload:");
            print_buffer(prnt_msg_g.msg_payload, prnt_msg_g.msg_size);
        }
        PRINTLN();
        PRINTLN("***********************");

        memset(gps_data_p.packet, 0, GPS_PACKET_LENGTH);
        memset(sns_data_p.packet, 0, SNS_PACKET_LENGTH);
        memset(energy_data_p.packet, 0, ENERGY_PACKET_LENGTH);
        memset(sd_data_p.packet, 0, SD_DATA_LENGTH);
        memset(radio_data_p.packet, 0, RADIO_DATA_LENGTH);
        memset(info_data_p.packet, 0, INFO_DATA_LENGTH);
        memset(ld_meta_data_p.packet, 0, LD_META_DATA_LENGTH);

        // if message is a long beacon
        if (prnt_msg_g.msg_type == type_becon)
        {
            memcpy(gps_data_g.packet, prnt_msg_g.msg_payload, GPS_PACKET_LENGTH);
            print_gps_data(&gps_data_g);
            PRINTLN();

            if (prnt_msg_g.msg_size > GPS_PACKET_LENGTH)
            {
                memcpy(sns_data_g.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH, SNS_PACKET_LENGTH);
                print_sensor_data(&sns_data_g);
                PRINTLN();
            }

            if (prnt_msg_g.msg_size > GPS_PACKET_LENGTH + SNS_PACKET_LENGTH)
            {
                memcpy(battery_data_g.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH + SNS_PACKET_LENGTH, BATTERY_PACKET_LENGTH);
                print_battery_stats(&battery_data_g);
                PRINTLN();
            }

            if (prnt_msg_g.msg_size > GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH, SD_DATA_LENGTH)
            {
                memcpy(sd_data_g.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH, SD_DATA_LENGTH);
                print_sd_data(&sd_data_g);
                PRINTLN();
            }

            if (prnt_msg_g.msg_size > GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH + SD_DATA_LENGTH, INFO_DATA_LENGTH)
                ;
            {
                memcpy(info_data_p.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH + SD_DATA_LENGTH, INFO_DATA_LENGTH);
                print_info_data(&info_data_p);
                PRINTLN();
            }

            uint8_t ld_size = prnt_msg_g.msg_size - (GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH + SD_DATA_LENGTH + INFO_DATA_LENGTH);
            if (ld_size > 0)
            {
                memcpy(ld_meta_data_b.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH + SD_DATA_LENGTH + INFO_DATA_LENGTH, LD_META_HEADER_LENGTH + ld_size);
                print_ld_meta_data(&ld_meta_data_b);
            }

            //             if (prnt_msg_g.msg_size > GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH + SD_DATA_LENGTH - 2) // there is a mismatch of 2
            //             {
            //                 memcpy(radio_data_g.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH + SD_DATA_LENGTH, RADIO_DATA_LENGTH);
            //                 print_radio_data(&radio_data_g);
            //                 PRINTLN();
            //             }

#if TELEGRAM_ENABLE && WIFI_ENABLE
            // print the beacon to the TLG
            publish_to_tlg_long_becon(satlla0_bot, message, message_length);
#endif
#if FIREBASE_ENABLE && WIFI_ENABLE
            unsigned long timestamp = 0L;
#if TLE_ENABLE
            timestamp = getTime();
#endif
            fb_store_data(message, message_length, timestamp);
#endif
        }
        else
        {
            // message is not a long beacon.
#if TELEGRAM_ENABLE && WIFI_ENABLE
            publish_to_tlg_other_message(satlla0_bot, message, message_length);
#endif
#if FIREBASE_ENABLE && WIFI_ENABLE
            unsigned long timestamp = 0L;
#if TLE_ENABLE
            timestamp = getTime();
#endif
            fb_store_data(message, message_length, timestamp);
#endif
        }

        // if message is an ack message
        if (prnt_msg_g.msg_type == type_ack)
        {
            memcpy(radio_data_g.packet, prnt_msg_g.msg_payload, RADIO_DATA_LENGTH);
            print_radio_data(&radio_data_g);
            PRINTLN();
        }

        // print to LCD
#if LCD_SSD
        lcd.clear();
        lcd.drawString(0, 0, "ID: " + String(prnt_msg_g.local_address, HEX) + ". DST: " + String(prnt_msg_g.destination, HEX) + ". TYP: " + String(prnt_msg_g.msg_type, HEX));
        lcd.drawString(0, 9, "SZE: " + String(prnt_msg_g.msg_size) + " b" + ". ACK: " + String(prnt_msg_g.msg_ack_req, HEX));
        lcd.drawString(0, 18, "IDX: " + String(prnt_msg_g.msg_index) + ". TME: " + String(prnt_msg_g.msg_time));
        lcd.display();
#endif
    }

    PRINTLN("End: print_message()");
    PRINTLN("***********************");
}

/* ============ */
/* Print        */
/* ============ */

void print_gps_data(gps_data_t *gps_data)
{
    PRINTLN("Func: print_gps_data()");

    memset(buffer_g, 0, BUFFER_SIZE_MAX);
    PRINT("Time:\t\t");
    sprintf(buffer_g, "%02d:%02d" /*:%02d"*/, gps_data->gps_date_time.gps_hour, gps_data->gps_date_time.gps_min /*, gps_data->gps_date_time.gps_sec*/);
    PRINTLN(buffer_g);

    PRINT("Date:\t\t");
    sprintf(buffer_g, "%02d/%02d" /*/%02d"*/, gps_data->gps_date_time.gps_day, gps_data->gps_date_time.gps_month /*, gps_data->gps_date_time.gps_year*/);
    PRINTLN(buffer_g);

    PRINT("Location:\t");
    float gps_lat = (float)gps_data->gps_lat;
    float gps_lng = (float)gps_data->gps_lng;
    sprintf(buffer_g, "St:%02d, (Lt,Lg):(%0.6f, %0.6f), At:%f, Sp:%0.2f, He:%0.2f, Sat:%02d", gps_data->gps_sat, gps_lat, gps_lng, (float)gps_data->gps_alt, (float)gps_data->gps_speed, (float)gps_data->gps_course / CONST_100), gps_data->gps_sat;
    PRINTLN(buffer_g);

    print_distance(gps_lat, gps_lng, (float)gps_data->gps_alt);
}

/* ============ */
/* Print        */
/* Sensor Data  */
/* ============ */

void print_sensor_data(sns_data_t *sns_data)
{
    PRINTLN("Func: print_sensor_data()");

    // PRINT("Acc:\t\t");
    // PRINTLN((float)(sns_data->sns_mpu_acc) / CONST_100);
    // PRINT("Gyro:\t\t");
    // PRINTLN((float)(sns_data->sns_mpu_gyro) / CONST_100);
    // PRINT("Mag:\t\t");
    // PRINTLN((float)(sns_data->sns_mpu_mag) / CONST_100);

    PRINT("Temp:\t\t");
    PRINT((float)(sns_data->sns_bmp_temp_c) / CONST_100);
    PRINTLN(" c");
    // PRINT("Pres:\t\t");
    // PRINTLN(sns_data->sns_bmp_presure);
    // PRINT("Alt:\t\t");
    // PRINTLN(sns_data->sns_bmp_alt_m * CONST_10);

    PRINT("gx:\t");
    PRINT((float)(sns_data->sns_gx) / CONST_100);
    PRINTLN(" rad/s");
    PRINT("gy:\t");
    PRINT((float)(sns_data->sns_gy) / CONST_100);
    PRINTLN(" rad/s");
    PRINT("gz:\t");
    PRINT((float)(sns_data->sns_gz) / CONST_100);
    PRINTLN(" rad/s");

    PRINT("mx:\t");
    PRINT((float)(sns_data->sns_mx) / CONST_100);
    PRINTLN(" uTes");
    PRINT("my:\t");
    PRINT((float)(sns_data->sns_my) / CONST_100);
    PRINTLN(" uTes");
    PRINT("mz:\t");
    PRINT((float)(sns_data->sns_mz) / CONST_100);
    PRINTLN(" uTes");

    PRINT("ntc1:\t");
    PRINT(sns_data->sns_ntc1);
    PRINTLN(" c");
    PRINT("ntc2:\t");
    PRINT(sns_data->sns_ntc2);
    PRINTLN(" c");
    // PRINT("ntc3:\t\t");
    // PRINTLN(sns_data->sns_ntc3);
}

/* ============ */
/* Print        */
/* ============ */

void print_energy_data(energy_data_t *energy_data)
{
    PRINTLN("Func: print_energy_data()");

    PRINT("Bus Voltage:\t");
    PRINT((float)(energy_data->engy_busvoltage));
    PRINTLN(" V");
    PRINT("Shunt Voltage:\t");
    PRINT((float)(energy_data->engy_shuntvoltage) / CONST_1000);
    PRINTLN(" mV");
    PRINT("Load Voltage:\t");
    PRINT((float)(energy_data->engy_loadvoltage));
    PRINTLN(" V");
    PRINT("Current:\t");
    PRINT(energy_data->engy_current_ma);
    PRINTLN(" mA");
    PRINT("Power:\t\t");
    PRINT(energy_data->engy_power_mw);
    PRINTLN(" mW");
    PRINTLN("");
}

void print_battery_stats(battery_data_t *battery_data)
{
    PRINTLN("Func: print_battery_stats()");

    PRINT("soc:\t\t");
    PRINT(battery_data->battery_soc);
    PRINTLN("%");

    PRINT("volts:\t\t");
    PRINT(battery_data->battery_volts);
    PRINTLN(" mV");

    PRINT("current:\t\t");
    PRINT(battery_data->battery_current);
    PRINTLN(" mA");

    PRINT("capacity:\t\t");
    PRINT(battery_data->battery_capacity);
    PRINTLN(" mAh");

    PRINT("fullCapacity:\t");
    PRINT(battery_data->battery_full_capacity);
    PRINTLN(" mAh");

    PRINT("power:\t\t");
    PRINT(battery_data->battery_power);
    PRINTLN(" mW");

    PRINT("health:\t\t");
    PRINT(battery_data->battery_health);
    PRINTLN("%");
}

/* ============= */
/* Print SD Data */
/* ============= */

void print_sd_data(sd_data_t *sd_data)
{
    PRINTLN("Func: print_sd_data()");

    PRINT("Outbox:\t");
    PRINTLN(sd_data->sd_outbox);
    PRINT("Sent:\t");
    PRINTLN(sd_data->sd_sent);
    PRINT("RPI:\t");
    PRINTLN(sd_data->sd_rpi);
    PRINT("Files:\t");
    PRINTLN(sd_data->sd_files);
}

/* ============= */
/* Print Radio   */
/* ============= */

void print_radio_data(radio_data_t *radio_data)
{
    PRINTLN("Func: print_radio_data()");

    PRINT("RSSI:\t");
    int rssi = radio_data->rssi * -1;
    PRINTLN(rssi);
    PRINT("SNR:\t");
    int snr = radio_data->snr;
    PRINTLN(snr);
}

void print_info_data(info_data_t *info_data)
{
    PRINTLN("Func: print_info_data()");

    PRINT("Reset index:\t");
    PRINTLN(info_data->reset_index);
    PRINT("RX SAT Counter:\t");
    PRINTLN(info_data->rx_sat_counter);
    PRINT("Message Received:\t");
    PRINTLN(info_data->msg_received);
}

/* ============ */
/* DISTANCE     */
/* ============ */

void print_distance(float lat1, float lng1, float alt1)
{
    double distanceKm = TinyGPSPlus::distanceBetween(lat1, lng1, gs_lat, gs_lng) / 1000.0;
    double angle = atan2(abs(alt1 - 0.670), distanceKm);
    angle = (angle * 4068) / 71;
    // Keep angle between 0 and 360
    angle = angle + ceil(-angle / 360) * 360;
    double courseToBalloon = TinyGPSPlus::courseTo(gs_lat, gs_lng, lat1, lng1);
    const char *cardinalToBalloon = TinyGPSPlus::cardinal(courseToBalloon);
    PRINT("HEAD,");
    PRINT("Ariel,");
    PRINT("Distance,");
    PRINT(distanceKm);
    PRINT(",Km");
    PRINT(",Degree,");
    PRINT(angle);
    PRINT(",Course,");
    PRINT(courseToBalloon);
    PRINT(",Directions,");
    PRINT(cardinalToBalloon);
    PRINT(",Opposite,");
    PRINTLN(360 - courseToBalloon);
    PRINTLN();
}

void json_short_beacon(uint8_t *message, uint8_t message_length)
{
    PRINTLN("Func: json_short_beacon()");
    memcpy(prnt_short_beacon_g.packet, message, message_length);

    doc.clear();
    doc["local_address"] = prnt_short_beacon_g.local_address;
    doc["msg_type"] = prnt_short_beacon_g.msg_type;
    doc["msg_index"] = prnt_short_beacon_g.msg_index;
    doc["battery_volts"] = serialized(String(prnt_short_beacon_g.battery_volts));
    doc["battery_current"] = serialized(String(prnt_short_beacon_g.battery_current));
    doc["sd_outbox"] = prnt_short_beacon_g.sd_outbox;
    doc["msg_received"] = prnt_short_beacon_g.msg_received;
    doc["ntc1"] = prnt_short_beacon_g.sns_ntc1;
    doc["ntc2"] = prnt_short_beacon_g.sns_ntc2;

    // serializeJson(doc, Serial);
    serializeJsonPretty(doc, Serial);
    PRINTLN();
}

void json_message(uint8_t *message, uint8_t message_length)
{
    PRINTLN("Func: json_message()");

    // lora_data_t lora_message;
    memset(prnt_msg_g.packet, 0, LORA_PACKET_LENGTH);
    memcpy(prnt_msg_g.packet, message, message_length);

    doc.clear();
    doc["local_address"] = prnt_msg_g.local_address;
    doc["destination"] = prnt_msg_g.destination;
    doc["msg_type"] = prnt_msg_g.msg_type;
    doc["msg_size"] = prnt_msg_g.msg_size;
    doc["msg_index"] = prnt_msg_g.msg_index;
    doc["msg_time"] = prnt_msg_g.msg_time;
    doc["msg_ack_req"] = prnt_msg_g.msg_ack_req;

    memset(gps_data_g.packet, 0, GPS_PACKET_LENGTH);
    memset(sns_data_g.packet, 0, SNS_PACKET_LENGTH);
    memset(energy_data_g.packet, 0, ENERGY_PACKET_LENGTH);
    memset(sd_data_g.packet, 0, SD_DATA_LENGTH);

    if (prnt_msg_g.msg_size > 0)
    {
        if (prnt_msg_g.msg_type == type_becon)
        {
            memcpy(gps_data_g.packet, prnt_msg_g.msg_payload, GPS_PACKET_LENGTH);
            JsonObject gps_date_json = doc.createNestedObject("gps_date_json");

            gps_date_json["gps_month"] = gps_data_g.gps_date_time.gps_month;
            gps_date_json["gps_day"] = gps_data_g.gps_date_time.gps_day;

            gps_date_json["gps_hour"] = gps_data_g.gps_date_time.gps_hour;
            gps_date_json["gps_min"] = gps_data_g.gps_date_time.gps_min;

            JsonObject gps_data_json = doc.createNestedObject("gps_data_json");
            gps_data_json["gps_lat"] = gps_data_g.gps_lat;
            gps_data_json["gps_lng"] = gps_data_g.gps_lng;
            gps_data_json["gps_alt"] = (float)gps_data_g.gps_alt;
            gps_data_json["gps_speed"] = (float)gps_data_g.gps_speed;
            gps_data_json["gps_course"] = (float)(gps_data_g.gps_course) / CONST_100;
            gps_data_json["gps_sat"] = gps_data_g.gps_sat;

            if (prnt_msg_g.msg_size > GPS_PACKET_LENGTH + SNS_PACKET_LENGTH)
            {
                memcpy(sns_data_g.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH, SNS_PACKET_LENGTH);

                JsonObject sns = doc.createNestedObject("sns");
                // sns["sns_mpu_acc"] = (float)(sns_data_g.sns_mpu_acc) / CONST_100;
                // sns["sns_mpu_gyro"] = (float)(sns_data_g.sns_mpu_gyro) / CONST_100;
                // sns["sns_mpu_mag"] = (float)(sns_data_g.sns_mpu_mag) / CONST_100;

                sns["sns_bmp_temp_c"] = (float)(sns_data_g.sns_bmp_temp_c) / CONST_100;
                // sns["sns_bmp_alt_m"] = sns_data_g.sns_bmp_alt_m * CONST_10;

                sns["sns_gx"] = (float)(sns_data_g.sns_gx) / CONST_100;
                sns["sns_gy"] = (float)(sns_data_g.sns_gy) / CONST_100;
                sns["sns_gz"] = (float)(sns_data_g.sns_gz) / CONST_100;

                sns["sns_mx"] = (float)(sns_data_g.sns_mx) / CONST_100;
                sns["sns_my"] = (float)(sns_data_g.sns_my) / CONST_100;
                sns["sns_mz"] = (float)(sns_data_g.sns_mz) / CONST_100;

                sns["sns_ntc1"] = sns_data_g.sns_ntc1;
                sns["sns_ntc2"] = sns_data_g.sns_ntc2;
                // sns["sns_ntc3"] = sns_data_g.sns_ntc3;
            }
            if (prnt_msg_g.msg_size > GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH - 2) // there is a mismatch of 2
            {
                memcpy(battery_data_g.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH + SNS_PACKET_LENGTH, BATTERY_PACKET_LENGTH);

                JsonObject energy = doc.createNestedObject("energy");
                energy["battery_soc"] = battery_data_g.battery_soc;
                energy["battery_volts"] = battery_data_g.battery_volts;
                energy["battery_current"] = battery_data_g.battery_current;
                energy["battery_capacity"] = battery_data_g.battery_capacity;
                energy["battery_full_capacity"] = battery_data_g.battery_full_capacity;
                energy["battery_power"] = battery_data_g.battery_power;
                energy["battery_health"] = battery_data_g.battery_health;
            }

            if (prnt_msg_g.msg_size > GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH - 2) // there is a mismatch of 2
            {
                memcpy(sd_data_g.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH, SD_DATA_LENGTH);

                JsonObject sd = doc.createNestedObject("sd");
                sd["sd_outbox"] = sd_data_g.sd_outbox;
                sd["sd_sent"] = sd_data_g.sd_sent;
                sd["sd_rpi"] = sd_data_g.sd_rpi;
                sd["sd_files"] = sd_data_g.sd_files;
            }

            if (prnt_msg_g.msg_size > GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH + SD_DATA_LENGTH - 2) // there is a mismatch of 2
            {
                memcpy(radio_data_g.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH + SD_DATA_LENGTH, RADIO_DATA_LENGTH);

                JsonObject radio = doc.createNestedObject("radio");
                radio["rssi"] = radio_data_g.rssi;
                radio["snr"] = radio_data_g.snr;
            }
        }
    }

    serializeJsonPretty(doc, Serial);
    PRINTLN();
}

void print_setting(setting_data_t *setting_data)
{
    PRINTLN("Func: print_setting()");

    // PRINTLN("set_eepromset:\t0x" + String(setting_data->set_eepromset, HEX));
    // PRINTLN("set_device_mode:\t" + String(setting_data->set_device_mode));
    // PRINTLN("set_local_address:\t0x" + String(setting_data->set_local_address, HEX));
    // PRINTLN("set_destination:\t0x" + String(setting_data->set_destination, HEX));
}

/* ============= */
/* Print Message */
/* ============= */

void print_command(command_data_t *command_data)
{
    PRINTLN("Func: print_command()");
    PRINT("Code:\t0x");
    PRINTLN(command_data->cmd_code, HEX);
    PRINT("Type:\t0x");
    PRINTLN(command_data->cmd_type, HEX);
    PRINT("Size:\t");
    PRINTLN(command_data->cmd_size);
    PRINT("Chksum:\t");
    PRINTLN(command_data->cmd_chksum, HEX);

    if (command_data->cmd_size)
    {
        PRINTLN("Payload: ");
        print_buffer(command_data->cmd_payload, command_data->cmd_size);
    }
    PRINTLN();
    PRINTLN("***********************");
}

void print_ld_meta_data(ld_meta_data_t *ld_meta_data)
{
    PRINTLN("Func: print_ld_meta_data()");
    PRINTLN("***********************");
    PRINT("ld_type:\t");
    PRINTLN(ld_meta_data->ld_type);
    PRINT("ld_size:\t");
    PRINTLN(ld_meta_data->ld_size);
    PRINTLN("Payload:");
    print_buffer(ld_meta_data->packet + LD_META_HEADER_LENGTH, ld_meta_data->ld_size);
    PRINTLN("***********************");
    PRINTLN();
}


void set_gs_location(float lat, float lng)
{
    PRINTLN("Func: set_gs_location()");
    // range from -90 to 90 for latitude and -180 to 180 for longitude
    if (lat < -90 || lat > 90)
    {
        gs_lat = MYLOC_LAT;
    }
    else
    {
        gs_lat = lat;
    }

    if (lng < -180 || lng > 180)
    {
        gs_lng = MYLOC_LNG;
    }
    else
    {
        gs_lng = lng;
    }

#if EEPROM_ENABLE
    update_gs_loc(lat, lng);
#endif
}

void get_date(char const *date, char *buff)
{
    PRINTLN("Func: get_date()");
    int month, day, year;
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    sscanf(date, "%s %d %d", buff, &day, &year);
    month = (strstr(month_names, buff) - month_names) / 3 + 1;
    sprintf(buff, "%d%02d%02d", year, month, day);
    // Serial.println(buff);
}

void get_time(char const *time, char *buff)
{
    PRINTLN("Func: get_time()");
    int hour, mnt, sec;
    char tempbuff[2];
    memcpy(tempbuff, time, 2);
    hour = atol(tempbuff);
    memcpy(tempbuff, time + 3, 2);
    mnt = atol(tempbuff);
    memcpy(tempbuff, time + 6, 2);
    sec = atol(tempbuff);
    sprintf(buff, "%02d%02d%02d", hour, mnt, sec);
    // Serial.println(buff);
}

uint8_t get_satid_by_call_sign(uint8_t call_sign)
{
    PRINTLN("Func: get_satid_by_call_sign()");
    if ((call_sign >= SATLLA0_CALL_SIGN && call_sign < SATLLA0_CALL_SIGN + 0x16) ||
        (call_sign >= GS0_CALL_SIGN && call_sign < GS0_CALL_SIGN + 0x16))
    {
        return 0;
    }
    return 1;
}