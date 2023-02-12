/**
    @file GPS_Utils.ino
    @brief Integration with NavSpark for SATLLA0.

    This file contains function to receive data from the GPS for the SATLLA0.

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

#if GPS_ENABLE

/* ============ */
/* FAT          */
/* ============ */

void gps_set_state(bool state)
{
    PRINTLN("Func: gps_set_state()");
    if (state)
    {
        gps_turn_on();
    }
    else
    {
        gps_turn_off();
    }
}

void gps_turn_on()
{
    PRINTLN("Func: gps_turn_on()");
    digitalWrite(FAT_GPS_PIN, HIGH);
    gps_is_on = millis();
    gps_is_sleep = 0;
}

void gps_turn_off()
{
    PRINTLN("Func: gps_turn_off()");
    digitalWrite(FAT_GPS_PIN, LOW);
    gps_is_on = 0;
    gps_is_sleep = 0;
}

void gps_sleep()
{
    PRINTLN("Func: gps_sleep()");
    // TODO: Set GPS to sleep
    gps_is_sleep = 1;
}

void gps_resume()
{
    PRINTLN("Func: gps_resume()");
    // TODO: Set GPS to resume
    gps_is_sleep = 0;
}

void reset_gps()
{
    PRINTLN("Func: reset_gps()");
    gps_turn_off();
    delay(100);
    gps_turn_on();
}

void gps_health()
{
    PRINTLN("Func: gps_health()");
    // if GPS is off, then return
    if (!gps_is_on)
    {
        return;
    }

    // if no GPS for certain time, reset
    if (millis() > GPS_VALID_TIME_THRESHOLD && gps.charsProcessed() < GPS_CHARS_PROCESSED)
    {
        PRINTLN("Func: GPS Helath(): charsProcessed() < 10");
        // No GPS data received -> Reset GPS
        reset_gps();
    }

    if (!gps.location.isValid())
    {
        PRINTLN("Func: GPS Helath(): isValid() = false");
        if (millis() - gps_not_valid > GPS_NOT_VALID_THRESHOLD)
        {
            gps_not_valid = millis();
            reset_gps();
        }
    }
}

/* ============ */
/* GPS          */
/* ============ */

void gps_setup()
{
    PRINTLN("Func: gps_serial_setup()");
    gps_serial.begin(9600); // gps
}

void gps_prepare_data()
{
    PRINTLN("Func: gps_prepare_data():");
    // if valid location then update
    if (gps.location.isValid())
    {
        gps_data_g.gps_sat = gps.satellites.value();
        gps_data_g.gps_lat = gps.location.lat();
        gps_data_g.gps_lng = gps.location.lng();
        gps_data_g.gps_alt = (uint16_t)gps.altitude.kilometers();
        gps_data_g.gps_speed = (uint16_t)gps.speed.kmph();
        gps_data_g.gps_course = (uint16_t)(gps.course.deg() * CONST_100);
    }
    else
    {
        PRINTLN("Func: gps_prepare_data(): GPS location is invalid!");
    }

    if (gps.time.isValid() || gps.date.isValid())
    {
        // gps_date_time_g.gps_year = (uint8_t)(gps.date.year() % 100); //Take only the last 2 digits
        gps_date_time_g.gps_month = gps.date.month();
        gps_date_time_g.gps_day = gps.date.day();
        gps_date_time_g.gps_hour = gps.time.hour();
        gps_date_time_g.gps_min = gps.time.minute();
        // gps_date_time_g.gps_sec = gps.time.second();

        gps_data_g.gps_date_time = gps_date_time_g;
#if RTC_ENABLE
        // set the Time to the latest GPS reading
        setTime(gps.time.hour(), gps.time.minute(), gps.time.second(), gps.date.day(), gps.date.month(), gps.date.year());
#endif
    }
    else
    {
        PRINTLN("Func: gps_prepare_data(): GPS date/time is invalid!");
    }

#ifdef PRINT_FUNC_DEBUG
    print_gps_data(&gps_data_g); // Print GPS data //TODO: Remove
#endif
}

/* ============ */
/* Feed GPS     */
/* ============ */
char buffer[400] = {0};

void feed_gps()
{
    unsigned long timeout = millis();
    int i = 0;
    uint8_t len;
    uint8_t seq;
    uint8_t msg_id;
    if (gps_serial.available())
    {
        delay(SEC_1);
        uint8_t c = gps_serial.read();
        // PRINT(c);
        if (c == 0xFE)
        {
            buffer[0] = c;
            len = gps_serial.read();
            buffer[1] = len;
            seq = gps_serial.read();
            buffer[2] = seq;
            uint8_t sysid = gps_serial.read();
            buffer[3] = sysid;
            uint8_t comp = gps_serial.read();
            buffer[4] = comp;
            msg_id = gps_serial.read();
            buffer[5] = msg_id;
            while (i < len)
            {
                buffer[i + 6] = gps_serial.read();
                i++;
            }
            uint8_t crc_l = gps_serial.read();
            buffer[len + 6] = crc_l;
            uint8_t crc_h = gps_serial.read();
            buffer[len + 7] = crc_h;
        }
        if (i > 0)
        {
            PRINT("len:\t");
            PRINTLN(len);
            PRINT("seq:\t");
            PRINTLN(seq);
            PRINT("msg_id:\t");
            PRINTLN(msg_id);
            for (uint8_t j = 0; j < i + 8; j++)
            {
                if (buffer[j] < 16)
                {
                    PRINT(0);
                }
                PRINT(buffer[j], HEX);
                // if (!((j + 1) % 16))
                // {
                //     PRINTLN();
                // }
                // else
                // {
                //     PRINT(" ");
                // }
            }
            PRINTLN();
        }
    }
}

void feed_gps1()
{
    unsigned long timeout = millis();
    while (gps_serial.available() && (millis() - timeout < SEC_1))
    {
        char c = gps_serial.read();
        PRINT(c);
        gps.encode(c);
    }
}

#endif

