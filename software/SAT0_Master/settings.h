/**
    @file setting.h
    @brief Header file for SATLLA0.

    This file contains settings and constant for the SATLLA0.

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

/* ===================== */
/* SPI Pinout Teensy 4.1 */
/* ===================== */
// CS_PIN 10
// MOSI_PIN 11
// MISO_PIN 12
// SCK_PIN 13
// SCL_PIN 19
// SDA_PIN 18

#include "debug.h"
#define SAT_NAME "SATLLA0"

#include <SPI.h>
#include <Wire.h>
#include <RingBufCPP.h>
#include <TinyGPS++.h>
#include <TimeLib.h>

union modem_data_t;

#include "data_struct.h"
#include "global_define.h"

const char DATE_[] = (__DATE__ "");
const char TIME_[] = (__TIME__ "");

// Enablers
#define GPS_ENABLE 0            // 0=Off, 1=On <- // GPS
#define LORA_SX1278_ENABLE 0    // 0=Off, 1=On    // LoRa 1278
#define LORA_SX127X_ENABLE 0    // 0=Off, 1=On    // LoRa 1278/2/6
#define LORA_RL_SX1278_ENABLE 1 // 0=Off, 1=On <- // RadioLib LoRa 1278
#define LORA_SX128X_ENABLE 0    // 0=Off, 1=On <- // SX LoRa 128X
#define EEPROM_ENABLE 1         // 0=Off, 1=On <- // EEPROM
#define SD_ENABLE 0             // 0=Off, 1=On <- // SD Card
#define BBSTR_BQ27441_ENABLE 1  // 0=Off, 1=On <- // Babysitter
#define MODELED_ENABLE 1        // 0=Off, 1=On <- // Mode LED
#ifdef ARDUINO_TEENSY41         // T4.1 Support WDT_T4
#define WDT_ENABLE 1            // 0=Off, 1=On <- // WDT_T4
#else                           // Else, On board WD
#define WD_MAX6369_ENABLE 1     // 0=Off, 1=On <- // Watchdog
#endif                          //
#define MAINLED_ENABLE 1        // 0=Off, 1=On <- // Main LED
#define LASER_ENABLE 0          // 0=Off, 1=On <- // Laser
#define IMU_LSM6DS33_ENABLE 1   // 0=Off, 1=On <- // IMU
#define NTC_ENABLE 0            // 0=Off, 1=On <- // NTC 1, 2
#define RPI_ENABLE 0            // 0=Off, 1=On <- // RPI 0
#define RTC_ENABLE 1            // 0=Off, 1=On <- // Real Tome Clock
#ifdef ARDUINO_TEENSY41         // T4.1 Support PSRAM.
#define TNSYFS_ENABLE 1         // 0=Off, 1=On <- // Flash Memory
#endif

#define IMU_ENABLE (IMU_LSM6DS33_ENABLE)
#define RF_433_ENABLE (LORA_SX1278_ENABLE | LORA_SX127X_ENABLE | LORA_RL_SX1278_ENABLE)
#define RF_24_ENABLE (LORA_SX128X_ENABLE)
#define WD_ENABLE (WDT_ENABLE | WD_MAX6369_ENABLE)

/* ============ */
/* Constants    */
/* ============ */
#define INITIAL_BOOT_TIME 2 * MINUTE_1 // 2 * MINUTE_1 // Delay 2 Min
#define TS_LOOP_RUNTIME SECS_20        // 20 secs
#define LOG_THRESHOLD HOUR_1           // Save to LOG every 1 hour
#define SEND_BEACON_THRESHOLD SECS_55  // 55 secs
#define DELAY_BETWEEN_BEACONS SECS_5   // 5 secs between 433 to 24
#define STOP_TX_FLAG_THRESHOLD DAY_1   // Reset every 24 Hours

