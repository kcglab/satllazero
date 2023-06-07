/**
    @file setting.h
    @brief Header file for SATLLA0 GS.

    This file contains settings and constant for the SATLLA0 GS.

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

#include <SPI.h>
#include <RingBufCPP.h>
#include <TinyGPS++.h>
#include <ArduinoJson.h>
#include <Wire.h>

#define LOCAL_ADDRESS 0xF0
byte local_address = LOCAL_ADDRESS; // default gs address

const char DATE_[] = (__DATE__ "");
const char TIME_[] = (__TIME__ "");
char firmware_version[14 + 1] = {0};

#define BOARD_HELTEC 0
#define BOARD_TEENSY 1
#define BOARD_T_BEAM 2
#define BOARD_T_3V16 3

#define DEVCIE BOARD_HELTEC // 0 : HELTEC, 1: Teensy, 2: TTGO T-BEAM, 3: TTGO T_3 V1.6

#if (DEVCIE == BOARD_HELTEC)    // Heltec
#define GPS_ENABLE 0            // 0=Off, 1=On, 2 : Serial
#define LCD_SSD 0               // 0=Off, 1=On
#define AXP20_ENABLE 0          // 0=Off, 1=On
#define LORA_E22_ENABLE 0       // 0=Off, 1=On
#define LORA_SX126X_ENABLE 0    // 0=Off, 1=On
#define LORA_SX1262_ENABLE 0    // 0=Off, 1=On
#define LORA_SX1278_ENABLE 0    // 0=Off, 1=On
#define LORA_SX128X_ENABLE 1    // 0=Off, 1=On <-
#define LORA_SX1280_ENABLE 0    // 0=Off, 1=On
#define LORA_RL_SX1278_ENABLE 1 // 0=Off, 1=On <-
#define EEPROM_ENABLE 1         // 0=Off, 1=On
#define WIFI_ENABLE 0           // 0=Off, 1=On
#define DRIVE_ENABLE 0          // 0=Off, 1=On
#define FIREBASE_ENABLE 0       // 0=Off, 1=On
#define OTA_ENABLE 0            // 0=Off, 1=On
#define TELEGRAM_ENABLE 0       // 0=Off, 1=On
#define TELEGRAM_BOT_ENABLE 0   // 0=Off, 1=On
#define TELEGRAM_CH_ENABLE 0    // 0=Off, 1=On
#define TLE_ENABLE 0            // 0=Off, 1=On
#elif (DEVICE == BOARD_TEENSY)  // Teensy
#define GPS_ENABLE 0            // 0=Off, 1=On, 2 : Serial
#define LCD_SSD 0               // 0=Off, 1=On
#define AXP20_ENABLE 0          // 0=Off, 1=On
#define LORA_SX126X_ENABLE 0    // 0=Off, 1=On
#define LORA_SX1262_ENABLE 0    // 0=Off, 1=On
#define LORA_SX1278_ENABLE 0    // 0=Off, 1=On
#define LORA_SX128X_ENABLE 1    // 0=Off, 1=On
#define LORA_SX1280_ENABLE 0    // 0=Off, 1=On
#define LORA_RL_SX1278_ENABLE 1 // 0=Off, 1=On <-
#define EEPROM_ENABLE 1         // 0=Off, 1=On
#define WIFI_ENABLE 0           // 0=Off, 1=On
#define DRIVE_ENABLE 0          // 0=Off, 1=On
#define FIREBASE_ENABLE 1       // 0=Off, 1=On
#define OTA_ENABLE 1            // 0=Off, 1=On
#define TELEGRAM_ENABLE 1       // 0=Off, 1=On
#define TELEGRAM_CH_ENABLE 0    // 0=Off, 1=On
#define TLE_ENABLE 0            // 0=Off, 1=On
#elif (DEVCIE == BOARD_T_BEAM)  // TTGO T-BEAM
#define TTGO_VER 1              // 0 : v07, 1 : v1.1 - Control the GPS pins
#define GPS_ENABLE 1            // 0=Off, 1=On, 2 : Serial
#define AXP20_ENABLE 1          // 0=Off, 1=On
#define LCD_SSD 0               // 0=Off, 1=On
#define LORA_SX126X_ENABLE 0    // 0=Off, 1=On
#define LORA_SX1262_ENABLE 0    // 0=Off, 1=On
#define LORA_SX1278_ENABLE 0    // 0=Off, 1=On
#define LORA_SX128X_ENABLE 1    // 0=Off, 1=On
#define LORA_SX1280_ENABLE 0    // 0=Off, 1=On
#define LORA_RL_SX1278_ENABLE 1 // 0=Off, 1=On <-
#define EEPROM_ENABLE 1         // 0=Off, 1=On
#define WIFI_ENABLE 1           // 0=Off, 1=On
#define DRIVE_ENABLE 0          // 0=Off, 1=On
#define FIREBASE_ENABLE 1       // 0=Off, 1=On
#define OTA_ENABLE 1            // 0=Off, 1=On
#define TELEGRAM_ENABLE 1       // 0=Off, 1=On
#define TELEGRAM_BOT_ENABLE 0   // 0=Off, 1=On
#define TELEGRAM_CH_ENABLE 0    // 0=Off, 1=On
#define TLE_ENABLE 1            // 0=Off, 1=On
#else                           // BOARD_T_3V16                        // TTGO T3_V1.6
#define GPS_ENABLE 0            // 0=Off, 1=On, 2 : Serial
#define AXP20_ENABLE 0          // 0=Off, 1=On
#define LCD_SSD 1               // 0=Off, 1=On
#define LORA_SX126X_ENABLE 0    // 0=Off, 1=On
#define LORA_SX1262_ENABLE 0    // 0=Off, 1=On
#define LORA_SX1278_ENABLE 0    // 0=Off, 1=On
#define LORA_SX128X_ENABLE 0    // 0=Off, 1=On
#define LORA_SX1280_ENABLE 0    // 0=Off, 1=On
#define LORA_RL_SX1278_ENABLE 1 // 0=Off, 1=On <-
#define EEPROM_ENABLE 1         // 0=Off, 1=On
#define WIFI_ENABLE 1           // 0=Off, 1=On
#define DRIVE_ENABLE 0          // 0=Off, 1=On
#define FIREBASE_ENABLE 0       // 0=Off, 1=On
#define OTA_ENABLE 0            // 0=Off, 1=On
#define TELEGRAM_ENABLE 1       // 0=Off, 1=On
#define TELEGRAM_CH_ENABLE 0    // 0=Off, 1=On
#define TLE_ENABLE 1            // 0=Off, 1=On
#endif

#define RF_433_ENABLE (LORA_RL_SX1278_ENABLE | LORA_SX126X_ENABLE | LORA_SX1262_ENABLE | LORA_SX1278_ENABLE)
#define RF_24_ENABLE (LORA_SX128X_ENABLE | LORA_SX1280_ENABLE)

#include "debug.h"
#include "data_struct.h"
#include "global_define.h"

#if (DEVCIE == BOARD_HELTEC) // Heltec
#define LED1 25
#define NSS 12 // GPIO18 -- SX1278's CS
#define OLED_R 16
#define OLED_SDA 4
#define OLED_SCL 15
#elif (DEVICE == BOARD_TEENSY) // Teensy
#define LED1 14
#else // TTGO T-BEAM
#define LED1 25
#define SCK 5   // GPIO5  -- SX1278's SCK
#define MISO 19 // GPIO19 -- SX1278's MISnO
#define MOSI 27 // GPIO27 -- SX1278's MOSI
#define NSS 18  // GPIO18 -- SX1278's CS
#define RST 14  // GPIO14 -- SX1278's RESET
#define DIO 26  // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define OLED_R 16
#define OLED_SDA 21
#define OLED_SCL 22
#define SERVO_PIN 23 // Servo Signal Pin
#endif

#define NUM_SATS 1
#define BTN_PRG_PIN 33

#if LCD_SSD
#include "SSD1306.h"
#include "images.h"
SSD1306 lcd(0x3c, OLED_SDA, OLED_SCL);
#endif

StaticJsonDocument<1024> doc;

bool ENABLE_GOOGLE_DRIVE = false;
bool PROD = false; // if set for real data then true. Otherwise, under testing.
bool REPLY_ACK = false;

/* ============ */
/* Constants    */
/* ============ */

