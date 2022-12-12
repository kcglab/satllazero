/**
    @file GPS_Airborne.ino.ino
    @brief GPS utils functions for SATLLA0 GS.

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

/*
Settings Array contains the following settings:
[0]NavMode,
[1]DataRate1,
[2]DataRate2,
[3]PortRateByte1,
[4]PortRateByte2,
[5]PortRateByte3,
[6]NMEA GLL Sentence,
[7]NMEA GSA Sentence,
[8NMEA GSV Sentence,
[9]NMEA RMC Sentence,
[10]NMEA VTG Sentence

NavMode:
Pedestrian Mode    = 0x03
Automotive Mode    = 0x04
Sea Mode           = 0x05
Airborne < 1G Mode = 0x06

DataRate:
1Hz     = 0xE8 0x03
2Hz     = 0xF4 0x01
3.33Hz  = 0x2C 0x01
4Hz     = 0xFA 0x00

PortRate:
4800   = C0 12 00
9600   = 80 25 00
19200  = 00 4B 00  **SOFTWARESERIAL LIMIT FOR ARDUINO UNO R3!**
38400  = 00 96 00  **SOFTWARESERIAL LIMIT FOR ARDUINO MEGA 2560!**
57600  = 00 E1 00
115200 = 00 C2 01
230400 = 00 84 03

NMEA Messages:
OFF = 0x00
ON  = 0x01
*/

#if GPS_ENABLE
#include <TinyGPS++.h>

#if GPS_ENABLE
#if (TTGO_VER == 0)
#define GPS_TX_PIN 12
#define GPS_RX_PIN 15
#elif (TTGO_VER == 1)
#define GPS_TX_PIN 34
#define GPS_RX_PIN 12
#else // HELTEC
#define GPS_TX_PIN -1
#define GPS_RX_PIN -1
#endif
#endif

TinyGPSPlus gps;
#define gps_serial Serial1

boolean gpsStatus[] = {false, false, false, false, false, false, false};

void gps_setup()
{
    PRINTLN("Func: gps_setup()");
    /* GPS */
    // gps_serial.begin(9600, SERIAL_8N1, 34, 12); //12-TX 15-RX
    gps_serial.begin(9600, SERIAL_8N1, GPS_TX_PIN, GPS_RX_PIN);

    // Airborne
    // byte settingsArray[] = {0x06, 0xE8, 0x03, 0x80, 0x25, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01};
    // Pedestrian
    byte settingsArray[] = {0x03, 0xE8, 0x03, 0x80, 0x25, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01};
    configureUblox(settingsArray);
}