#define ANTENNA_DEPLOY_PERIOD 20 // 20 Secounds
#define RPI_COMMAND_TRIES 10     // 10 times
#define SD_INIT_TRIES 4          // 5 times
#define FOLDER_MAX_FILES 255     // Max files
#define LOG_SIZE 1048576         // 2^20; //1048576
#define END_LINE '>'
#define CONST_10 10
#define CONST_100 100
#define CONST_1000 1000
#define FLASH_ON 1
#define FLASH_OFF 0
#define MSG_RELAY_MAX_TIMES 10


/* ============ */
/* Serials      */
/* ============ */
// /Applications/Teensyduino.app/Contents/Java/hardware/teensy/avr/cores/teensy4/HardwareSerial1.cpp
#define SERIAL1_RX_BUFFER_SIZE 16384

#define rpi1_serial Serial1 // RX=0, TX=1 // RPI UART 0

// GPS
#if GPS_ENABLE
#define gps_serial Serial2 // RX=7, TX=8 // GPS UART 3
TinyGPSPlus gps;           // GPS
#endif

#define GPS_CHARS_PROCESSED 10
#define GPS_NOT_VALID_THRESHOLD MINUTES_30
#define GPS_VALID_TIME_THRESHOLD MINUTE_1

// LoRa Buffer
#define LORA_RX_TX_BYTE 255
#define BUFFER_SIZE_MAX 255
#define LRG_BUFFER_SIZE_MAX 16384

// Ring Buffer Size
#ifdef ARDUINO_TEENSY32
#define TX_RBUFF_MAX_NUM_ELEMENTS 10
#else
#define TX_RBUFF_MAX_NUM_ELEMENTS 200
#endif

// Restart Teensy
#define CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
#define CPU_RESTART_VAL 0x5FA0004
#define CPU_RESTART (*CPU_RESTART_ADDR = CPU_RESTART_VAL);

// ANT Relase Button
#define ANT_SW_PIN 26

// Main LED
#ifdef ARDUINO_TEENSY32
#define MODE_LED_PIN 6
#else
#define MODE_LED_PIN 16
#endif

// FAT
#define FAT_IMU_PIN 2       //
#define FAT_GPS_PIN 3       //
#ifdef ARDUINO_TEENSY32
#define FAT_LORA_433_PIN 14 //
#else
#define FAT_LORA_433_PIN 4  //
#endif
#define FAT_RPI_1_PIN 5     //
#define FAT_LORA_24_PIN 6   // On-Board
#define FAT_LASER_PIN 28    // FAT1-ON
#ifdef ARDUINO_TEENSY32
#define FAT_MAIN_LED_PIN 8 //
#else
#define FAT_MAIN_LED_PIN 29 //
#endif
#define FAT_ANT_PIN 31      //
#define FAT_GEN2_PIN -1     //

// NTC's
#define NTC1_PIN 40 // PIN_A21 // Bat
#define NTC2_PIN 41 // PIN_A22 // Panel

// Watch Dog
#define WDI_PIN 32

// SD
#define DIRNAME_SIZE 13
#define FILENAME_SIZE 70

// Mode LED
#define MODELED_SHORT 150
#define MODELED_LONG 300

/* ============= */
/* LoRa 433      */
/* ============= */

#if RF_433_ENABLE
#if LORA_SX127X_ENABLE
#define LORA_433_CR_4_5 0x02
#define LORA_433_SF10 0x0A
#define LORA_433_BW_062 96 // actually 812500hz
#else
#define LORA_433_CR_4_5 5
#define LORA_433_SF10 10
#define LORA_433_BW_062 6 // table
#endif                    // LORA_SX127X_ENABLE

#define LORA_433_CRC_ON 0x00  // Packet CRC is activated
#define LORA_433_LDRO_ON 0x00 // 0x00 OFF, 0x01 ON, 0x02 AUTO

#define LORA_433_NSS_PIN 22
#ifdef ARDUINO_TEENSY32
#define LORA_433_DIO0_PIN 16
#else
#define LORA_433_DIO0_PIN 21
#endif
#define LORA_433_DIO1_PIN -1
#define LORA_433_RST_PIN 23
#define LORA_433_RX_EN_PIN -1
#define LORA_433_TX_EN_PIN -1