#define TS_LOOP_RUNTIME SECS_20 // 20 secs
#define POST_TO_GOOGLE_THRESHOLD MINUTE_1

// LoRa Buffer
#define LORA_RX_TX_BYTE 255

// Ring Buffer Size
#define RX_RBUFF_MAX_NUM_ELEMENTS 40
// #define TX_RBUFF_MAX_NUM_ELEMENTS 200

// buffer_g
#define BUFFER_SIZE_MAX 255
#define LRG_BUFFER_SIZE_MAX 4096

#define CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
#define CPU_RESTART_VAL 0x5FA0004
#define CPU_RESTART (*CPU_RESTART_ADDR = CPU_RESTART_VAL);

#define CONST_10 10
#define CONST_100 100
#define CONST_1000 1000

/* ============= */
/* LoRa 433      */
/* ============= */
#if RF_433_ENABLE

#if (DEVCIE == BOARD_HELTEC) // Heltech
#define LORA_433_NSS_PIN 18
#define LORA_433_RST_PIN 14
#define LORA_433_DIO0_PIN 26
#elif (DEVCIE == BOARD_TEENSY) // Teensy
#define LORA_433_NSS_PIN 4
#define LORA_433_RST_PIN 3
#define LORA_433_DIO0_PIN 2
#elif (DEVCIE == BOARD_T_BEAM) // TTGO T-BEAM
#define LORA_433_NSS_PIN 18
#define LORA_433_RST_PIN 23
#define LORA_433_DIO0_PIN 26
#elif (DEVCIE == BOARD_T_3V16) // TTGO T3_V1.6
#define LORA_433_NSS_PIN 18
#define LORA_433_RST_PIN 14
#define LORA_433_DIO0_PIN 26
#endif

