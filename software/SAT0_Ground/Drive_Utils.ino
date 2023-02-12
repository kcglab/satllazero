/**
    @file Drive_Utils.ino
    @brief Google Drive Untils for SATLLA0 GS.

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

#if (DEVCIE == BOARD_HELTEC)
#if DRIVE_ENABLE && WIFI_ENABLE

/* ============ */
/* Google Docs  */
/* ============ */

void postToGoogleDoc(uint8_t *message, uint8_t messageLength, int rssi, float snr, lorafreq_e freq)
{
    PRINTLN("Func: postToGoogleDoc()");
    if (messageLength <= 0)
    {
        return;
    }

    memset(prnt_msg_g.packet, 0, LORA_PACKET_LENGTH);
    memcpy(prnt_msg_g.packet, message, messageLength);

    String header_data;
    for (uint8_t i = 0; i < LORA_HEADER_LENGTH - 1; i++)
    {
        if (prnt_msg_g.packet[i] < 16)
        {
            header_data += "0";
        }
        header_data += String(prnt_msg_g.packet[i], HEX);
    }

    String message_data;
    for (uint8_t i = 0; i < prnt_msg_g.msg_size; i++)
    {
        if (prnt_msg_g.packet[i + LORA_HEADER_LENGTH - 1] < 16)
        {
            message_data += "0";
        }
        message_data += String(prnt_msg_g.packet[i + LORA_HEADER_LENGTH - 1], HEX);
    }

    // Post to Google Form.
    PRINT("Connecting to ");
    PRINTLN(host);

    String band_str = lora_freq_convert(freq);

    if (prnt_msg_g.msg_type == type_becon)
    {
        memcpy(gps_data_g.packet, prnt_msg_g.msg_payload, GPS_PACKET_LENGTH);
        memcpy(sns_data_g.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH, SNS_PACKET_LENGTH);
        memcpy(energy_data_g.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH + SNS_PACKET_LENGTH, ENERGY_PACKET_LENGTH);
    }
    else
    {
        memset(gps_data_g.packet, 0, GPS_PACKET_LENGTH);
        memset(sns_data_g.packet, 0, SNS_PACKET_LENGTH);
        memset(energy_data_g.packet, 0, ENERGY_PACKET_LENGTH);
    }

    sprintf(buffer_g, "%dh%02dm%02ds", (prnt_msg_g.msg_time / 3600), (prnt_msg_g.msg_time / 60) % 60, (prnt_msg_g.msg_time % 60));
    String msg_time = buffer_g;

    sprintf(buffer_g, "%02d:%02d"/*:%02d"*/, gps_data_g.gps_date_time.gps_hour, gps_data_g.gps_date_time.gps_min/*, gps_data_g.gps_date_time.gps_sec*/);
    String gps_time = buffer_g;

    sprintf(buffer_g, "%02d/%02d"/*/%02d"*/, gps_data_g.gps_date_time.gps_day, gps_data_g.gps_date_time.gps_month/*, gps_data_g.gps_date_time.gps_year*/);
    String gps_date = buffer_g;

    String gps_sat = String(gps_data_g.gps_sat);

    sprintf(buffer_g, "%.6f", gps_data_g.gps_lat);
    String gps_lat = buffer_g;

    sprintf(buffer_g, "%.6f", gps_data_g.gps_lng);
    String gps_lng = buffer_g;

    sprintf(buffer_g, "%.3f", gps_data_g.gps_alt);
    String gps_alt = buffer_g;

    String gps_speed = String(gps_data_g.gps_speed);
    String gps_course = String(gps_data_g.gps_course);

    String sns_mpu_acc = String(sns_data_g.sns_mpu_acc);
    String sns_mpu_gyro = String(sns_data_g.sns_mpu_gyro);
    String sns_mpu_mag = String(sns_data_g.sns_mpu_mag);

    String sns_bmp_temp_c = "0";//String(sns_data_g.sns_bmp_temp_c);
    String sns_bmp_presure = "0";//String(sns_data_g.sns_bmp_presure);
    String sns_bmp_alt_m = "0";//String(sns_data_g.sns_bmp_alt_m * CONST_10);

    String sns_gx = String(sns_data_g.sns_gx);
    String sns_gy = String(sns_data_g.sns_gy);
    String sns_gz = String(sns_data_g.sns_gz);

    String sns_mx = String(sns_data_g.sns_mx);
    String sns_my = String(sns_data_g.sns_my);
    String sns_mz = String(sns_data_g.sns_mz);

    String engy_busvoltage = String(energy_data_g.engy_busvoltage % CONST_1000);
    String engy_shuntvoltage = String(energy_data_g.engy_shuntvoltage % CONST_1000);
    String engy_loadvoltage = String(energy_data_g.engy_loadvoltage % CONST_1000);
    String engy_current_ma = String(energy_data_g.engy_current_ma % CONST_10);
    String engy_power_mw = String(energy_data_g.engy_power_mw % CONST_10);

    String url = String("/macros/s/" + SCRIPT_ID_S + "/exec" +
                        "?sender=" + String(prnt_msg_g.local_address, HEX) +
                        "&destination=" + String(prnt_msg_g.destination, HEX) +
                        "&type=" + String(prnt_msg_g.msg_type, HEX) +
                        "&size=" + (String)prnt_msg_g.msg_size +
                        "&ack=" + (String)prnt_msg_g.msg_ack_req +
                        "&index=" + (String)prnt_msg_g.msg_index +
                        "&time=" + msg_time +
                        "&header=" + header_data +
                        "&message=" + urlencode(message_data) +
                        "&rssi=" + (String)rssi +
                        "&snr=" + (String)snr +
                        "&freq=" + (String)band_str +
                        "&gps_sat=" + gps_sat +
                        "&gps_lat=" + gps_lat +
                        "&gps_lng=" + gps_lng +
                        "&gps_alt=" + gps_alt +
                        "&gps_speed=" + gps_speed +
                        "&gps_course=" + gps_course +
                        "&gps_time=" + gps_time +
                        "&gps_date=" + gps_date +
                        "&sns_mpu_acc=" + sns_mpu_acc +
                        "&sns_mpu_gyro=" + sns_mpu_gyro +
                        "&sns_mpu_mag=" + sns_mpu_mag +
                        "&sns_bmp_temp_c=" + sns_bmp_temp_c +
                        "&sns_bmp_presure=" + sns_bmp_presure +
                        "&sns_bmp_alt_m=" + sns_bmp_alt_m +
                        "&sns_gx=" + sns_gx +
                        "&sns_gy=" + sns_gy +
                        "&sns_gz=" + sns_gz +
                        "&sns_mx=" + sns_mx +
                        "&sns_my=" + sns_my +
                        "&sns_mz=" + sns_mz +
                        "&engy_busvoltage=" + engy_busvoltage +
                        "&engy_shuntvoltage=" + engy_shuntvoltage +
                        "&engy_loadvoltage=" + engy_loadvoltage +
                        "&engy_current_ma=" + engy_current_ma +
                        "&engy_power_mw=" + engy_power_mw);

    PRINT("requesting URL: ");
    PRINTLN(url);

    client1 = new HTTPSRedirect(httpsPort);
    client1->setPrintResponseBody(true);
    client1->setContentTypeHeader("application/json");

    PRINT("Connecting to ");
    PRINTLN(host);

    // Try to connect for a maximum of 5 times
    bool flag = false;
    for (int i = 0; i < 2; i++)
    {
        int retval = client1->connect(host, httpsPort);
        if (retval == 1)
        {
            flag = true;
            break;
        }
        else
        {
            PRINTLN("Connection failed. Retrying...");
        }
        delay(10);
    }

    if (!flag)
    {
        PRINT("Could not connect to server: ");
        PRINTLN(host);
        PRINTLN("Exiting...");
        delete client1;
        client1 = nullptr;

        return;
    }

    // fetch spreadsheet data
    client1->GET(url, host);
    PRINTLN("Exiting...");

    delete client1;
    client1 = nullptr;

    return;
}