#define LORA_433_BAND 437250000 // Band
#define LORA_433_OFFSET 0       // Offset
#define LORA_433_PABOOST true   // Boost

#define LORA_433_BW LORA_433_BW_062
#define LORA_433_SF LORA_433_SF10
#define LORA_433_CR LORA_433_CR_4_5
#define LORA_433_TX_RX_POWER 20
#define LORA_433_SW 0x2A               // Syncword
#define LORA_433_PL 8                  // preambleLength
#define LORA_433_TCXO -1               // tcxoVoltage
#define LORA_433_LDRO LORA_433_LDRO_ON // 0x00 OFF, 0x01 ON, 0x02 AUTO
#define LORA_433_CRC LORA_433_CRC_ON   // 0x00 OFF, 0x01 ON

#define FSK_433_DEVIATION 5.0       // kHz single-sideband
#define FSK_433_BW 39.0             // 62.5
#define FSK_433_BR 9.6              // kbps nominal
#define FSK_433_CR 140.0            // mA
#define FSK_433_TX_RX_POWER 20      // Tx Power
#define FSK_433_DS 0x02             // RADIOLIB_SHAPING_0_5 GFSK filter BT
#define FSK_433_SW 0x2A             // Syncword
#define FSK_433_PL 16               // bits
#define FSK_433_CRC LORA_433_CRC_ON // 0x00 OFF, 0x01 ON
#endif                              // RF_433_ENABLE

/* ============= */
/* LoRa 2.4      */
/* ============= */

#if RF_24_ENABLE
#define LORA_24_CR_4_5 0x01
#define LORA_24_SF10 0xA0
#define LORA_24_BW_0800 0x18 // actually 812500hz

#define LORA_24_NSS_PIN 39
#define LORA_24_BSY_PIN 36
#define LORA_24_RST_PIN 35
#define LORA_24_DIO1_PIN 17
#define LORA_24_TCXO_PIN -1
#define LORA_24_RX_EN_PIN 24
#define LORA_24_TX_EN_PIN 25

#define LORA_24_BAND 2401000000 // frequency of transmissions
#define LORA_24_OFFSET 0        // offset frequency for calibration purposes
#define LORA_24_BW LORA_24_BW_0800
#define LORA_24_SF LORA_24_SF10
#define LORA_24_CR LORA_24_CR_4_5
#define LORA_24_TX_RX_POWER 13
#define LORA_24_SW 0x2A000000
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
#define LORA_24_RNG_COUNT 10                // number of ranging requests at each attempt
#define LORA_24_RNG_DELAY_MS 275            // mS delay between packets, minimum 130mS for BW400, SF10 ranging packets

/* ============= */
/* LoRa 2.4 EXT  */
/* ============= */

#define LORA_24_ENABLE_EXT_PIN 10

#define LORA_24_NSS_EXT_PIN 20
#define LORA_24_BSY_EXT_PIN 37
#define LORA_24_RST_EXT_PIN 38
#define LORA_24_DIO1_EXT_PIN 9
#define LORA_24_TCXO_EXT_PIN -1
#define LORA_24_RX_EN_EXT_PIN 15
#define LORA_24_TX_EN_EXT_PIN 14

#endif // RF_24_ENABLE

/* ============ */
/* Globals      */
/* ============ */
volatile int modeled_blink_count = 0;
volatile int modeled_blink_period = MODELED_SHORT;
uint8_t rpi1_command_tries = 0;
modem_e current_modem = modem_none;
bool gfsk_beacon = true;
beacon_type_e b_type;
uint8_t sat_state = sat_high_power;

const bool reset_device = true;           // enable device reset between ranging requests.
const int8_t RangingTXPower = 10;         // Transmit power
const float distance_adjustment = 0.8967; // adjustment to calculated distance
bool ranging_execute_start = false;
bool ranging_request_ack = false;

char buffer_g[BUFFER_SIZE_MAX];
char lrg_buffer_g[LRG_BUFFER_SIZE_MAX];
uint8_t rx_buffer_g[BUFFER_SIZE_MAX];
uint8_t rx_buffer_g_size;