#define LORA_433_CR_4_5 5
#define LORA_433_SF10 10
#define LORA_433_BW_062 6 // table

#define LORA_433_CRC_ON 0x01    // Packet CRC is activated
#define LORA_433_CRC_OFF 0x0    // Packet CRC is activated
#define LORA_433_LDRO_ON 0x01   // 0x00 OFF, 0x01 ON, 0x02 AUTO
#define LORA_433_LDRO_OFF 0x00  // 0x00 OFF, 0x01 ON, 0x02 AUTO
#define LORA_433_LDRO_AUTO 0x02 // 0x00 OFF, 0x01 ON, 0x02 AUTO

#define LORA_433_BAND 43400000 // Band
#define LORA_433_OFFSET 0      // Offset
#define LORA_433_PABOOST true  // Boost

#define LORA_433_BW LORA_433_BW_062
#define LORA_433_SF LORA_433_SF10
#define LORA_433_CR LORA_433_CR_4_5
#define LORA_433_TX_RX_POWER 20
#define LORA_433_SW 0x12                // Syncword
#define LORA_433_PL 8                   // preambleLength
#define LORA_433_TCXO -1                // tcxoVoltage
#define LORA_433_LDRO LORA_433_LDRO_OFF // 0x00 OFF, 0x01 ON, 0x02 AUTO
#define LORA_433_CRC LORA_433_CRC_ON    // 0x00 OFF, 0x01 ON

#define FSK_433_DEVIATION 5.0       // kHz single-sideband
#define FSK_433_BW 39.0             // 62.5
#define FSK_433_BR 9.6              // kbps nominal
#define FSK_433_CR 140.0            // mA
#define FSK_433_TX_RX_POWER 20      // Tx Power
#define FSK_433_DS 0x02             // RADIOLIB_SHAPING_0_5 GFSK filter BT
#define FSK_433_SW 0x12             // Syncword
#define FSK_433_PL 16               // bits
#define FSK_433_CRC LORA_433_CRC_ON // 0x00 OFF, 0x01 ON

#endif

/* ============= */
/* LoRa 2.4      */
/* ============= */

#if RF_24_ENABLE

#define LORA_24_CR_4_5 0x01
#define LORA_24_SF10 0xA0
#define LORA_24_BW_0800 0x18 // actually 812500hz

