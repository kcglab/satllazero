/**
    @file WiFi_Utils.ino
    @brief WiFi utils file function for SATLLA0 GS.

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

/* ============ */
/* Wifi Setup  */
/* ============ */

#if WIFI_ENABLE

#include "HTTPSRedirect.h"
#include <WiFi.h>
#include <WiFiMulti.h>

//*******  Setup wifi ! ***************
const char *WFSSID1 = "user";
const char *WFPASS1 = "password";
const char *NO_PASSWD = "";

const uint16_t port = 80;
bool wifi_connected = false;

// Define hostid where data will be send in script format
const char *host = "script.google.com";
// define the secure port for HTTP protocol = 443
const int httpsPort = 443;

HTTPSRedirect *client1 = nullptr;

WiFiMulti WiFiMulti;

#if OTA_ENABLE
#include <ArduinoOTA.h>
#endif

void wifi_setup()
{
    PRINTLN("Func: wifi_setup()");
    PRINT("Waiting for WiFi... ");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFiMulti.addAP(WFSSID1, WFPASS1);

#if LCD_SSD
    lcd.clear();
#endif

    wifi_connect();

    if (WiFiMulti.run() == WL_CONNECTED)
    {
        wifi_connected = true;
        PRINTLN("WiFi connected");
        PRINT("SSID: ");
        PRINT(WiFi.SSID());
        PRINT(", IP address: ");
        PRINTLN(WiFi.localIP());

#if LCD_SSD
        lcd.drawString(0, 0, "WiFi: " + String(WiFi.SSID()));
        lcd.drawString(0, 10, "IP: " + WiFi.localIP().toString());
        lcd.display();
#endif
    }
    else
    {
        PRINTLN("WiFi not connected!");
#if LCD_SSD
        lcd.drawString(0, 0, "WiFi not connected!");
        lcd.display();
#endif
    }
}

void wifi_connect()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        int i = 0;
        while (WiFiMulti.run() != WL_CONNECTED && i < 20)
        {
            PRINT(".");
#if LCD_SSD
            lcd.drawString(i, 10, ".");
            lcd.display();
#endif
            delay(500);
            i += 2;
        }
        
        // If still no connection.
        if (WiFi.status() != WL_CONNECTED)
        {
            wifi_scan_and_connect();
        }
    }
}

void print_ip()
{
    PRINT("IP address: ");
    PRINTLN(WiFi.localIP());
}

void wifi_scan_and_connect()
{
    PRINTLN("Func: wifi_scan_and_connect()");
    // WiFi.scanNetworks will return the number of networks found
    WiFi.disconnect();
    WiFi.scanDelete();

    int8_t scanResult = 0;
    int i = 0;
    do
    {
        delay(500);
        i += 2;
        scanResult = WiFi.scanNetworks();
    } while (scanResult == WIFI_SCAN_RUNNING && i < 20);

    PRINTLN("scan done");
    if (scanResult <= 0)
    {
        PRINTLN("no networks found");
    }
    else
    {
        PRINT("Networks founds:\t");
        PRINTLN(scanResult);

        int idx = -1;
        int rssi = -1;
        for (int i = 0; i < scanResult; ++i)
        {
            // Print SSID and RSSI for each network found
            PRINT(i + 1);
            PRINT(": ");
            PRINT(WiFi.SSID(i));
            PRINT(" (");
            PRINT(WiFi.RSSI(i));
            PRINT(")");
            bool auth_open = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
            PRINTLN(auth_open ? " " : "*");
            if (auth_open)
            {
                PRINT("Connecting to:\t");
                PRINTLN(WiFi.SSID(i));
                memset(buffer_g, 0x00, BUFFER_SIZE_MAX);
                WiFi.SSID(i).toCharArray(buffer_g, 60);
                PRINTLN(buffer_g);
                WiFi.begin(buffer_g, NO_PASSWD);
                int w = 0;
                while (WiFi.status() != WL_CONNECTED && w < 20)
                {
#if LCD_SSD
                    lcd.drawString(i, 10, ".");
                    lcd.display();
#endif
                    delay(500);
                    w += 2;
                }
                if (WiFi.status() == WL_CONNECTED)
                {
                    break;
                }
            }

            delay(10);
        }
    }
}

#if OTA_ENABLE
void arduinoota_setup()
{
    if (WiFiMulti.run() == WL_CONNECTED)
    {
        // start the WiFi OTA library with internal (flash) based storage
        ArduinoOTA.begin();
    }
}

void arduinoota_handle()
{
    if (WiFiMulti.run() == WL_CONNECTED)
    {
        ArduinoOTA.handle();
    }
}

#endif
#endif