// SATLLA0 Key
char key[KEY_LENGTH] = {'A', 'A', 'A'};
const char KEY[KEY_LENGTH] = {'A', 'A', 'A'};

uint16_t tx_counter = 0;     // TX Counter
uint16_t rx_counter = 0;     // RX Counter
uint16_t errors_counter = 0; // Counter of errors
uint8_t rx_sat_counter = 0;  // Counter of RX not GS origin

const byte local_address = 0xA0; // address of this device
const byte destination = 0xE0;   // destination to send to

RingBufCPP<ld_data_t, TX_RBUFF_MAX_NUM_ELEMENTS> LDTXbuffer;
RingBufCPP<ld_meta_data_t, TX_RBUFF_MAX_NUM_ELEMENTS> LDMETATXbuffer;

// Radio
int rssi_g = 0;
float snr_g = 0.0;

uint8_t save_to_flash = FLASH_ON;  // FLASH_ON/FLASH_OFF
uint8_t size_last_meta = CONST_10; //

// Globals Data Struct
lora_data_t lora_data_g;              // LoRa Message
lora_data_t beacon_g;                 // Beacon
lora_data_t prnt_msg_g;               // Print Message
lora_data_t reply_ack_g;              // Replay Ack Message
lora_data_t wrap_msg_g;               // Wrap Message for Re-send
lora_data_t ld_packet_g;              // LrgData Package
lora_data_t rpi_packet_g;             // RPI Package
lora_data_t send_command_g;           // Send Command
lora_data_t rng_request_g;            // ranging request
shrt_bcn_data_t short_beacon_g;       // Short Beacon
shrt_bcn_data_t prnt_short_beacon_g;  // Pring Short Beacon
sns_data_t sns_data_g;                // Sensors
gps_data_t gps_data_g;                // GPS Lat/Lng
date_time_t gps_date_time_g;          // GPS Date/Time
energy_data_t energy_data_g;          // Energy
command_data_t command_data_g;        // Command
command_data_t shoutdown_cmd_g;       // Shoutdown
command_data_t command_data_mv_g;     // Command
command_data_t command_data_echo_msg; // Command
command_data_t command_data_rpi1_g;   // RPI_1
setting_data_t setting_data_g;        // Setting
setting_data_t setting_data_b;        // Setting
ld_data_t ld_data_g;                  // RPI
ld_meta_data_t ld_meta_data_g;        // Meta Data
ld_meta_data_t ld_meta_data_b;        // Meta Data for beacons
sd_data_t sd_data_g;                  // SD ls
radio_data_t radio_data_g;            // Radio
battery_data_t battery_data_g;        // Battery
info_data_t info_data_g;              // system info
modem_data_t modem_lora_433_g;        // LoRa 433
modem_data_t modem_fsk_433_g;         // GFSK 433
modem_data_t modem_lora_24_g;         // LoRa 24

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

// Battery
float HIGH_POWER_CRITERIA = 3800;
float LOW_POWER_CRITERIA = 3400;
float LOW_BATTERY = 3200;

// Ant Switch
#ifdef DEBUG
volatile uint8_t ant_button_ref = 0;
volatile uint8_t ant_button_state = 0;
#endif

/* ============ */
/* GPS          */
/* ============ */


/* ============ */
/* Blink LED    */
/* ============ */
bool should_blink_LED = false; // Set True to turn on FAT LED
bool didBlinked = false;       // Indicate the LED did blinked

bool should_blink_LED_delay = false; // Flag indicating LED should be blinked in delay maner
unsigned long start_blink_delay = 0; // Delayed blink started in millis()
unsigned long blink_delay_msec = 0;  // Delay time in msec to start blinking

// Constants for blinking
const uint16_t TimePeriods[] = {MAIN_LED_BLINK_1HZ, MAIN_LED_BLINK_10HZ, MAIN_LED_BLINK_100HZ, MAIN_LED_BLINK_9600HZ};
const uint16_t CycleSecs[] = {MAIN_LED_CYCLE_1HZ_30S, MAIN_LED_CYCLE_10HZ_30S, MAIN_LED_CYCLE_100HZ_30S, MAIN_LED_CYCLE_9600HZ_6S};
const uint16_t PeriodUnits[] = {SEC_1, SEC_1, SEC_1, 1};
uint32_t cycleSec = CycleSecs[0];