#if (DEVCIE == BOARD_HELTEC) // Heltech
#define LORA_24_NSS_PIN 12
#define LORA_24_BSY_PIN 38
#define LORA_24_RST_PIN 17
#define LORA_24_DIO1_PIN 39
#define LORA_24_TCXO_PIN -1
#define LORA_24_TX_EN_PIN -1
#define LORA_24_RX_EN_PIN -1
#elif (DEVCIE == BOARD_TEENSY) // Teensy
#define LORA_24_NSS_PIN 12
#define LORA_24_BSY_PIN 38
#define LORA_24_RST_PIN 17
#define LORA_24_DIO1_PIN 39
#define LORA_24_TCXO_PIN -1
#define LORA_24_TX_EN_PIN -1
#define LORA_24_RX_EN_PIN -1
#elif (DEVCIE == BOARD_T_BEAM) // TTGO T-BEAM
#define LORA_24_NSS_PIN 2
#define LORA_24_BSY_PIN 35
#define LORA_24_RST_PIN 32
#define LORA_24_DIO1_PIN 23
#define LORA_24_TCXO_PIN 25
#define LORA_24_TX_EN_PIN -1
#define LORA_24_RX_EN_PIN -1
#elif (DEVCIE == BOARD_T_3V16) // TTGO T3_V1.6
#define LORA_24_NSS_PIN 12
#define LORA_24_BSY_PIN 38
#define LORA_24_RST_PIN 17
#define LORA_24_DIO1_PIN 39
#define LORA_24_TCXO_PIN -1
#define LORA_24_TX_EN_PIN -1
#define LORA_24_RX_EN_PIN -1
#endif

#define LORA_24_BAND 2400000000 // frequency of transmissions
#define LORA_24_OFFSET 0        // offset frequency for calibration purposes
#define LORA_24_BW LORA_24_BW_0800
#define LORA_24_SF LORA_24_SF10
#define LORA_24_CR LORA_24_CR_4_5
#define LORA_24_TX_RX_POWER 13
#define LORA_24_SW 0x00000012
#define LORA_24_LDRO 0x00        // 0x00 USE_LDO
#define LORA_24_CRC LORA_CRC_OFF // 0x00 OFF, 0x20 ON,
#define LORA_24_PL 12            // bits

// Ranging
#define LORA_24_BW_RNG LORA_BW_0400 // LoRa lora_24_bw for Ranging
#define LORA_24_SF_RNG LORA_SF10    // LoRa spreading factor  for Ranging
#define LORA_24_CR_RNG LORA_CR_4_5  // LoRa coding rate for Ranging

#define LORA_24_RNG_ADDR 0x0A               // Ranging address the receiver accepts, must match setting in requestor
#define LORA_24_RNG_CALIB_SF10BW400 12200   // calibration value for ranging, SF10, BW400, see table "Introduction to Ranging"
#define LORA_24_WAIT_TIME_MS 5 * SEC_1      // wait this long in mS for packet before assuming timeout
#define LORA_24_RNG_RX_TIMEOUT_MS 5 * SEC_1 // SX1280 ranging RX timeout in mS
#define LORA_24_RNG_TX_TIMEOUT_MS 5 * SEC_1 // ranging TX timeout in mS
#define LORA_24_RNG_COUNT 6                 // number of ranging requests at each attempt
#define LORA_24_RNG_DELAY_MS 275            // mS delay between packets, minimum 130mS for BW400, SF10 ranging packets

#endif

/* Heltec Pin Out */
/*
 * MOSI 27
 * MISO 19
 * CS   18
 * SCK   5
 *
 */

// Main LED
#define MODE_LED_PIN 29

/* ============ */
/* Globals      */
/* ============ */
modem_e current_modem = modem_none;
bool gfsk_beacon = true;
beacon_type_e b_type;

const bool reset_device = true;           // enable device reset between ranging requests.
const int8_t RangingTXPower = 10;         // Transmit power
const float distance_adjustment = 0.8967; // adjustment to calculated distance
bool ranging_execute_start = false;
bool ranging_request_ack = false;

bool button_pressed_flag = false;

// SATLLA-2 Key
char key[KEY_LENGTH] = {'A', 'A', 'A'};
const char KEY[KEY_LENGTH] = {'A', 'A', 'A'};

const char SATLLA0_CALL_SIGN = 0xA0;
const char GS0_CALL_SIGN = 0xE0;

uint16_t tx_counter = 0;     // TX Counter
uint16_t rx_counter = 0;     // RX Counter
uint16_t errors_counter = 0; // Counter of errors
uint8_t rx_sat_counter = 0;  // Counter of RX not GS origin

RingBufCPP<ringbuffer_data_t, RX_RBUFF_MAX_NUM_ELEMENTS> RXbuffer;

const double MYLOC_LAT = 32.10544342149793;
const double MYLOC_LNG = 35.211391207993;
double gs_lat = MYLOC_LAT;
double gs_lng = MYLOC_LNG;
uint8_t gs_gps_enable = GPS_ENABLE;
uint8_t gs_drive_enable = DRIVE_ENABLE;

