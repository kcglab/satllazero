# Commands Code and Examples
- Commands are sent in Hex
- Structure: <SATID><CMD><PARAMS>

## CMD_CPU_RESTART 0x01
- Restart Satellite
- B401

## CMD_RST_GPS 0x02
- Reset GPS
- B402

## CMD_RST_IMU 0x03
- Reset IMU
- B403

## CMD_RST_SETTING 0x04
- Reset global setting
- B404

## CMD_RST_OUTBOX 0x05
- Delete OUTBOX folder
- B405

## CMD_RST_SENT 0x06
- Delete SENT folder
- B406

## CMD_RST_SD_CARD 0x07
- Wipe SD Card (Format)
- B407

## CMD_RST_OPER_TIME 0x08
- Reset the operation time counter
- B408

## CMD_RST_FULL 0x09
- Wipe SD CARD and EEPROM (Format and Delete EEPROM settings)
- B409

## CMD_STRT_BLINK 0x0A
- Start Blink LED
- B40A

## CMD_STRT_BLINK_X_MNT 0x0B
- Start Blink LED in X minutes
- Params: X = 2 bytes
- B40B0168 Blink in 360 minutes (0x168 = 360)

## CMD_SEND_BEACON_433 0x0C
- Send long beacon on 433 now
- B40C

## CMD_SEND_BEACON_24 0x0D
- Send long beacon on 24 now
- B40D

## CMD_GET_LOG_LN 0x0E
- Send a line from the log file. Default is first line and LoRa 433
- Params: Line No = 2 bytes, Radio = 1 byte
- B40E000100

## CMD_ECHO_MESSAGE 0x0F
- Echo Message
- Params: Radio = 1 byte, Repeat = 1 byte, Message = Upto 225 bytes
- B40F0001020304

## CMD_ECHO_MESSAGE_X_MNT 0x25
- Echo Message in X minutes
- Params: Radio = 1 byte, X = 2 bytes, Repeat = 1 byte, Message = Upto 223 bytes
- B42501A00102030405

## CMD_FAT_ON_OFF 0x10             // FAT on/off
- Open/Close FAT Module
- Params: FAT Module = 1 byte, Off/On = 1 byte (0/1)
- B4100301

## CMD_CHG_SETTING 0x11            // Change one global setting. Index, Value
- Change gloval setting param by index
- Params: Index = 1 byte, Value = 1 byte

## CMD_GET_SETTING 0x12            // Get global setting
- Get global setting

## CMD_CHG_GLB_SET 0x13            // Change entire global setting
- Change Global Settings
- Params: setting_data_g

## CMD_STOP_TX_FLAG 0x14           // A request to stop TX
- Stop beacon transmitting

## CMD_RESUME_TX_FLAG 0x15         // A request to resume TX
- Resume beacon transmitting

## CMD_DEPLOY_ANT 0x16             // A request to deploy antenna
- Deploy antenna. Keep the FAT open for X seconds.
- Params: Seconds = 1 byte

## CMD_STRT_BLINK_LED_LSR 0x17     // Blink both LED and laser
- Blink both, LED and Laser

## CMD_GET_META 0x18               // Retrieve a specific meta
- Get metafile
- Params: Mission = 2 bytes, Radio = 1 byte
- 

## CMD_GET_FILE_PART 0x19          // Retrieve a file by mission and index and part #
- Get file part
- Params: Mission = 2 bytes, File Index = 2 bytes, Packet Length = 1 byte, Part Number = 1 byte, Radio = 1 byte

## CMD_GET_FILE 0x1A               // Retrieve a file by mission and index
- Get file
- Params: Mission = 2 bytes, File Index = 2 bytes, Packet Length = 1 byte, Radio = 1 byte

## CMD_GET_LAST_FILE 0x1B          // Retrieve last meta
- Get last meta file created 
- Params: Radio = 1 byte

## CMD_GET_OUTBOX 0x1C             // Retrieve OUTBOX folder/Meta Files
- Get all meta files from Outbox folder
- Params: Radio = 1 byte

## CMD_GET_SENT 0x1D               // Retrieve SENT folder/Meta files that were sent
- Get all meta files from Sent folder
- Params: Radio = 1 byte

