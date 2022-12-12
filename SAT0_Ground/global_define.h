/**
    @file global_define.h
    @brief Global DEFINE macros/constants for SATLLA0 GS.

    This file contains the global definition for some macros and constants of the SATLLA0 GS.

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

#define NO_ACK 0x00
#define ACK 0x01

/* ============ */
/* TIME Def     */
/* ============ */

#define TEN_MS 10
#define TENTH_SEC 100
#define TENTH2_SEC 200
#define TENTH3_SEC 300
#define HALF_SEC 500
#define SEC_1 1000
#define SECS_2 2 * SEC_1
#define SECS_3 3 * SEC_1
#define SECS_4 4 * SEC_1
#define SECS_5 5 * SEC_1
#define SECS_6 6 * SEC_1
#define SECS_7 7 * SEC_1
#define SECS_8 8 * SEC_1
#define SECS_9 9 * SEC_1
#define SECS_10 10 * SEC_1
#define SECS_15 15 * SEC_1
#define SECS_20 20 * SEC_1
#define SECS_25 25 * SEC_1
#define SECS_30 30 * SEC_1

#define MINUTE_1 60 * 1000
#define MINUTES_2 2 * MINUTE_1
#define MINUTES_3 3 * MINUTE_1
#define MINUTES_4 4 * MINUTE_1
#define MINUTES_5 5 * MINUTE_1
#define MINUTES_6 6 * MINUTE_1
#define MINUTES_7 7 * MINUTE_1
#define MINUTES_8 8 * MINUTE_1
#define MINUTES_9 9 * MINUTE_1
#define MINUTES_10 10 * MINUTE_1
#define MINUTES_20 20 * MINUTE_1
#define MINUTES_30 30 * MINUTE_1
#define HOUR_1 60 * MINUTE_1
#define HOUR_1_5 90 * MINUTE_1
#define HOURS_2 2 * HOUR_1
#define HALF_DAY 12 * HOUR_1
#define DAY_1 24 * HOUR_1

#define DAYS_2 2 * DAY_1
#define DAYS_3 3 * DAY_1
#define DAYS_4 4 * DAY_1
#define DAYS_5 5 * DAY_1
#define DAYS_6 6 * DAY_1
#define WEEK_1 7 * DAY_1

#define WEEKS_2 2 * WEEK_1
#define WEEKS_3 3 * WEEK_1
#define WEEKS_4 4 * WEEK_1

/* ============= */
/* SAT Commands  */
/* ============= */
#define MSG_CHKSUN_ERR 0xE1 // Error
#define CMD_ERROR 0xE0      // Error Chksum
#define CMD_COMPLETED 0x00  // Command Succeed and Complete

#define CMD_CPU_RESTART 0x01   // Restart SAT
#define CMD_RST_GPS 0x02       // Reset GPS
#define CMD_RST_MPU 0x03       // Reset MPU
#define CMD_RST_SETTING 0x04   // Reset global setting
#define CMD_RST_OUTBOX 0x05    // Drop OUTBOX folder
#define CMD_RST_SENT 0x06      // Drop SENT folder
#define CMD_RST_SD_CARD 0x07   // Wipe SD Card
#define CMD_RST_OPER_TIME 0x08 // Reset the oper time
#define CMD_RST_FULL 0x09      // Wipe SD CARD and EEPROM