uint8_t doppler_enable = 1; // Doppler enable
long doppler_g = 0L;        // Doppler global parameter
long rv_g = 0L;             // Doppler global parameter

uint8_t destination_g = 0x00; // Global dest address

// Data Struct
lora_data_t lora_data_g; // Beacon
lora_data_t lora_data_b; // RXBuffer
lora_data_t prnt_msg_g;  // Print Message
lora_data_t send_command_g;
lora_data_t reply_ack_g;
lora_data_t rng_request_g;
ringbuffer_data_t ringbuffer_data_g;
ringbuffer_data_t ringbuffer_data_b;
shrt_bcn_data_t prnt_short_beacon_g; // Pring Short Beacon
shrt_bcn_data_t short_beacon_g;
gs_setting_data_t gs_setting_data_g;
ld_meta_data_t ld_meta_data_g; // Meta Data
ld_meta_data_t ld_meta_data_b; // Meta Data for beacons
info_data_t info_data_g;       // System Info

sns_data_t sns_data_g;         // Sensors
gps_data_t gps_data_g;         // GPS
gps_data_t gps_data_s;         // GPS
date_time_t gps_date_time_g;   // GPS Date/Time
energy_data_t energy_data_g;   // Energy
command_data_t command_data_g; // Command
setting_data_t setting_data_g; // Setting
sd_data_t sd_data_g;           // SD
radio_data_t radio_data_g;     // Radio
battery_data_t battery_data_g; // Power
modem_data_t modem_lora_433_g; // LoRa 433
modem_data_t modem_fsk_433_g;  // GFSK 433
modem_data_t modem_lora_24_g;  // LoRa 24

// Data struct for printing
energy_data_t energy_data_p;   // Energy
sd_data_t sd_data_p;           // SD ls
sns_data_t sns_data_p;         // Sensors
gps_data_t gps_data_p;         // GPS
radio_data_t radio_data_p;     // Radio
info_data_t info_data_p;       // System Info
ld_meta_data_t ld_meta_data_p; // Meta Data for beacons

// Commands counter
uint32_t cmdCounter = 0;
uint32_t cmdGoodCounter = 0;

char buffer_g[BUFFER_SIZE_MAX];
char lrg_buffer_g[LRG_BUFFER_SIZE_MAX];
int rssi_g = 0;
float snr_g = 0;

// Flags
volatile bool received_433_flag = false;
volatile bool received_24_flag = false;
unsigned long restart_requested_flag = 0L;
bool switch_modem_override = false;
bool command_pending_flag = false;

// Telegram
bool recieved_tlg_flag = false;
bool publish_to_tlg = false;
int64_t satlla0_bot = 1000;
int64_t satlla0_channel = -1000;

// TELEGRAM BOT TOKEN
String token = "mytelegramid:token";
int tlg_ch_enable = TELEGRAM_CH_ENABLE;

/* ============= */
/* Time/Millis() */
/* ============= */
unsigned long ranging_threshold = SECS_15;
unsigned long ts_loop_runtime = TS_LOOP_RUNTIME;
unsigned long post_to_google_threshold = POST_TO_GOOGLE_THRESHOLD;
unsigned long last_post_to_google = 0L;
unsigned long change_radio_setting_timer = 0L; // Global setting change timer
unsigned long tle_read_threshold = SECS_10;
unsigned long tle_read_millis = 0L;
unsigned long tle_updated_threshold = DAY_1;
unsigned long tle_updated_millis = 0L;
unsigned long tlg_read_threshold = SECS_5;
unsigned long tlg_read_millis = 0L;
unsigned long tlgrelay_read_millis = 0L;
unsigned long change_radio_lora_433_setting_timer = 0; // Global setting change timer
unsigned long change_radio_fsk_433_setting_timer = 0;  // Global setting change timer
unsigned long change_radio_lora_24_setting_timer = 0;  // Global setting change timer
unsigned long lora_433_is_on = 0;                      // LoRa 433 is on for duration in millis
uint8_t lora_433_is_sleep = 0;                         // LoRa 433 is on for duration in millis
unsigned long fsk_433_is_on = 0;                       // FSK 433 is on for duration in millis
unsigned long lora_24_is_on = 0;                       // LoRa 24 is on for duration in millis
uint8_t lora_24_is_sleep = 0;                          // LoRa 24 is on for duration in millis