## CMD_SET_LORA_433_BSCP 0x1F      // Set LoRa 433
## CMD_SET_LORA_24_BSCP 0x20       // Set Lora 24
## CMD_RANGING_SLAVE_24 0x21       // Set SAT into SLAVE for ranging 24
## CMD_STRT_BLINK_LSR 0x22         // Start Blink Laser
## CMD_STRT_BLINK_X_MNT_LSR 0x23   // Start Blink Laser in X minutes
## CMD_RANGING_RUN 0x24            // Start Master Ranging Execution
## CMD_RANGING_CALIB 0x26          // Set SAT into SLAVE for ranging 24 calibration
## CMD_SET_DATE_TIME 0x27          // Set a Date and Time
## CMD_RST_DATE_TIME 0x28          // Reset Date and Time to last burn time date
## CMD_SEND_BEACON_FSK 0x29        // Send Beacon on 433 now
## CMD_SET_LORA_433_BW 0x2A        // Set Lora 433 BW
## CMD_SET_LORA_433_SF 0x2B        // Set Lora 433 SF
## CMD_SET_LORA_433_CR 0x2C        // Set Lora 433 CR
## CMD_SET_LORA_433_PM 0x2D        // Set Lora 433 PM
## CMD_SET_LORA_433_PL 0x2E        // Set Lora 433 PL
## CMD_SET_LORA_433_LDRO 0x2F      // Set Lora 433 LDRO
## CMD_SET_LORA_433_CRC 0x30       // Set Lora 433 CRC
## CMD_SET_LORA_433_OFFSET 0x3D    // Set Lora 433 Offset
## CMD_SET_LORA_433_FREQUENCY 0x3F // Set Lora 433 Frequency
## CMD_SET_LORA_433_STRUCT 0x40    // Set Lora 433 Struct
## CMD_SET_LORA_433_SW 0x3A        // Set Lora 433 SW
## CMD_SET_FSK_433_BW 0x31         // Set FSK 433 BW
## CMD_SET_FSK_433_SF 0x32         // Set FSK 433 SF
## CMD_SET_FSK_433_CR 0x33         // Set FSK 433 CR
## CMD_SET_FSK_433_PM 0x34         // Set FSK 433 PM
## CMD_SET_FSK_433_PL 0x35         // Set FSK 433 PL
## CMD_SET_FSK_433_LDRO 0x36       // Set FSK 433 LDRO
## CMD_SET_FSK_433_CRC 0x37        // Set FSK 433 CRC
## CMD_SET_FSK_433_DS 0x38         // Set FSK 433 DS
## CMD_SET_FSK_433_BR 0x39         // Set FSK 433 BR
## CMD_SET_FSK_433_SW 0x3B         // Set FSK 433 SW
## CMD_SET_FSK_433_DEVIATION 0x3C  // Set FSK 433 Deviation
## CMD_SET_FSK_433_FREQUENCY 0x3E  // Set FSK 433 Frequency
## CMD_SET_FSK_433_STRUCT 0x41     // Set Lora 433 Struct
## CMD_SET_LORA_24_STRUCT 0x42     // Set Lora 433 Struct
## CMD_SET_LORA_24_BW 0x43         // Set Lora 433 BW
## CMD_SET_LORA_24_SF 0x44         // Set Lora 433 SF
## CMD_SET_LORA_24_CR 0x45         // Set Lora 433 CR
## CMD_SET_LORA_24_PM 0x46         // Set Lora 433 PM
## CMD_SET_LORA_24_PL 0x47         // Set Lora 433 PL
## CMD_SET_LORA_24_SW 0x48         // Set Lora 433 SW
## CMD_SET_LORA_24_LDRO 0x49       // Set Lora 433 LDRO
## CMD_SET_LORA_24_CRC 0x4A        // Set Lora 433 CRC
## CMD_SET_LORA_24_OFFSET 0x4B     // Set Lora 433 Offset
## CMD_SET_LORA_24_FREQUENCY 0x4C  // Set Lora 433 Frequency
## CMD_SET_SAVE_TO_FLASH 0x4D      // Set Lora 433 Frequency
## CMD_GET_OUTBOX_FS 0x4E          // Get Outbox folder from FS
## CMD_GET_SENT_FS 0x4F            // Get Sent folder from FS
## CMD_RST_FS 0x60   // Wipe FS 
## CMD_RST_OPER_TIME_FS 0x61   // Reset the oper time FS

## CMD_RPI_1_COMMAND 0x55       // RPI Command
## CMD_RPI_1_COMMAND_X_MNT 0x56 // RPI turn on in X minutes command