/* ============ */
/* Blink LASER  */
/* ============ */
bool should_blink_laser = false; // Set True to turn on FAT LED
bool did_blinked_laser = false;  // Indicate the LED did blinked

bool should_blink_laser_delay = false;     // Flag indicating LED should be blinked in delay maner
unsigned long start_blink_laser_delay = 0; // Delayed blink started in millis()
unsigned long blink_laser_delay_msec = 0;  // Delay time in msec to start blinking

// Constants for blinking
const uint16_t time_periods_laser[] = {MAIN_LASER_BLINK_1HZ, MAIN_LASER_BLINK_10HZ, MAIN_LASER_BLINK_100HZ, MAIN_LASER_BLINK_9600HZ};
// const uint16_t cycle_secs_laser[] = {MAIN_LASER_CYCLE_1HZ_30S, MAIN_LASER_CYCLE_10HZ_30S, MAIN_LASER_CYCLE_100HZ_30S, MAIN_LASER_CYCLE_9600HZ_6S};
const uint16_t cycle_secs_laser[] = {MAIN_LASER_CYCLE_1HZ_5S, MAIN_LASER_CYCLE_10HZ_5S, MAIN_LASER_CYCLE_100HZ_5S, MAIN_LASER_CYCLE_9600HZ_5S};

const uint16_t period_units_laser[] = {SEC_1, SEC_1, SEC_1, 1};
uint32_t cycle_sec_laser = cycle_secs_laser[0];

/* ============ */
/* SD Card      */
/* ============ */
uint8_t sd_outbox_g = 0;
uint16_t file_index_g = 0;

char dirname_g[DIRNAME_SIZE] = {0};
char new_dirname_g[DIRNAME_SIZE] = {0};

char filename_g[FILENAME_SIZE] = {0};
char new_filename_g[FILENAME_SIZE] = {0};

const static char *SEQ_FILE = "SEQUENCE.BIN";          // Sequence file name
const static char *INITIAL_BOOT_FILE = "INITBOOT.FIN"; // Initboot file name. Indicate INIT is complete
const static char *SET_FILE = "SETTING.BIN";           // Setting file. At moment moved to EEPROM so not in used.
// const static char *LAST_IDX_FILE = "LAST.BIN";         // Pointer to the last image file
const static char *IDX_FILE = "INDEX.BIN";    // Pointer to the last image file
const static char *OPERATE_FILE = "OPER.BIN"; // Pointer to the operate time file. Counting half days
const static char *RESET_FILE = "RESET.BIN";  // Pointer to the last image file
const static char SAT_LOG_FILE[] = "LOG";     // Name of GPS file name
const static char TXT_SUFFIX[] = "TXT";       // Prefix for text files
const static char BIN_SUFFIX[] = "BIN";       // Prefix for text files

const static char OUTBOX_FOLDER[] = "OUTB"; // Outbox folder
const static char SENT_FOLDER[] = "SENT";   // Sent Folder
const static char RPI_FOLDER[] = "RPI";     // RPI Folder

/* ============ */
/* Flags        */
/* ============ */
volatile bool received_433_flag = false;
volatile bool received_24_flag = false;

bool restart_requested_flag = false;
bool idle_requested_flag = false;
bool is_antenna_deployed_flag = false;

bool mission_laser_blink_flag = false;
bool mission_led_blink_flag = false;
bool mission_rpi_flag = false;

bool should_turn_rpi1_delay_flag = false;
bool rpi_task_completed_flag = false;
bool rpi1_command_noack_flag = false;

bool should_echo_msg_delay_flag = false; // Echo message delay