#define CMD_STRT_BLINK 0x0A             // Start Blink LED
#define CMD_STRT_BLINK_X_MNT 0x0B       // Start Blink LED in X minutes
#define CMD_SEND_BEACON_433 0x0C        // Send Beacon on 433 now
#define CMD_SEND_BEACON_24 0x0D         // Send beacon on 24 now
#define CMD_GET_LOG_LN 0x0E             // Get the 1st line of the log
#define CMD_ECHO_MESSAGE 0x0F           // Echo Message Command
#define CMD_ECHO_MESSAGE_X_MNT 0x25     // Echo Message Command in X minutes
#define CMD_FAT_ON_OFF 0x10             // FAT on/off
#define CMD_CHG_SETTING 0x11            // Change one global setting. Index, Value
#define CMD_GET_SETTING 0x12            // Get global setting
#define CMD_CHG_GLB_SET 0x13            // Change entire global setting
#define CMD_STOP_TX_FLAG 0x14           // A request to stop TX
#define CMD_RESUME_TX_FLAG 0x15         // A request to resume TX
#define CMD_DEPLOY_ANT 0x16             // A request to deploy antenna
#define CMD_STRT_BLINK_LED_LSR 0x17     // Blink both LED and laser
#define CMD_GET_META 0x18               // Retreive a specific meta
#define CMD_GET_FILE_PART 0x19          // Retreive a file by mission and index and part #
#define CMD_GET_FILE 0x1A               // Retreive a file by mission and index
#define CMD_GET_LAST_FILE 0x1B          // Retreive last meta
#define CMD_GET_OUTBOX 0x1C             // Retreive OUTBOX folder/Meta Files
#define CMD_GET_SENT 0x1D               // Retreive SENT folder/Meta files that were sent
#define CMD_SET_LORA_433_BSCP 0x1F      // Set LoRa 433
#define CMD_SET_LORA_24_BSCP 0x20       // Set Lora 24
#define CMD_RANGING_SLAVE_24 0x21       // Set SAT into SLAVE for ranging 24
#define CMD_STRT_BLINK_LSR 0x22         // Start Blink Laser
#define CMD_STRT_BLINK_X_MNT_LSR 0x23   // Start Blink Laser in X minutes
#define CMD_RANGING_RUN 0x24            // Start Master Ranging Execution
#define CMD_RANGING_CALIB 0x26          // Set SAT into SLAVE for ranging 24 calibration
#define CMD_SET_DATE_TIME 0x27          // Set a Date and Time
#define CMD_RST_DATE_TIME 0x28          // Reset Date and Time to last burn time date
#define CMD_SEND_BEACON_FSK 0x29        // Send Beacon on 433 now
#define CMD_SET_LORA_433_BW 0x2A        // Set Lora 433 BW
#define CMD_SET_LORA_433_SF 0x2B        // Set Lora 433 SF
#define CMD_SET_LORA_433_CR 0x2C        // Set Lora 433 CR
#define CMD_SET_LORA_433_PM 0x2D        // Set Lora 433 PM
#define CMD_SET_LORA_433_PL 0x2E        // Set Lora 433 PL
#define CMD_SET_LORA_433_LDRO 0x2F      // Set Lora 433 LDRO
#define CMD_SET_LORA_433_CRC 0x30       // Set Lora 433 CRC
#define CMD_SET_LORA_433_OFFSET 0x3D    // Set Lora 433 Offset
#define CMD_SET_LORA_433_FREQUENCY 0x3F // Set Lora 433 Frequency
#define CMD_SET_LORA_433_STRUCT 0x40    // Set Lora 433 Struct
#define CMD_SET_LORA_433_SW 0x3A        // Set Lora 433 SW
#define CMD_SET_FSK_433_BW 0x31         // Set FSK 433 BW
#define CMD_SET_FSK_433_SF 0x32         // Set FSK 433 SF
#define CMD_SET_FSK_433_CR 0x33         // Set FSK 433 CR
#define CMD_SET_FSK_433_PM 0x34         // Set FSK 433 PM
#define CMD_SET_FSK_433_PL 0x35         // Set FSK 433 PL
#define CMD_SET_FSK_433_LDRO 0x36       // Set FSK 433 LDRO
#define CMD_SET_FSK_433_CRC 0x37        // Set FSK 433 CRC
#define CMD_SET_FSK_433_DS 0x38         // Set FSK 433 DS
#define CMD_SET_FSK_433_BR 0x39         // Set FSK 433 BR
#define CMD_SET_FSK_433_SW 0x3B         // Set FSK 433 SW
#define CMD_SET_FSK_433_DEVIATION 0x3C  // Set FSK 433 Deviation
#define CMD_SET_FSK_433_FREQUENCY 0x3E  // Set FSK 433 Frequency
#define CMD_SET_FSK_433_STRUCT 0x41     // Set Lora 433 Struct
#define CMD_SET_LORA_24_STRUCT 0x42     // Set Lora 433 Struct
#define CMD_SET_LORA_24_BW 0x43         // Set Lora 433 BW
#define CMD_SET_LORA_24_SF 0x44         // Set Lora 433 SF
#define CMD_SET_LORA_24_CR 0x45         // Set Lora 433 CR
#define CMD_SET_LORA_24_PM 0x46         // Set Lora 433 PM
#define CMD_SET_LORA_24_PL 0x47         // Set Lora 433 PL
#define CMD_SET_LORA_24_SW 0x48         // Set Lora 433 SW
#define CMD_SET_LORA_24_LDRO 0x49       // Set Lora 433 LDRO
#define CMD_SET_LORA_24_CRC 0x4A        // Set Lora 433 CRC
#define CMD_SET_LORA_24_OFFSET 0x4B     // Set Lora 433 Offset
#define CMD_SET_LORA_24_FREQUENCY 0x4C  // Set Lora 433 Frequency
#define CMD_SET_SAVE_TO_FLASH 0x4D      // Set Lora 433 Frequency
#define CMD_GET_OUTBOX_FS 0x4E          // Get Outbox folder from FS
#define CMD_GET_SENT_FS 0x4F            // Get Sent folder from FS
#define CMD_RST_FS 0x60   // Wipe FS
#define CMD_RST_OPER_TIME_FS 0x61   // Reset the oper time FS