String lora_freq_convert(lorafreq_e freq)
{
    String band;
    switch (freq)
    {
    case lora_433:
        band = "UHF";
        break;

    case lora_24:
        band = "2.4";
        break;

    default:
        band = "None";
        break;
    }
    return band;
}

String urldecode(String str)
{

    String encodedString = "";
    char c;
    char code0;
    char code1;
    for (int i = 0; i < str.length(); i++)
    {
        c = str.charAt(i);
        if (c == '+')
        {
            encodedString += ' ';
        }
        else if (c == '%')
        {
            i++;
            code0 = str.charAt(i);
            i++;
            code1 = str.charAt(i);
            c = (h2int(code0) << 4) | h2int(code1);
            encodedString += c;
        }
        else
        {

            encodedString += c;
        }

        yield();
    }

    return encodedString;
}

String urlencode(String str)
{
    String encodedString = "";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i = 0; i < str.length(); i++)
    {
        c = str.charAt(i);
        if (c == ' ')
        {
            encodedString += '+';
        }
        else if (isalnum(c))
        {
            encodedString += c;
        }
        else
        {
            code1 = (c & 0xf) + '0';
            if ((c & 0xf) > 9)
            {
                code1 = (c & 0xf) - 10 + 'A';
            }
            c = (c >> 4) & 0xf;
            code0 = c + '0';
            if (c > 9)
            {
                code0 = c - 10 + 'A';
            }
            code2 = '\0';
            encodedString += '%';
            encodedString += code0;
            encodedString += code1;
        }
        yield();
    }
    return encodedString;
}

unsigned char h2int(char c)
{
    if (c >= '0' && c <= '9')
    {
        return ((unsigned char)c - '0');
    }
    if (c >= 'a' && c <= 'f')
    {
        return ((unsigned char)c - 'a' + 10);
    }
    if (c >= 'A' && c <= 'F')
    {
        return ((unsigned char)c - 'A' + 10);
    }
    return (0);
}

#endif
#endif