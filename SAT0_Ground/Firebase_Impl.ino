/**
    @file Firebase_Impl.ino
    @brief Firebase utils functions for SATLLA0 GS.

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

#if FIREBASE_ENABLE && WIFI_ENABLE

#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "common.h"

// Provide the token generation process info.
#include <addons/TokenHelper.h>

/* 2. Define the API Key */
#define API_KEY "apikey"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "myemail@email.com"
#define USER_PASSWORD "password"

/* 4. Define the Firebase storage bucket ID e.g bucket-name.appspot.com */
#define STORAGE_BUCKET_ID "mubukect.appspot.com"

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 3. Define the RTDB URL */
#define DATABASE_URL "mydb-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted = false;

void firebase_setup()
{
    PRINTLN("Func: firebase_setup()");

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

uint8_t fb_read_TLE(char *t_buffer, uint8_t satid)
{
    bool success = false;

    String TLE1 = "";
    String TLE2 = "";

    String path = "/TLE_" + String(satid) + "/TLE1";
    success = Firebase.RTDB.getString(&fbdo, path);
    if (success)
    {
        TLE1 = fbdo.stringData();
        TLE1.toCharArray(t_buffer, 70);
        PRINTLN(TLE1);
    }
    else
    {
        PRINTLN(fbdo.errorReason().c_str());
    }

    path = "/TLE_" + String(satid) + "/TLE2";
    success = Firebase.RTDB.getString(&fbdo, path);
    if (success)
    {
        TLE2 = fbdo.stringData();
        TLE2.toCharArray(t_buffer + 70, 70);
        PRINTLN(TLE2);
    }
    else
    {
        PRINTLN(fbdo.errorReason().c_str());
    }

    return success;
}

void update_firmware()
{
    if (Firebase.ready())
    {
        PRINTLN("Firebase Ready!");
        bool success = false;

        String url = "";
        String version = "";

        String path = "/firmware/" + String(local_address, HEX) + "/version";
        success = Firebase.RTDB.getString(&fbdo, path);
        if (success)
        {
            version = fbdo.stringData();
            PRINTLN(version);
        }
        else
        {
            PRINTLN(fbdo.errorReason().c_str());
        }

        if (strcmp(version.c_str(), firmware_version) <= 0)
        {
            PRINTLN("Already up to date!.");
        }
        else
        {
            PRINTLN("Download firmware file...");

            path = "/firmware/" + String(local_address, HEX) + "/filename";
            success = Firebase.RTDB.getString(&fbdo, path);
            if (success)
            {
                url = fbdo.stringData();
                PRINTLN(url);

                // In ESP8266, this function will allocate 16k+ memory for internal SSL client.
                if (!Firebase.Storage.downloadOTA(&fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */, url /* path of firmware file stored in the bucket */ /*, fcsDownloadCallback*/ /* callback function */))
                {
                    PRINTLN(fbdo.errorReason());
                }
                else
                {
                    PRINTLN("Update firmware completed.");
                    PRINTLN();
                    PRINTLN("Restarting...\n\n");
                    delay(2000);
                    ESP.restart();
                }

                // Serial.printf("Get file Metadata... %s\n", Firebase.Storage.getMetadata(&fbdo, STORAGE_BUCKET_ID, "SAT3_Ground.ino.bin" /* remote file */) ? "ok" : fbdo.errorReason().c_str());
            }
            else
            {
                PRINTLN(fbdo.errorReason().c_str());
            }
        }
    }
    else
    {
        PRINTLN("Firebase Not Ready!");
    }
}

void fb_store_data(uint8_t *message, uint8_t message_length, unsigned long timestamp)
{
    PRINTLN("Func: fb_store_data()");

    if (message_length <= 0)
    {
        return;
    }
    memset(prnt_msg_g.packet, 0, LORA_PACKET_LENGTH);
    memcpy(prnt_msg_g.packet, message, message_length);
    uint8_t h = (prnt_msg_g.msg_time / 3600);
    uint8_t m = (prnt_msg_g.msg_time / 60) % 60;

    FirebaseJson json;

    json.add("station", String(local_address, HEX));
    json.add("rssi", String(rssi_g));
    json.add("snr", String(snr_g));
    json.add("doppler", String(doppler_g));
    json.add("local_address", String(prnt_msg_g.local_address, HEX));
    json.add("msg_type", String(prnt_msg_g.msg_type, HEX));
    json.add("msg_index", String(prnt_msg_g.msg_index));
    json.add("msg_size", String(prnt_msg_g.msg_size));
    json.add("time", String(h) + ":" + ((m < 10) ? "0" + String(m) : String(m)));
    String message_hex;
    for (int i = 0; i < message_length; i++)
    {
        if (message[i] < 16)
        {
            message_hex += '0';
        }
        message_hex += String(message[i], HEX);
    }
    json.add("message", message_hex);

    memset(buffer_g, 0x00, BUFFER_SIZE_MAX);

    time_t now = (time_t) timestamp;
    struct tm  ts;
    ts = *localtime(&now);
    strftime(buffer_g, BUFFER_SIZE_MAX, "%Y%m%d_%H%M%S", &ts);
    String time_stamp = String(buffer_g);

    String path = "/data/" + String(local_address, HEX) + "/" + String(prnt_msg_g.local_address, HEX) + "/" + time_stamp + "/";

    if (!Firebase.RTDB.setJSON(&fbdo, path, &json))
    {
        PRINTLN(fbdo.errorReason());
    }
    else
    {
        PRINTLN("Update completed.");
    }
}

#endif