#define CMD_RPI_1_COMMAND 0x55       // RPI Command
#define CMD_RPI_1_COMMAND_X_MNT 0x56 // RPI turn on in X minutes command

/* ============= */
/* LoRa UHF       */
/* ============= */

// LoRa spreading factors
#define LORA_433_SF6 6
#define LORA_433_SF7 7
#define LORA_433_SF8 8
#define LORA_433_SF9 9
#define LORA_433_SF10 10
#define LORA_433_SF11 11
#define LORA_433_SF12 12

// LoRa coding rates
#define LORA_433_CR_4_5 5
#define LORA_433_CR_4_6 6
#define LORA_433_CR_4_7 7
#define LORA_433_CR_4_8 8

// LoRa bandwidths
//  using bw_433_table[BW_433_SIZE]
#define LORA_433_BW_078 0
#define LORA_433_BW_104 1
#define LORA_433_BW_156 2
#define LORA_433_BW_208 3
#define LORA_433_BW_3125 4
#define LORA_433_BW_417 5
#define LORA_433_BW_625 6
#define LORA_433_BW_125 7
#define LORA_433_BW_250 8

/* ============= */
/* LoRa24        */
/* ============= */

// //LoRa spreading factors
#define LORA_24_SF5 0x50
#define LORA_24_SF6 0x60
#define LORA_24_SF7 0x70
#define LORA_24_SF8 0x80
#define LORA_24_SF9 0x90
#define LORA_24_SF10 0xA0
#define LORA_24_SF11 0xB0
#define LORA_24_SF12 0xC0

// LoRa bandwidths
#define LORA_24_BW_0200 0x34 // actually 203125hz
#define LORA_24_BW_0400 0x26 // actually 406250hz
#define LORA_24_BW_0800 0x18 // actually 812500hz
#define LORA_24_BW_1600 0x0A // actually 1625000hz

// LoRa coding rates
#define LORA_24_CR_4_5 0x01
#define LORA_24_CR_4_6 0x02
#define LORA_24_CR_4_7 0x03
#define LORA_24_CR_4_8 0x04
