/**
    @file TLE_Utils.ino
    @brief TLE utils file function for SATLLA0 GS.

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

#include <Sgp4.h>
#include "time.h"
#include <HTTPClient.h>
#include <NTPClient.h>

const unsigned long C = 299792458; // m / s
// Number of satellites to track.
const int timeZone = 2;                      // Your Time zone.
char TLE[500];                               // Variable to store satellite TLEs.
char satnames[NUM_SATS][30] = {"SATLLA-2B"}; //, "SATLLA-2B"}; //, "NORBI"}; //, "ISS"}  // Names of satellites.

// http://104.168.149.178/NORAD/elements/gp.php?CATNR=51014

int HTTP_PORT = 80;
String HTTP_METHOD = "GET"; // or POST
char HOST_NAME[] = "celestrak.org";
String PATH_NAME = "/NORAD/elements/gp.php?CATNR=";

int satID[NUM_SATS] = {51014};  //, 51024}; //, 46494}; //, "/satcat/tle.php?CATNR=25544"}; // URL of Celestrak TLEs for satellites (In same order as names).
int satAddress[NUM_SATS] = {0xB2};  //, 0xB2};
char TLE1[NUM_SATS][70] = {"1 51014U 22002AG  22264.11601180  .00012524  00000+0  61112-3 0  9998"}; //, "1 51024U 22002AS  22029.88687611  .00002566  00000-0  14849-3 0  9996"}; //, 1 46494U 20068J   22013.90788195  .00001890  00000-0  14075-3 0  9996"};
char TLE2[NUM_SATS][70] = {"2 51014  97.4895 330.6653 0013425  94.1949 266.0820 15.18353214 37942"}; //, "2 51024  97.5083  99.3155 0013424 176.1487 183.9850 15.13070333  2468"}; //, 2 46494  97.7124 315.5193 0019509  86.4048 273.9409 15.04009063 70964"};

#if TLE_ENABLE && WIFI_ENABLE

char info[10][100];

// Your latitude, longitude and altitude.
float myLat = gs_lat;
float myLong = gs_lng;
float myAlt = 670;

Sgp4 sat[NUM_SATS];
passinfo overpass[NUM_SATS];

int next_sat;
long passEnd;
int pass_status = 0;
char satname[] = " ";

int st_year, st_mon, st_day, st_hr, st_mnt, st_today;
int mx_year, mx_mon, mx_day, mx_hr, mx_mnt, mx_today;
int sp_year, sp_mon, sp_day, sp_hr, sp_mnt, sp_today;
double st_sec, mx_sec, sp_sec;

// long nextpassEpoch;
unsigned long upcoming_passes[NUM_SATS];
unsigned long timeNow = 0;
int h, m, s;

// Initialize the Ethernet client library
WiFiClient client;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
const long gmtOffset_sec = 3600 * 2;
const int daylightOffset_sec = 3600;
String text;

int next_sat_pass(unsigned long nextpassEpoch[NUM_SATS]);

int message_counter = 0;

double prev_lat = 0L;
double prev_lon = 0L;
unsigned long prev_time = 0UL;

void tle_setup()
{
    PRINTLN("Func: tle_setup()");

    // Init and get the time
    // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    // timeClient.setTimeOffset(gmtOffset_sec + daylightOffset_sec);
    timeClient.setUpdateInterval(86400);
    timeClient.begin();

    wifi_connect();
    timeClient.update();

    // Get Unix time
    timeNow = getTime();

    for (size_t satid = 0; satid < NUM_SATS; satid++)
    {
        sat[satid].site(myLat, myLong, myAlt); // set location latitude[°], longitude[°] and altitude[m]
    }

    // Get TLEs
    update_TLE();
    // Set next sat
    switch_next_sat();
}

void switch_next_sat()
{
    // Set next sat
    next_sat = find_next_pass();
    // Update Modem
    // switch_modem(next_sat); // Change radio to default

    PRINTLN("Next satellite: " + String(satnames[next_sat]));
}

void update_TLE()
{
    PRINTLN("Func: update_TLE()");
    // Get TLEs
    for (int i = 0; i < NUM_SATS; i++)
    {
        get_TLE(i);

        // Print obtained TLE in serial.
        PRINTLN("TLE set #: " + String(i));
        PRINT("TLE 1: ");
        for (int j = 0; j < 70; j++)
        {
            PRINT(TLE1[i][j]);
        }
        PRINTLN();
        PRINT("TLE 2: ");
        for (int j = 0; j < 70; j++)
        {
            PRINT(TLE2[i][j]);
        }
        PRINTLN();
    }
}

void update_TLE_from_TLG()
{
    PRINTLN("Func: update_TLE_from_TLG()");
    // Get TLEs
#if FIREBASE_ENABLE
    char t_buffer[140] = {0x00};
    for (int i = 0; i < NUM_SATS; i++)
    {
        uint8_t res = fb_read_TLE(t_buffer, i + 1);
        if (res > 0)
        {
            // Print obtained TLE in serial.
            PRINT("TLE 1: ");
            for (int j = 0; j < 70; j++)
            {
                TLE1[i][j] = t_buffer[j];
                PRINT(TLE1[i][j]);
            }
            PRINTLN();
            PRINT("TLE 2: ");
            for (int j = 0; j < 70; j++)
            {
                TLE2[i][j] = t_buffer[70 + j];
                PRINT(TLE2[i][j]);
            }
            PRINTLN();
        }
    }
#endif
}

void get_TLE(int satid)
{
    PRINTLN("Func: get_TLE()");
    memset(lrg_buffer_g, 0x00, sizeof(lrg_buffer_g));
    // connect to web server on port 80:
    if (client.connect(HOST_NAME, HTTP_PORT))
    {
        // if connected:
        Serial.println("Connected to server");
        Serial.println(HTTP_METHOD + " " + PATH_NAME + String(satID[satid]) + " HTTP/1.1");
        Serial.println("Host: " + String(HOST_NAME));
        // make a HTTP request:
        // send HTTP header
        client.println(HTTP_METHOD + " " + PATH_NAME + String(satID[satid]) + " HTTP/1.1");
        client.println("Host: " + String(HOST_NAME));
        client.println("Connection: close");
        client.println(); // end HTTP header

        int connectLoop = 0;
        int k = 0;
        while (client.connected())
        {
            // PRINTLN("client.connected()");
            while (client.available())
            {
                char c = client.read();
                PRINT(c);
                lrg_buffer_g[k++] = c; // store characters to string
                connectLoop = 0;
            }

            connectLoop++;
            delay(100);
            if (connectLoop > 100)
            {
                client.stop();
                break;
            }
        }
        Serial.println();
        Serial.println("disconnected");

        Serial.print("Total byte received:\t");
        Serial.println(k);

        if (k > 167)
        {
            // received data. find "sat name" and read TLE
            String data = String(lrg_buffer_g);
            int nl = data.indexOf('\n');
            while (nl != -1)
            {
                if (data[nl + 1] == '1')
                {
                    PRINT("TLE 1: ");
                    for (int j = 0; j < 70; j++)
                    {
                        TLE1[satid][j] = data[nl + 1 + j];
                        PRINT(TLE1[satid][j]);
                    }
                    PRINTLN();
                }
                else if (data[nl + 1] == '2')
                {
                    PRINT("TLE 2: ");
                    for (int j = 0; j < 70; j++)
                    {
                        TLE2[satid][j] = data[nl + 1 + j];
                        PRINT(TLE2[satid][j]);
                    }
                    PRINTLN();
                }
                nl = data.indexOf('\n', nl + 1);
            }
        }
        else
        { // if not connected:
            Serial.println("Failed to obtain TLE");
        }
        PRINTLN("Func: get_TLE(): Done");
    }
}

void tle_loop()
{
    PRINTLN("Func: tle_loop()");

    wifi_connect();

    timeClient.update();

    unsigned long get_time_now = getTime(); // Update time.
    if (get_time_now)
    {
        timeNow = get_time_now; // Update time.
    }
    // timeNow += 73*60;

    int year, mon, day, hr, mnt, today;
    double sec;
    sat[next_sat].findsat(timeNow);
    invjday(sat[next_sat].satJd, timeZone, true, year, mon, day, hr, mnt, sec);

    unsigned long nextpassEpoch = upcoming_passes[next_sat] - timeNow;
    seconds_to_HMS(nextpassEpoch, h, m, s);
    if (upcoming_passes[next_sat] < timeNow)
    {
        h = 0;
        m = 0;
        s = 0;
    }

    // Calculate doppler shift
    rv_g = range_velocity(prev_lat, prev_lon, sat[next_sat].satLat, sat[next_sat].satLon, timeNow - prev_time);
    // PRINT("Doppler Shift:\t");
    // PRINTLN(doppler_g);
    // PRINT("New Freq:\t");
    // PRINTLN(LORA_433_BAND + doppler_g);
    prev_lat = sat[next_sat].satLat;
    prev_lon = sat[next_sat].satLon;
    prev_time = timeNow;
    // PRINT("Range Velocity:\t");
    // PRINTLN(rv);
    doppler_g = dopplercalc(rv_g, LORA_433_BAND) * -1; // if approach, then reduce from freq. else increase.

    sprintf(info[0], "Local time: %d/%d/%d - %d:%0.2d:%0.2d", day, mon, year, hr, mnt, (int)sec);
    sprintf(info[1], "Next SAT is %s in: %d:%0.2d:%0.2d", satnames[next_sat], h, m, s);
    sprintf(info[2], "Current: az: %.2f°, elv: %.2f°, dst: %.2f km", sat[next_sat].satAz, sat[next_sat].satEl, sat[next_sat].satDist);
    sprintf(info[3], "lat/lon: [%f,%f], alt: %.0f km", sat[next_sat].satLat, sat[next_sat].satLon, sat[next_sat].satAlt);
    sprintf(info[4], "Doppler: %d, Freq: %.0f, Velociry: %d", doppler_g, LORA_433_BAND + doppler_g, rv_g);
    sprintf(info[5], "Predict: az: %.2f°, max: %.2f°, stop: %.2f°", overpass[next_sat].azstart, overpass[next_sat].azmax, overpass[next_sat].azstop);

    for (size_t i = 0; i < 6; i++)
    {
        PRINTLN(String(info[i]));
    }

#if LCD_SSD
    text = "";
    for (size_t i = 1; i < 4; i++) // skip local time
    {
        text += String(info[i]) + ", ";
    }
    lcd.clear();
    lcd.drawStringMaxWidth(0, 0, 126, text);
    lcd.display();
#endif

    if (nextpassEpoch < 180 && nextpassEpoch + 5 > 0)
    {
        PRINTLN("Status: Pre pass");
        pre_pass(next_sat);
        if (!publish_to_tlg)
        {
            publish_sat_to_tlg(-1, satlla0_bot);
            if (tlg_ch_enable)
            {
                publish_sat_to_tlg(-1, satlla0_channel);
            }
            publish_to_tlg = true;
        }
    }
    else if (sat[next_sat].satVis != -2)
    {
        PRINTLN("Status: In pass");
        tle_read_threshold = SEC_1; // if in-pass, repeat every sec.
        if (doppler_enable > 0)
        {
            in_pass(next_sat, doppler_g);
        }
        else
        {
            in_pass(next_sat, 0L);
        }
#if TRK_ENABLE
        // trk_in_pass(sat[next_sat].satAz, sat[next_sat].satEl);
#endif
    }
    else if (timeNow - passEnd < 120)
    {
        PRINTLN("Status: Post-pass");
        post_pass(next_sat);
    }
    else if (sat[next_sat].satVis == -2)
    {
        PRINTLN("Status: Standby");
        tle_read_threshold = SECS_10; // repeat every 10 secs.
        // standby();
    }
    PRINTLN();
}

void get_current(int satid, sat_current_info *sat_current)
{
    PRINTLN("Func: get_current()");
    int year, mon, day, hr, mnt, today;
    double sec;

    if (satid < 0)
    {
        satid = next_sat;
    }
    PRINT("SAT:\t");
    PRINTLN(satid);

    wifi_connect();

    timeClient.update();

    unsigned long get_time_now = getTime(); // Update time.
    if (get_time_now)
    {
        timeNow = get_time_now; // Update time.
    }
    sat[satid].findsat(timeNow);
    invjday(sat[satid].satJd, timeZone, true, year, mon, day, hr, mnt, sec);

    sat_current->year = year;
    sat_current->mon = mon;
    sat_current->day = day;
    sat_current->hr = hr;
    sat_current->mnt = mnt;
    sat_current->sec = (int)sec;

    sat_current->az = sat[satid].satAz;
    sat_current->ele = sat[satid].satEl;
    sat_current->dist = sat[satid].satDist;

    sat_current->lat = sat[satid].satLat;
    sat_current->lon = sat[satid].satLon;
    sat_current->alt = sat[satid].satAlt;
}

void get_pass(int satid, sat_current_info *sat_current, int pass)
{
    PRINTLN("Func: get_pass()");
    int year, mon, day, hr, mnt, today;
    double sec;

    if (satid < 0)
    {
        satid = next_sat;
    }
    PRINT("SAT:\t");
    PRINTLN(satid);
    PRINT("Pass:\t");
    PRINTLN(pass);

    wifi_connect();

    timeClient.update();

    unsigned long get_time_now = getTime(); // Update time.
    if (get_time_now)
    {
        timeNow = get_time_now; // Update time.
    }
    unsigned long nextpassEpoch = upcoming_passes[satid];

    if (nextpassEpoch <= timeNow)
    {
        nextpassEpoch = timeNow;
    }

    // sat[satid].findsat(nextpassEpoch);
    // sat[satid].findsat(overpass[satid].jdstart);
    switch (pass)
    {
    case 1:
        sat[satid].findsat(overpass[satid].jdmax);
        break;

    case 2:
        sat[satid].findsat(overpass[satid].jdstop);
        break;

    default:
        sat[satid].findsat(overpass[satid].jdstart);
        break;
    }

    invjday(sat[satid].satJd, timeZone, true, year, mon, day, hr, mnt, sec);

    sat_current->year = year;
    sat_current->mon = mon;
    sat_current->day = day;
    sat_current->hr = hr;
    sat_current->mnt = mnt;
    sat_current->sec = (int)sec;

    sat_current->az = sat[satid].satAz;
    sat_current->ele = sat[satid].satEl;
    sat_current->dist = sat[satid].satDist;

    sat_current->lat = sat[satid].satLat;
    sat_current->lon = sat[satid].satLon;
    sat_current->alt = sat[satid].satAlt;
}

void pre_pass(int sat_id)
{
    PRINTLN("Func: pre_pass()");
    // Change radio to coming SAT
    // if (!switch_modem_override)
    // {
    //     switch_modem(sat_id);
    // }
}

void in_pass(int sat_id, long doppler)
{
    passEnd = timeNow;
    pass_status = 1;

    modem_data_t modem;
    memcpy(modem.packet, modem_lora_433_g.packet, MODEM_DATA_LENGTH);
    modem.modem_frequency += doppler;

    setup_lora_433(&modem);

    if (command_pending_flag && destination_g != 0x00)
    {
        cmd_send(&command_data_g, destination_g, lora_433);
        message_counter++;
        if (message_counter > 50)
        {
            command_pending_flag = false;
            message_counter = 0;
        }
    }
}

void post_pass(int sat_id)
{
    PRINTLN("Func: post_pass()");

    Serial.println("Post pass time left: " + String(passEnd + 120 - timeNow));

    if (pass_status == 1 && timeNow - passEnd > 100)
    {
        setup_lora_433_default(); // TODO: should be part of the switch_modem
        switch_modem_override = false;
        switch_next_sat();
        pass_status = 0;
    }
    publish_to_tlg = false;

    if (command_pending_flag)
    {
        command_pending_flag = false;
    }
}

int find_next_pass()
{
    PRINTLN("Func: find_next_pass()");

    int satid = 0;
    unsigned long time = 0L;
    unsigned long next_predict = 0;
    for (int satid = 0; satid < NUM_SATS; satid++)
    {
        PRINT("satname: ");
        PRINTLN(satname);
        PRINT("satid: ");
        PRINTLN(satid);

        sat[satid].init(satname, TLE1[satid], TLE2[satid]);
        sat[satid].findsat(timeNow);
        time = timeNow;
        for (int j = 0; j < 10; j++)
        {
            next_predict = predict(satid, 1, time);
            if (next_predict > timeNow)
            {
                upcoming_passes[satid] = next_predict;
                break;
            }
            else
            {
                time += (120 * j);
            }
        }
        seconds_to_HMS(upcoming_passes[satid] - timeNow, h, m, s);
        sprintf(buffer_g, "Next SAT is %s in: %d:%0.2d:%0.2d", satnames[satid], h, m, s);
        PRINTLN(buffer_g);
    }
    satid = next_sat_pass(upcoming_passes);

    return satid;
}

int next_sat_pass(unsigned long nextpassEpoch[NUM_SATS])
{
    PRINTLN("Func: next_sat_pass()");

    unsigned long pass_epoch = nextpassEpoch[0];
    if (nextpassEpoch[0] > timeNow)
    {
        pass_epoch -= timeNow;
    }

    uint8_t next_sat = 0;
    for (int satid = 1; satid < NUM_SATS; satid++)
    {
        if ((nextpassEpoch[satid] - timeNow) < pass_epoch)
        {
            next_sat = satid;
        }
    }
    return next_sat;
}

// Adapted from sgp4 library
// Predicts time of next pass and start azimuth for satellites
unsigned long predict(int satid, int many, unsigned long time_now)
{
    PRINTLN("Func: predict()");
    unsigned long nextpassEpoch;
    // passinfo overpass;                       //structure to store overpass info
    sat[satid].initpredpoint(time_now, 0.0); // finds the startpoint
    PRINT("time_now:\t");
    PRINTLN(time_now);

    bool error;
    for (int i = 0; i < many; i++)
    {
        error = sat[satid].nextpass(&overpass[satid], 20); // search for the next overpass, if there are more than 20 maximums below the horizon it returns false
        delay(1);

        if (error == 1)
        { // no error, prints overpass information
            nextpassEpoch = (overpass[satid].jdstart - 2440587.5) * 86400;
            PRINTLN("Predict: " + String(nextpassEpoch));
            PRINTLN("time_now: " + String(time_now));

            invjday(overpass[satid].jdstart, timeZone, true, st_year, st_mon, st_day, st_hr, st_mnt, st_sec); // Convert Julian date to print in serial.
            PRINTLN("Next pass for SAT: " + String(satnames[satid]) + " in: " + String(nextpassEpoch - time_now) + ", " + String(st_hr) + ':' + String(st_mnt) + ':' + String(st_sec));
            PRINTLN("Start: az: " + String(overpass[satid].azstart) + "° " + String(st_hr) + ':' + String(st_mnt) + ':' + String(st_sec));

            invjday(overpass[satid].jdmax, timeZone, true, mx_year, mx_mon, mx_day, mx_hr, mx_mnt, mx_sec); // Convert Julian date to print in serial.
            PRINTLN("Max: elev: " + String(overpass[satid].maxelevation) + "° " + String(mx_hr) + ':' + String(mx_mnt) + ':' + String(mx_sec));

            invjday(overpass[satid].jdstop, timeZone, true, sp_year, sp_mon, sp_day, sp_hr, sp_mnt, sp_sec); // Convert Julian date to print in serial.
            PRINTLN("Stop: az: " + String(overpass[satid].azstop) + "° " + String(sp_hr) + ':' + String(sp_mnt) + ':' + String(sp_sec));
        }
        else
        {
            PRINT("Prediction error:\t");
            PRINTLN(error);
        }
        delay(0);
    }
    return nextpassEpoch;
}

void publish_sat_to_tlg(int8_t satid, int64_t sender_id)
{
    PRINTLN("Func: publish_sat_to_tlg()");
    if (satid < 0)
    {
        satid = next_sat;
    }

    sat_current_info sat_current;
    get_current(satid, &sat_current);

    sat_current_info sat_pass;
    get_pass(satid, &sat_pass, 0);

    unsigned long nextpassEpoch = upcoming_passes[satid];
    seconds_to_HMS(nextpassEpoch - timeNow, h, m, s);
    if (nextpassEpoch < timeNow)
    {
        h = 0;
        m = 0;
        s = 0;
    }

    sprintf(info[0], "\U0001F4ED %s", String(local_address, HEX));
    sprintf(info[1], "\U0001F551 %d/%d/%d - %d:%0.2d:%0.2d", sat_current.day, sat_current.mon, sat_current.year, sat_current.hr, sat_current.mnt, sat_current.sec);
    sprintf(info[2], "\U0001F6F0 <b>%s</b> in %d:%0.2d:%0.2d", satnames[satid], h, m, s);
    sprintf(info[3], "Start: (%d:%0.2d:%0.2d)\n\U0001F7E2 %.2f°, \U000026AA %.2f°, \U0001F534 %.2f°", sat_pass.hr, sat_pass.mnt, sat_pass.sec, overpass[satid].azstart, overpass[satid].azmax, overpass[satid].azstop);

    get_pass(satid, &sat_pass, 1);
    sprintf(info[4], "Max: (%d:%0.2d:%0.2d)\n\U0001F4D0 %.2f°, \U0001F4CF %.2f°, \U00002194 %.0f km", sat_pass.hr, sat_pass.mnt, sat_pass.sec, sat_pass.az, sat_pass.ele, sat_pass.dist);

    sprintf(info[5], "Doppler:\n\U0001F50A %d Hz, \U0001F4C8 %.0f Hz, \U0001F680 %d m/s", doppler_g, LORA_433_BAND + doppler_g, rv_g);

    sprintf(info[6], "Current:\n\U0001F4D0 %.2f°, \U0001F4CF %.2f°, \U00002194 %.0f km", sat_current.az, sat_current.ele, sat_current.dist);
    sprintf(info[7], "\U0001F5FA <a href='https://www.google.com/maps/place/%f,%f'>%.3f,%.3f</a>, Alt: %.0f km", sat_current.lat, sat_current.lon, sat_current.lat, sat_current.lon, sat_current.alt);

    text = "";
    for (size_t i = 0; i < 8; i++)
    {
        text += String(info[i]) + "\n";
    }
#if TELEGRAM_ENABLE
    publish_to_tlg_message(sender_id, text, 2);
#endif
}

void seconds_to_HMS(const uint32_t seconds, int &h, int &m, int &s)
{
    PRINTLN("Func: seconds_to_HMS()");
    uint32_t t = seconds;

    s = t % 60;

    t = (t - s) / 60;
    m = t % 60;

    t = (t - m) / 60;
    h = t;
}

// Function that gets current epoch time
unsigned long getTime()
{
    PRINTLN("Func: getTime()");

    unsigned long now = timeClient.getEpochTime();
    PRINTLN("Time now: " + String(now));

    return now;
}

// Function that gets current epoch time
String getFormattedTime()
{
    PRINTLN("Func: getFormattedTime()");

    String now = timeClient.getFormattedTime();
    PRINTLN("Time now: " + String(now));

    return now;
}

// # NOTE FOR DOPPLER CALCULATION:
// # Negative velocities are towards you,
// # Positive velocities are away from you.

long range_velocity(double lat1, double lon1, double lat2, double lon2, unsigned long time)
{
    PRINTLN("Func: range_velocity()");
    // PRINT("Time:\t");
    // PRINTLN(time);
    // current - prev: if approach then - else +
    double distance_m1 = TinyGPSPlus::distanceBetween(gs_lat, gs_lng, lat1, lon1); // prev
    double distance_m2 = TinyGPSPlus::distanceBetween(gs_lat, gs_lng, lat2, lon2); // current
    long rv = (distance_m2 - distance_m1) / (time + 3);                            // current - Prev. Add 3 secd for asjustment
    // PRINT("rv:\t");
    // PRINTLN(rv);
    return rv;
}

long dopplercalc(long rv, long long freq)
{
    PRINTLN("Func: dopplercalc()");
    return (rv * freq) / C;
}

void get_tle_for_sat(int satid, char *buffer)
{
    memset(buffer, 0x00, 140);
    memcpy(buffer, TLE1[satid], 70);
    memcpy(buffer + 70, TLE2[satid], 70);
}

#endif