/* ============= */
/* Time/Millis() */
/* ============= */
unsigned long ts_loop_runtime = TS_LOOP_RUNTIME;
unsigned long delay_between_beacons = SECS_2;
unsigned long sleep_delay_non_panic = SECS_30;
unsigned long sleep_delay_panic = SECS_45;
unsigned long save_to_log_threshold = LOG_THRESHOLD;
// unsigned long read_energy_threshold = READ_ENERGY_THRESHOLD;
unsigned long send_beacon_threshold = SEND_BEACON_THRESHOLD; // interval between sends
unsigned long stop_tx_flag_threshold = STOP_TX_FLAG_THRESHOLD;
unsigned long ranging_threshold = SECS_15;

unsigned long turn_rpi1_delay = 0;
unsigned long turn_rpi1_msec = 0;
unsigned long last_sent_433_time = 0;                        // last sent on 433 time
unsigned long last_sent_beacon_time = SEND_BEACON_THRESHOLD; // last sent beacon time
unsigned long last_sent_long_beacon = 0;                     // last sent long beacon
uint8_t sent_long_beacon_counter = 0;                        // long beacon counter for threshold
unsigned long rpi1_is_on = 0;                                // RPI is on for duration in millis
unsigned long gps_is_on = 0;                                 // GPS is on for duration in millis
uint8_t gps_is_sleep = 0;                                    // GPS is on for duration in millis
unsigned long lora_433_is_on = 0;                            // LoRa 433 is on for duration in millis
uint8_t lora_433_is_sleep = 0;                               // LoRa 433 is on for duration in millis
unsigned long fsk_433_is_on = 0;                             // FSK 433 is on for duration in millis
unsigned long lora_24_is_on = 0;                             // LoRa 24 is on for duration in millis
uint8_t lora_24_is_sleep = 0;                                // LoRa 24 is on for duration in millis
unsigned long imu_is_on = 0;                                 // BNO is on for duration in millis
uint8_t imu_is_sleep = 0;                                    // BNO is on for duration in millis
unsigned long ina_is_on = 0;                                 // INA is on for duration in millis
unsigned long sdcard_is_on = 0;                              // SD_CARD is on for duration in millis
unsigned long flash_is_on = 0;                               // Flash MEM is on for duration in millis
unsigned long last_save_to_log = 0;                          // Last write into log file in millis
unsigned long stop_tx_flag = 0;                              // Indicator to stop sending radio data
unsigned long loop_start_millis = 0;                         // Variable to to capture millis on the main loop
unsigned long gps_not_valid = 0;                             //
unsigned long last_blink_led = 0;                            // last blink led auto
unsigned long last_save_oper_half_days = 0;                  // last save time 21 days
unsigned long last_ranging_24 = 0;                           // last ranging auto
unsigned long time_on_g = 0;                                 // Global temp variable
unsigned long change_radio_lora_433_setting_timer = 0;       // Global setting change timer
unsigned long change_radio_fsk_433_setting_timer = 0;        // Global setting change timer
unsigned long change_radio_lora_24_setting_timer = 0;        // Global setting change timer
unsigned long start_echo_msg_delay = 0.0;                    // Echo message delay starting time
unsigned long echo_msg_delay_msec = 0.0;                     // Echo message delay time
unsigned long last_auto_task = 0.0;                          // Auto Task


/* ============= */
/* Auto Task     */
/* ============= */
// CMD_TAKE_PHOTO 0xB45504
uint8_t rpi_cmd_take_photo[5] = {0x55, 0x00, 0x01, 0x00, 0x04};
// CMD_SIT 0x05 (Satellite Image Transfer)
uint8_t rpi_cmd_sit[5] = {0x55, 0x00, 0x01, 0x00, 0x05};
// CMD_SEEK_LASER 0x06 (Satellite Image Transfer)
uint8_t rpi_cmd_seek_laser[5] = {0x55, 0x00, 0x01, 0x00, 0x06};
// CMD_RWCS 0x07 (Reaction Wheel Control System)
uint8_t rpi_cmd_rwcs[5] = {0x55, 0x00, 0x01, 0x00, 0x07};
// CMD_NEW_TAKE_PHOTO 0x0F ()
uint8_t rpi_cmd_new_take_photo[5] = {0x55, 0x00, 0x01, 0x00, 0x0f};