void configureUblox(byte *settingsArrayPointer)
{
    PRINTLN("Func: configureUblox()");
    byte gpsSetSuccess = 0;
    PRINTLN("Configuring u-Blox GPS initial state...");

    // Generate the configuration string for Navigation Mode
    byte setNav[] = {0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, *settingsArrayPointer, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    calcChecksum(&setNav[2], sizeof(setNav) - 4);

    // Generate the configuration string for Data Rate
    byte setDataRate[] = {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, settingsArrayPointer[1], settingsArrayPointer[2], 0x01, 0x00, 0x01, 0x00, 0x00, 0x00};
    calcChecksum(&setDataRate[2], sizeof(setDataRate) - 4);

    // Generate the configuration string for Baud Rate
    byte setPortRate[] = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, settingsArrayPointer[3], settingsArrayPointer[4], settingsArrayPointer[5], 0x00, 0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    calcChecksum(&setPortRate[2], sizeof(setPortRate) - 4);

    byte setGLL[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x2B};
    byte setGSA[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x32};
    byte setGSV[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x39};
    byte setRMC[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x40};
    byte setVTG[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x46};

    delay(2500);

    while (gpsSetSuccess < 3)
    {
        PRINTLN("Setting Navigation Mode... ");

        sendUBX(&setNav[0], sizeof(setNav));     // Send UBX Packet
        gpsSetSuccess += getUBX_ACK(&setNav[2]); // Passes Class ID and Message ID to the ACK Receive function
        if (gpsSetSuccess == 5)
        {
            gpsSetSuccess -= 4;
            setBaud(settingsArrayPointer[4]);
            delay(1500);
            byte lowerPortRate[] = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x80, 0x25, 0x00, 0x00, 0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA2, 0xB5};
            sendUBX(lowerPortRate, sizeof(lowerPortRate));
            // gps_serial.begin(9600);
            delay(2000);
        }
        if (gpsSetSuccess == 6)
        {
            gpsSetSuccess -= 4;
        }
        if (gpsSetSuccess == 10)
        {
            gpsStatus[0] = true;
        }
    }
    if (gpsSetSuccess == 3)
    {
        PRINTLN("Navigation Mode - Failed!");
    }
    else
    {
        PRINTLN("Navigation Mode - Success");
    }

    gpsSetSuccess = 0;
    while (gpsSetSuccess < 3)
    {
        PRINTLN("Setting Data Update Rate... ");
        sendUBX(&setDataRate[0], sizeof(setDataRate)); // Send UBX Packet
        gpsSetSuccess += getUBX_ACK(&setDataRate[2]);  // Passes Class ID and Message ID to the ACK Receive function
        if (gpsSetSuccess == 10)
        {
            gpsStatus[1] = true;
        }
        if (gpsSetSuccess == 5 | gpsSetSuccess == 6)
        {
            gpsSetSuccess -= 4;
        }
    }
    if (gpsSetSuccess == 3)
    {
        PRINTLN("Data Update Rate - Failed!");
    }
    else
    {
        PRINTLN("Data Update Rate - Success");
    }

    gpsSetSuccess = 0;
    while (gpsSetSuccess < 3 && settingsArrayPointer[6] == 0x00)
    {
        PRINTLN("Deactivating NMEA GLL Messages ");
        sendUBX(setGLL, sizeof(setGLL));
        gpsSetSuccess += getUBX_ACK(&setGLL[2]);
        if (gpsSetSuccess == 10)
        {
            gpsStatus[2] = true;
        }
        if (gpsSetSuccess == 5 | gpsSetSuccess == 6)
        {
            gpsSetSuccess -= 4;
        }
    }
    if (gpsSetSuccess == 3)
    {
        PRINTLN("NMEA GLL Messages - Failed.");
    }
    else
    {
        PRINTLN("NMEA GLL Messages - Success");
    }

    gpsSetSuccess = 0;
    while (gpsSetSuccess < 3 && settingsArrayPointer[7] == 0x00)
    {
        PRINTLN("Deactivating NMEA GSA Messages ");
        sendUBX(setGSA, sizeof(setGSA));
        gpsSetSuccess += getUBX_ACK(&setGSA[2]);
        if (gpsSetSuccess == 10)
        {
            gpsStatus[3] = true;
        }
        if (gpsSetSuccess == 5 | gpsSetSuccess == 6)
        {
            gpsSetSuccess -= 4;
        }
    }
    if (gpsSetSuccess == 3)
    {
        PRINTLN("NMEA GSA Messages - Failed!");
    }
    else
    {
        PRINTLN("NMEA GSA Messages - Success");
    }
    gpsSetSuccess = 0;

    while (gpsSetSuccess < 3 && settingsArrayPointer[8] == 0x00)
    {
        PRINTLN("Deactivating NMEA GSV Messages ");
        sendUBX(setGSV, sizeof(setGSV));
        gpsSetSuccess += getUBX_ACK(&setGSV[2]);
        if (gpsSetSuccess == 10)
        {
            gpsStatus[4] = true;
        }
        if (gpsSetSuccess == 5 | gpsSetSuccess == 6)
        {
            gpsSetSuccess -= 4;
        }
    }
    if (gpsSetSuccess == 3)
    {
        PRINTLN("NMEA GSV Messages - Failed.");
    }
    else
    {
        PRINTLN("NMEA GSV Messages - Success");
    }
    gpsSetSuccess = 0;

    while (gpsSetSuccess < 3 && settingsArrayPointer[9] == 0x00)
    {
        PRINTLN("Deactivating NMEA RMC Messages ");
        sendUBX(setRMC, sizeof(setRMC));
        gpsSetSuccess += getUBX_ACK(&setRMC[2]);
        if (gpsSetSuccess == 10)
        {
            gpsStatus[5] = true;
        }
        if (gpsSetSuccess == 5 | gpsSetSuccess == 6)
        {
            gpsSetSuccess -= 4;
        }
    }
    if (gpsSetSuccess == 3)
    {
        PRINTLN("NMEA RMC Messages - Failed!");
    }
    else
    {
        PRINTLN("NMEA RMC Messages - Success");
    }
    gpsSetSuccess = 0;

    while (gpsSetSuccess < 3 && settingsArrayPointer[10] == 0x00)
    {
        PRINTLN("Deactivating NMEA VTG Messages ");
        sendUBX(setVTG, sizeof(setVTG));
        gpsSetSuccess += getUBX_ACK(&setVTG[2]);
        if (gpsSetSuccess == 10)
        {
            gpsStatus[6] = true;
        }
        if (gpsSetSuccess == 5 | gpsSetSuccess == 6)
        {
            gpsSetSuccess -= 4;
        }
    }
    if (gpsSetSuccess == 3)
    {
        PRINTLN("NMEA VTG Messages - Failed!");
    }
    else
    {
        PRINTLN("NMEA VTG Messages - Success");
    }

    gpsSetSuccess = 0;
    if (settingsArrayPointer[4] != 0x25)
    {
        PRINTLN("Setting Port Baud Rate... ");
        sendUBX(&setPortRate[0], sizeof(setPortRate));
        setBaud(settingsArrayPointer[4]);
        PRINTLN("Port Baud Rate - Success");
        delay(500);
    }
}

void calcChecksum(byte *checksumPayload, byte payloadSize)
{
    PRINTLN("Func: calcChecksum()");
    byte CK_A = 0, CK_B = 0;
    for (int i = 0; i < payloadSize; i++)
    {
        CK_A = CK_A + *checksumPayload;
        CK_B = CK_B + CK_A;
        checksumPayload++;
    }
    *checksumPayload = CK_A;
    checksumPayload++;
    *checksumPayload = CK_B;
}

