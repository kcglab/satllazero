# SATLLA0 Decode Long Beacon:

## Description:
SATLLA0 has long and short beacons. Each beacon is sent in a period of 45 seconds to 1 minute depends on the battery power.

## Short Beacon:
- Short beacon consists of 12  bytes:
> uint8_t local_address;   // 1, SAT Address/Call Sign \
> uint8_t sd_outbox;       // 2, Pending files to sent \
> uint8_t msg_type;        // 3, Message Type per messgae_type_e enum \
> uint8_t msg_received;    // 4, Recevied Messages \
> uint16_t msg_index;      // 5, Message ID \
> uint16_t battery_volts;  // 6, Read battery voltage (mV) \
> int16_t battery_current; // 7, Read average current (mA) \
> int8_t sns_ntc1;         // 8, Temperature \
> int8_t sns_ntc2;         // 9, Temperature \

## Long Beacon:
- Long beacon consists of a header (9 bytes) and payload (dynamic size).
- The type of the beacon located in the 3rd byte of the message.
- SATLLA0 has messages that aren't beacons. Nevertheless, all of them has the same structure: 9 byte header and rest is payload.
- The header is as follows:
- lora_data_t:
> uint8_t local_address; // 1, SAT Address/Call Sign. \
> uint8_t destination;   // 2, Ground Station address \
> uint8_t msg_type;      // 3, Message Type per messgae_type_e enum \ 
> uint8_t msg_size;      // 4, Message Length (for payload) \
> uint16_t msg_index;    // 5, Message ID \
> uint16_t msg_time;     // 7, The time of the message since last boot \
> uint8_t msg_ack_req;   // 9. Message ack req. 0=False. 1=True \
> uint8_t msg_payload[]; // 10. The Data \

- The payload structure in a long beacon consists of 52 bytes as follows:
-- gps_data_t (include date_time_t) 20 bytes
-- sns_data_t 16 bytes
-- battery_data_t 12 bytes
-- sd_data_t 4 bytes

- gps_data_t:
> date_time_t gps_date_time; // 1, Date/Time \
> float gps_lat;        // 2, Latitude \
> float gps_lng;        // 3, Longitude \
> uint16_t gps_alt;     // 4, Altitude \
> uint16_t gps_speed;   // 5, Sat Speed KPH \
> uint16_t gps_course;  // 6, Sat Course Degree \
> uint8_t gps_sat;      // 7, Number of gps satellites \

-date_time_t:
> uint8_t gps_month; // 1, Month \
> uint8_t gps_day;   // 2, Day \
> uint8_t gps_hour;  // 3, Hour \
> uint8_t gps_min;   // 4, Min \

- sns_data_t:
> int16_t sns_gx;         // 1, Gyro X rad/s \
> int16_t sns_gy;         // 2, Gyro Y rad/s \
> int16_t sns_gz;         // 3, Gyro Z rad/s \
> int16_t sns_mx;         // 4, Mag X uTesla \
> int16_t sns_my;         // 5, Mag Y uTesla \
> int16_t sns_mz;         // 6, Mag Z uTesla \
> int16_t sns_bmp_temp_c; // 7, Temperature C \
> int8_t sns_ntc1;        // 8, ? Temperature 1 C \
> int8_t sns_ntc2;        // 9, ? Temperature 2 C \

- battery_data_t
> int8_t battery_soc;             // 1, Read state-of-charge (%) \
> int8_t battery_health;          // 2, Read state-of-health (%) \
> uint16_t battery_volts;         // 3, Read battery voltage (mV) \
> int16_t battery_current;        // 4, Read average current (mA) \
> uint16_t battery_full_capacity; // 5, Read full capacity (mAh) \
> uint16_t battery_capacity;      // 6, Read remaining capacity (mAh) \
> int16_t battery_power;          // 7, Read average power draw (mW) \

- sd_data_t:
> uint8_t sd_outbox; // 1, Pending files to upload \
> uint8_t sd_sent;   // 2, Files sent \
> uint8_t sd_rpi;    // 3, Pending files under Mission Computer to upload \
> uint8_t sd_files;  // 4, All files on the SD - Informative. \

- enum messgae_type_e:
> type_becon = 0x00 \
> type_echo = 0x01 \
> type_ack = 0x02 \
> type_range = 0x03 \
> type_rpi = 0x04 \
> type_text = 0x05 \
> type_sbeacon = 0x06 \
> type_command = 0x07 \
> type_setting = 0x08 \
> type_none = 0x09 \
> type_mv = 0x0A \
> type_ld = 0x0B \
> type_rng = 0x0C \