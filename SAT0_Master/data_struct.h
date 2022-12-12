/**
    @file data_struct.h
    @brief Data Struct and enums for SATLLA0.
    This contains the data structure and the enum for the different messages for the SAT.
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

enum modem_e
{
    modem_lora_433 = 0x00, // 0
    modem_fsk_433 = 0x01,  // 1
    modem_none = 0x02,     // 2
};

typedef enum lorafreq_e
{
    lora_433 = 0x00,  // 0
    lora_868 = 0x01,  // 1
    lora_915 = 0x02,  // 2
    lora_24 = 0x03,   // 3
    lora_gs = 0x04,   // 4
    lora_none = 0x05, // 5
};

enum messgae_type_e
{
    type_becon = 0x00,
    type_echo = 0x01,
    type_ack = 0x02,
    type_range = 0x03,
    type_rpi = 0x04,
    type_text = 0x05,
    type_sbeacon = 0x06,
    type_command = 0x07,
    type_setting = 0x08,
    type_none = 0x09,
    type_mv = 0x0A,
    type_ld = 0x0B,
    type_rng = 0x0C,
};

enum
{
    sat_panic_power = 0x00,
    sat_med_power = 0x01,
    sat_high_power = 0x02,
};

enum beacon_type_e
{
    beacon_type_long = 0x00,
    beacon_type_short = 0x01
};

enum
{
    ld_type_other = 0x00,
    ld_type_image = 0x01,
    ld_type_meta = 0x14,
    ld_type_stars = 0x15,
    ld_type_text = 0x16,
};

enum
{
    cmd_type_none = 0x00,
    cmd_type_param = 0x01,
    cmd_type_bin = 0x02,
    cmd_type_str = 0x03,
};

enum fat_module
{
    fat_none = 0x00,
    fat_imu = 0x01,
    fat_gps = 0x02,
    fat_433 = 0x03,
    fat_rpi = 0x04,
    fat_24 = 0x05,
    fat_laser = 0x06,
    fat_mainled = 0x07,
    fat_ant = 0x08,
    fat_gen2 = 0x09,
};

#define LORA_HEADER_LENGTH 9 // Struct pad 0 due ^2
#define LORA_PACKET_LENGTH 240
#define LORA_DATA_LENGTH (LORA_PACKET_LENGTH - LORA_HEADER_LENGTH)

union lora_data_t
{
    struct
    {
        uint8_t local_address;                 // 1, SAT Address/Call Sign
        uint8_t destination;                   // 2, Ground Station address
        uint8_t msg_type;                      // 3, Message Type per messgae_type_e enum
        uint8_t msg_size;                      // 4, Message Length
        uint16_t msg_index;                    // 5, Message ID
        uint16_t msg_time;                     // 6, The time of the message since last boot
        uint8_t msg_ack_req;                   // 7. Message ack req. 0=False. 1=True
        uint8_t msg_payload[LORA_DATA_LENGTH]; // 8. The Data
    };
    uint8_t packet[LORA_PACKET_LENGTH]; // This array is used to push out through the LoRa module.
};

#define SHRT_BCN_PACKET_LENGTH 12

union shrt_bcn_data_t
{
    struct
    {
        uint8_t local_address;   // 1, SAT Address/Call Sign
        uint8_t sd_outbox;       // 2, Pending files to sent
        uint8_t msg_type;        // 3, Message Type per messgae_type_e enum
        uint8_t msg_received;    // 4, Recevied Messages
        uint16_t msg_index;      // 5, Message ID
        uint16_t battery_volts;  // 6, Read battery voltage (mV)
        int16_t battery_current; // 7, Read average current (mA)
        int8_t sns_ntc1;         // 8, Temperature
        int8_t sns_ntc2;         // 9, Temperature
    };
    uint8_t packet[SHRT_BCN_PACKET_LENGTH]; // This array is used to push out through the LoRa module.
};

#define DATE_TIME_LENGTH 4

union date_time_t
{
    struct
    {
        uint8_t gps_month; // 2, Month
        uint8_t gps_day;   // 3, Day
        uint8_t gps_hour;  // 4, Hour
        uint8_t gps_min;   // 5, Min
    };
    uint8_t packet[DATE_TIME_LENGTH]; // This array is used to push out through the LoRa module.
};

#define GPS_PACKET_LENGTH 20 + DATE_TIME_LENGTH

union gps_data_t
{
    struct
    {
        date_time_t gps_date_time; // 1, date/time
        float gps_lat;             // 2, Latitude
        float gps_lng;             // 3, Longtidute
        uint16_t gps_alt;          // 4, Altitude
        uint16_t gps_speed;        // 5, Sat Speed KPH
        uint16_t gps_course;       // 6, Sat Course Degree
        uint8_t gps_sat;           // 7, Number of gps sattelites
        uint8_t temp;              // 8, temp field for ^2
    };
    uint8_t packet[GPS_PACKET_LENGTH]; // This array is used to push out throught the LoRa module.
};

#define SNS_PACKET_LENGTH 16

union sns_data_t
{
    struct
    {
        int16_t sns_gx;         // 1, Gyro X rad/s
        int16_t sns_gy;         // 2, Gyro Y rad/s
        int16_t sns_gz;         // 3, Gyro Z rad/s
        int16_t sns_mx;         // 4, Mag X uTesla
        int16_t sns_my;         // 5, Mag Y uTesla
        int16_t sns_mz;         // 6, Mag Z uTesla
        int16_t sns_bmp_temp_c; // 7, BMP Temperature
        int8_t sns_ntc1;        // 8, Temperature
        int8_t sns_ntc2;        // 9, Temperature
    };
    uint8_t packet[SNS_PACKET_LENGTH]; // This array is used to push out throught the LoRa module.
};

#define LD_META_HEADER_LENGTH 2
#define LD_META_PACKET_LENGTH 230
#define LD_META_DATA_LENGTH (LD_META_PACKET_LENGTH - LD_META_HEADER_LENGTH)

union ld_meta_data_t
{
    struct
    {
        uint8_t ld_type;                         // 1, 0=meta, 1=else
        uint8_t ld_size;                         // 2, Size.
        uint8_t ld_payload[LD_META_DATA_LENGTH]; // 3. The Data
    };
    uint8_t packet[LD_META_PACKET_LENGTH]; // This array is used to push out through the LoRa module.
};

#define LD_HEADER_LENGTH 6
#define LD_PACKET_LENGTH 230
#define LD_DATA_LENGTH (LD_PACKET_LENGTH - LD_HEADER_LENGTH)

union ld_data_t
{
    struct
    {
        uint16_t ld_file_seq;               // 1, File Seq (name).
        uint16_t ld_file_index;             // 3, File Index (index within subsequence)
        uint8_t ld_type;                    // 4, 0=meta, 1=else
        uint8_t ld_size;                    // 5, Size.
        uint8_t ld_payload[LD_DATA_LENGTH]; // 6. The Data
    };
    uint8_t packet[LD_PACKET_LENGTH]; // This array is used to push out through the LoRa module.
};

#define ENERGY_PACKET_LENGTH 10

union energy_data_t
{
    struct
    {
        uint16_t engy_shuntvoltage; // 1, Shunt Voltage
        uint16_t engy_busvoltage;   // 2, Bus Voltage
        uint16_t engy_current_ma;   // 3, Current
        uint16_t engy_loadvoltage;  // 4, Load Voltage
        uint16_t engy_power_mw;     // 5, Power
    };
    uint8_t packet[ENERGY_PACKET_LENGTH]; // This array is used to push out through the LoRa module.
};

#define BATTERY_PACKET_LENGTH 12

union battery_data_t
{
    struct
    {
        int8_t battery_soc;             // 1, Read state-of-charge (%)
        int8_t battery_health;          // 2, Read state-of-health (%)
        uint16_t battery_volts;         // 3, Read battery voltage (mV)
        int16_t battery_current;        // 4, Read average current (mA)
        uint16_t battery_full_capacity; // 5, Read full capacity (mAh)
        uint16_t battery_capacity;      // 6, Read remaining capacity (mAh)
        int16_t battery_power;          // 7, Read average power draw (mW)
    };
    uint8_t packet[BATTERY_PACKET_LENGTH]; // This array is used to push out through the LoRa module.
};

#define COMMAND_HEADER_LENGTH 4
#define COMMAND_PACKET_LENGTH 230
#define COMMAND_DATA_LENGTH (COMMAND_PACKET_LENGTH - COMMAND_HEADER_LENGTH)

union command_data_t
{
    struct
    {
        uint8_t cmd_code;                         // 1, Command Code
        uint8_t cmd_type;                         // 2, Command Code
        uint8_t cmd_size;                         // 3, Command payload size
        uint8_t cmd_chksum;                       // 4, Command Chksum
        uint8_t cmd_payload[COMMAND_DATA_LENGTH]; // 5. The Data
    };
    uint8_t packet[COMMAND_PACKET_LENGTH]; // This array is used to push out throught the LoRa module.
};

#define SETTING_PACKET_LENGTH 12
#define KEY_LENGTH 3

union setting_data_t
{
    struct
    {
        uint8_t set_ts_loop_runtime;       // 1, Run time loop/Smart Delay
        uint8_t set_save_to_log_threshold; // 2, Log threshold
        uint8_t set_sleep_delay_panic;     // 3, Sleep delay for panic mode
        uint8_t set_sleep_delay_non_panic; // 4, Sleep delay for non panic mode
        uint8_t set_send_beacon_threshold; // 5, Send beacon threshold
        uint8_t set_delay_between_beacons; // 6, Delay between beacons
        uint8_t set_save_to_flash;         // 7, Overwrite save to flash
        uint8_t set_size_last_meta;        // 8, Size of how many metas to include in the buffer
        char set_key[KEY_LENGTH];          // 9,
    };
    uint8_t packet[SETTING_PACKET_LENGTH]; // This array is used to push out through the LoRa module.
};

#define RINGBUFFER_LENGTH LORA_PACKET_LENGTH + 9

union ringbuffer_data_t
{
    struct
    {
        lora_data_t lora_data; // 1,
        int rssi;              // 2,
        float snr;             // 3,
        lorafreq_e freq;       // 4,
    };
    uint8_t packet[RINGBUFFER_LENGTH]; // This array is used to push out through the LoRa module.
};

#define SD_DATA_LENGTH 4

union sd_data_t
{
    struct
    {
        uint8_t sd_outbox; // 1,
        uint8_t sd_sent;   // 2,
        uint8_t sd_rpi;    // 3,
        uint8_t sd_files;  // 4,
    };
    uint8_t packet[SD_DATA_LENGTH]; // This array is used to push out through the LoRa module.
};

#define RADIO_DATA_LENGTH 2

union radio_data_t
{
    struct
    {
        uint8_t rssi; // 1,
        int8_t snr;   // 2,
    };
    uint8_t packet[RADIO_DATA_LENGTH]; // This array is used to push out through the LoRa module.
};

#define INFO_DATA_LENGTH 4

union info_data_t
{
    struct
    {
        uint8_t reset_index;    // 1,
        uint8_t rx_sat_counter; // 2,
        uint8_t msg_received;   // 3,
        uint8_t temp4;          // 4,
    };
    uint8_t packet[INFO_DATA_LENGTH]; // This array is used to push out through the LoRa module.
};

#define MODEM_DATA_LENGTH 24

union modem_data_t
{
    struct
    {
        uint8_t modem_bw;         // 1, Band Width
        uint8_t modem_cr;         // 2, Codding Rate
        uint8_t modem_sf;         // 3, Spred Factor
        uint8_t modem_power;      // 4, Tx/Rx Power
        uint8_t modem_ldro;       // 5, LDRO (0, 1, 2)
        uint8_t modem_pl;         // 6, Preamble
        uint8_t modem_crc;        // 7, CRC (bool)
        uint8_t modem_ds;         // 8, Gaussin shaping (FSK)
        uint32_t modem_sw;        // 9, Sync Word
        uint32_t modem_frequency; // 10, Freuency
        int32_t modem_offset;     // 11, Offset/Deviation
        float modem_br;           // 12, Bit Rate (FSK)
    };
    uint8_t packet[MODEM_DATA_LENGTH]; // This array is used to push out through the LoRa module.
};