void sendUBX(byte *UBXmsg, byte msgLength)
{
    PRINTLN("Func: SendUBX()");
    for (int i = 0; i < msgLength; i++)
    {
        gps_serial.write(UBXmsg[i]);
        gps_serial.flush();
    }
    gps_serial.println();
    gps_serial.flush();
}

byte getUBX_ACK(byte *msgID)
{
    PRINTLN("Func: getUBX_ACK()");
    byte CK_A = 0, CK_B = 0;
    byte incoming_char;
    boolean headerReceived = false;
    unsigned long ackWait = millis();
    byte ackPacket[10] = {0xB5, 0x62, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int i = 0;
    while (1)
    {
        if (gps_serial.available())
        {
            incoming_char = gps_serial.read();
            if (incoming_char == ackPacket[i])
            {
                i++;
            }
            else if (i > 2)
            {
                ackPacket[i] = incoming_char;
                i++;
            }
        }
        if (i > 9)
            break;
        if ((millis() - ackWait) > 1500)
        {
            PRINTLN("ACK Timeout");
            return 5;
        }
        if (i == 4 && ackPacket[3] == 0x00)
        {
            PRINTLN("NAK Received");
            return 1;
        }
    }

    for (i = 2; i < 8; i++)
    {
        CK_A = CK_A + ackPacket[i];
        CK_B = CK_B + CK_A;
    }
    if (msgID[0] == ackPacket[6] && msgID[1] == ackPacket[7] && CK_A == ackPacket[8] && CK_B == ackPacket[9])
    {
        PRINTLN("Success!");
        PRINT("ACK Received! ");
        printHex(ackPacket, sizeof(ackPacket));
        return 10;
    }
    else
    {
        PRINT("ACK Checksum Failure: ");
        printHex(ackPacket, sizeof(ackPacket));
        delay(1000);
        return 1;
    }
}

void printHex(uint8_t *data, uint8_t length) // prints 8-bit data in hex
{
    PRINTLN("Func: printHex()");
    char tmp[length * 2 + 1];
    byte first;
    int j = 0;
    for (byte i = 0; i < length; i++)
    {
        first = (data[i] >> 4) | 48;
        if (first > 57)
            tmp[j] = first + (byte)7;
        else
            tmp[j] = first;
        j++;

        first = (data[i] & 0x0F) | 48;
        if (first > 57)
            tmp[j] = first + (byte)7;
        else
            tmp[j] = first;
        j++;
    }
    tmp[length * 2] = 0;
    for (byte i = 0, j = 0; i < sizeof(tmp); i++)
    {
        PRINT(tmp[i]);
        if (j == 1)
        {
            PRINT(" ");
            j = 0;
        }
        else
            j++;
    }
    PRINTLN();
}

void setBaud(byte baudSetting)
{
    PRINTLN("Func: setBaud()");
    if (baudSetting == 0x12)
        gps_serial.begin(4800);
    if (baudSetting == 0x4B)
        gps_serial.begin(19200);
    if (baudSetting == 0x96)
        gps_serial.begin(38400);
    if (baudSetting == 0xE1)
        gps_serial.begin(57600);
    if (baudSetting == 0xC2)
        gps_serial.begin(115200);
    if (baudSetting == 0x84)
        gps_serial.begin(230400);
}

void gps_read()
{
    // PRINTLN("Func: gps_read()");
    uint32_t starttime = millis();
    while ((millis() - starttime) < 1000)
    {
        while (gps_serial.available() > 0)
        {
            // char c = gps_serial.read();
            // PRINT(c);
            // gps.encode(c);
            gps.encode(gps_serial.read());
        }
    }
}

void gps_info()
{
    // PRINTLN("Func: gps_info()");

    if (millis() > 5000 && gps.charsProcessed() < 10)
    {
        // PRINTLN(F("No GPS data received: check wiring"));
        return;
    }
    // if valid location then update
    if (gps.location.isValid())
    {
        gps_data_s.gps_sat = (uint8_t)gps.satellites.value();
        gps_data_s.gps_lat = gps.location.lat();
        gps_data_s.gps_lng = gps.location.lng();
        gps_data_s.gps_alt = (uint16_t)gps.altitude.meters();
        gps_data_s.gps_speed = (uint8_t)gps.speed.kmph();
        gps_data_s.gps_course = (uint16_t)gps.course.deg();

        // gps_date_time_g.gps_year = gps.date.year();
        gps_data_s.gps_date_time.gps_month = gps.date.month();
        gps_data_s.gps_date_time.gps_day = gps.date.day();
        gps_data_s.gps_date_time.gps_hour = gps.time.hour();
        gps_data_s.gps_date_time.gps_min = gps.time.minute();
        // gps_date_time_g.gps_sec = gps.time.second();
    }
    else
    {
        PRINTLN("Func: read_GPS_data(): GPS Invalid");
    }

#if DEVICE_MODE == BOARD_HELTEC
    if (gps.location.lat() != 0 && gps.location.lng() != 0)
    {
        gs_lat = gps.location.lat();
        gs_lng = gps.location.lng();
    }
#endif
}

#endif