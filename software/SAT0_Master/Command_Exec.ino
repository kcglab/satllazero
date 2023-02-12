/**
    @file Command_Exec.ino
    @brief Commands utils functions for SATLLA0.
    
    This contains the commands file for the SAT.
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

/* =============== */
/* Receive Command */
/* =============== */

void cmd_receive(command_data_t *command_data)
{
    PRINTLN(F("Func: cmd_receive()"));

    uint8_t chkSum = code_checksum(command_data);
    command_data->cmd_chksum = chkSum;

    int commandStatus = activate_command(command_data);

    PRINT(F("Parse and Command Status: "));
    PRINTLN(commandStatus);
}

/* =============== */
/* Send Command    */
/* =============== */

void cmd_send(command_data_t *command_data, lorafreq_e freq)
{
    PRINTLN(F("Func: cmd_send()"));

    uint8_t cmd_code = command_data->cmd_code;
    uint8_t cmd_type = command_data->cmd_type;
    uint8_t cmd_size = command_data->cmd_size;
    uint8_t checksum = checksum_func(cmd_code, cmd_type, cmd_size);
    command_data->cmd_chksum = checksum;

#ifdef PRINT_FUNC_DEBUG
    print_command(command_data);
#endif

    encrypt_decrypt(command_data);

    send_command_g.local_address = local_address;
    send_command_g.destination = destination;
    send_command_g.msg_type = type_command;
    send_command_g.msg_size = COMMAND_HEADER_LENGTH + cmd_size;
    send_command_g.msg_index = tx_counter;
    send_command_g.msg_time = millis() / SEC_1;
    send_command_g.msg_ack_req = 0x01;

    memcpy(send_command_g.msg_payload, command_data->packet, COMMAND_HEADER_LENGTH + cmd_size);
#if RF_433_ENABLE
    send_message_lora_433(send_command_g.packet, LORA_HEADER_LENGTH + COMMAND_HEADER_LENGTH + cmd_size);
#endif
}

/* =============== */
/* Code ChkSum     */
/* =============== */
uint8_t checksum_func(uint8_t x1, uint8_t x2, uint8_t x3)
{
    return (0xFF - (x1 ^ x2 ^ x3));
}

uint8_t code_checksum(command_data_t *command_data)
{
    PRINTLN(F("Func: code_checksum()"));
    uint8_t cmd_code = command_data->cmd_code;
    uint8_t cmd_type = command_data->cmd_type;
    uint8_t cmd_size = command_data->cmd_size;

    uint8_t checksum = checksum_func(cmd_code, cmd_type, cmd_size);
    PRINT(F("ANS:\t"));
    PRINTLN(checksum, HEX);

    return checksum;
}

bool check_if_msg_correct(command_data_t *command_data)
{
    PRINTLN(F("Func: check_if_msg_correct()"));

    encrypt_decrypt(command_data);

    uint8_t cmd_code = command_data->cmd_code;
    uint8_t cmd_type = command_data->cmd_type;
    uint8_t cmd_size = command_data->cmd_size;
    uint8_t checksum = command_data->cmd_chksum;

    uint8_t ans = checksum_func(cmd_code, cmd_type, cmd_size);
    PRINT(F("ANS: "));
    PRINTLN(ans, HEX);
    PRINT(F("CKS: "));
    PRINTLN(checksum, HEX);
    return (checksum == ans);
}

/* ================= */
/* Encrypt Decrypt   */
/* ================= */

void encrypt_decrypt(command_data_t *command_data)
{
    PRINTLN(F("Func: encrypt_decrypt()"));

    for (uint8_t i = 0; i < COMMAND_HEADER_LENGTH - 1; i++)
    {
        command_data->packet[i] = command_data->packet[i] ^ key[i % KEY_LENGTH];
    }
}

/* ================= */
/* Parse Command     */
/* ================= */

int parse_and_activate_command(command_data_t *command_data)
{
    PRINTLN(F("Func: parse_and_activate_command()"));
    cmdCounter++;
    bool checksum = check_if_msg_correct(command_data);
    if (!checksum) // there is error in checksum. Msg is invalid!
    {
        PRINTLN(F("Error: Message checksum incorrect!"));
        return MSG_CHKSUN_ERR;
    }
    cmdGoodCounter++;
    int commandStatus = activate_command(command_data);
    return commandStatus;
}

/* ================= */
/* Activate Command  */
/* ================= */

int activate_command(command_data_t *command_data)
{
    PRINTLN(F("Func: activate_command()"));
#ifdef PRINT_FUNC_DEBUG
    print_command(command_data);
#endif
    uint8_t cmd_code = command_data->cmd_code;
    uint8_t cmd_type = command_data->cmd_type;
    uint8_t cmd_size = command_data->cmd_size;
    uint8_t checksum = command_data->cmd_chksum;
    uint8_t cmd_payload[COMMAND_DATA_LENGTH] = {0};

    if (cmd_size > 0)
    {
        memcpy(cmd_payload, command_data->cmd_payload, cmd_size);
    }

    // if need to wake PI, then turned it on.
    if (cmd_code == CMD_RPI_1_COMMAND && !rpi1_is_on)
    {
        PRINTLN(F("1. CMD_RPI_1_COMMAND"));
#if RPI_ENABLE
        rpi1_turn_on();
        rpi1_command_noack_flag = true;
        memcpy(command_data_g.packet, command_data->packet, COMMAND_HEADER_LENGTH + cmd_size);
#endif
    }

    // switch command
    switch (cmd_code)
    {

    // 0x01 // Reset SAT
    case CMD_CPU_RESTART:
    {
        PRINTLN(F("CMD_CPU_RESTART"));
        restart_requested_flag = true;
    }
    break;

        // 0x02 // Reset GPS
    case CMD_RST_GPS:
    {
        PRINTLN(F("CMD_RST_GPS"));
#if GPS_ENABLE
        reset_gps();
#endif
    }
    break;

        // 0x03 // Reset IMU
    case CMD_RST_IMU:
    {
        PRINTLN(F("CMD_RST_IMU"));
#if IMU_ENABLE
        reset_imu();
#endif
    }
    break;

        // 0x04 // Reset global setting
    case CMD_RST_SETTING:
    {
        PRINTLN(F("CMD_RST_SETTING"));
#if EEPROM_ENABLE
        reset_global_setting();
        eeprom_system_clear();
#endif
        restart_requested_flag = true;
    }
    break;

        // 0x05 // Drop OUTBOX folder
    case CMD_RST_OUTBOX:
    {
        PRINTLN(F("CMD_RST_OUTBOX"));

        if (sdcard_is_on)
        {
#if SD_ENABLE
            delete_folder(OUTBOX_FOLDER);
#endif
        }

        if (flash_is_on)
        {
#if TNSYFS_ENABLE
            fs_delete_folder(OUTBOX_FOLDER);
#endif
        }
    }
    break;

        // 0x06 // Drop SENT folder
    case CMD_RST_SENT:
    {
        PRINTLN(F("CMD_RST_SENT"));

        if (sdcard_is_on)
        {
#if SD_ENABLE
            delete_folder(SENT_FOLDER);
#endif
        }

        if (flash_is_on)
        {
#if TNSYFS_ENABLE
            fs_delete_folder(SENT_FOLDER);
#endif
        }
    }
    break;

        // 0x07 // Wipe SD Card
    case CMD_RST_SD_CARD:
    {
        PRINTLN(F("CMD_RST_SD_CARD"));
        if (sdcard_is_on)
        {
#if SD_ENABLE
            wipe_sd_card();
            restart_requested_flag = false; // since the above set to true
#endif
        }
    }
    break;

        // 0x60 // Wipe FS
    case CMD_RST_FS:
    {
        PRINTLN(F("CMD_RST_FS"));
        if (flash_is_on)
        {
#if TNSYFS_ENABLE
            wipe_fs();
            restart_requested_flag = false; // since the above set to true
#endif
        }
    }
    break;

        // 0x08 // Reset the oper time
    case CMD_RST_OPER_TIME:
    {
        PRINTLN(F("CMD_RST_OPER_TIME"));

        if (sdcard_is_on)
        {
#if SD_ENABLE
            set_operation_time_file(0);
#endif
        }
    }
    break;

        // 0x61 // Reset the oper time FS
    case CMD_RST_OPER_TIME_FS:
    {
        PRINTLN(F("CMD_RST_OPER_TIME_FS"));

        if (flash_is_on)
        {
#if TNSYFS_ENABLE
            fs_set_operation_time_file(0);
#endif
        }
    }
    break;

        // 0x09 // Wipe SD CARD and EEPROM
    case CMD_RST_FULL:
    {
        PRINTLN(F("CMD_RST_FULL"));

        if (sdcard_is_on)
        {
#if SD_ENABLE
            wipe_sd_card();
            restart_requested_flag = false; // since the above set to true
#endif
        }

        if (flash_is_on)
        {
#if TNSYFS_ENABLE
            wipe_fs();
            restart_requested_flag = false; // since the above set to true
#endif
        }

#if EEPROM_ENABLE
        eeprom_clear_settings();
        eeprom_system_clear();
#endif
        restart_requested_flag = true;
    }
    break;

        // 0x0A // Start Blink LED
    case CMD_STRT_BLINK:
    {
        PRINTLN(F("CMD_STRT_BLINK"));
#if MAINLED_ENABLE
        should_blink_LED = true;
#endif
    }
    break;

        // 0x0B // Start Blink LED in X minutes
    case CMD_STRT_BLINK_X_MNT:
    {
        PRINTLN(F("CMD_STRT_BLINK_X_MNT"));
#if MAINLED_ENABLE
        should_blink_LED_delay = true;
        start_blink_delay = millis();
        blink_delay_msec = ((cmd_payload[0] << 8) + cmd_payload[1]) * MINUTE_1;
        PRINT(F("blink_delay_msec:\t"));
        PRINTLN(blink_delay_msec);
#endif
    }
    break;

        // 0x17 // Blink both LED and laser
    case CMD_STRT_BLINK_LED_LSR:
    {
        PRINTLN(F("CMD_STRT_BLINK_LED_LSR"));
#if LASER_ENABLE || MAINLED_ENABLE
        should_blink_LED = true;
        should_blink_laser = true;
#endif
    }
    break;

        // 0x22 // Start Blink Laser
    case CMD_STRT_BLINK_LSR:
    {
        PRINTLN(F("CMD_STRT_BLINK_LSR"));
#if LASER_ENABLE
        should_blink_laser = true;
#endif
    }
    break;

        // 0x23 // Start Blink Laser in X minutes
    case CMD_STRT_BLINK_X_MNT_LSR:
    {
        PRINTLN(F("CMD_STRT_BLINK_X_MNT_LSR"));
#if LASER_ENABLE
        should_blink_laser_delay = true;
        start_blink_laser_delay = millis();
        blink_laser_delay_msec = ((cmd_payload[0] << 8) + cmd_payload[1]) * MINUTE_1;
        PRINT(F("blink_laser_delay_msec:\t"));
        PRINTLN(blink_laser_delay_msec);
#endif
    }
    break;

        // 0x0C // Send Beacon on 433 now
    case CMD_SEND_BEACON_433:
    {
        PRINTLN(F("CMD_SEND_BEACON_433"));
        uint16_t file_count = 0;
        if (sdcard_is_on)
        {
#if SD_ENABLE
            file_count = sd_ls(&sd_data_g);
#endif
        }

        if (file_count < 1 || !sdcard_is_on || save_to_flash)
        {
            if (flash_is_on)
            {
#if TNSYFS_ENABLE
                fs_ls(&sd_data_g);
#endif
            }
        }
        handle_beacon(beacon_type_long, true);
        send_beacon(beacon_type_long, lora_433);
    }
    break;

        // 0x0D // Send beacon on 24 now
    case CMD_SEND_BEACON_24:
    {
        PRINTLN(F("CMD_SEND_BEACON_24"));
        uint16_t file_count = 0;
        if (sdcard_is_on)
        {
#if SD_ENABLE
            file_count = sd_ls(&sd_data_g);
#endif
        }

        if (file_count < 1 || !sdcard_is_on || save_to_flash)
        {
            if (flash_is_on)
            {
#if TNSYFS_ENABLE
                fs_ls(&sd_data_g);
#endif
            }
        }
        handle_beacon(beacon_type_long, true);
        send_beacon(beacon_type_long, lora_24);
    }
    break;

        // 0x0E // Get line from the log
    case CMD_GET_LOG_LN:
    {
        PRINTLN(F("CMD_GET_LOG_LN"));
#if SD_ENABLE && (RF_433_ENABLE || RF_24_ENABLE)
        // if no parmas then first line
        uint16_t line = (cmd_payload[0] << 8) + cmd_payload[1];
        uint8_t radio = cmd_payload[2];
        int length = read_line_from_log_file(line); // read into buffer_g
        if (radio == lora_433)
        {
#if RF_433_ENABLE
            wrap_message_433(buffer_g, length, type_none);
#endif
        }
        else
        {
#if RF_24_ENABLE
            wrap_message_24(buffer_g, length, type_none);
#endif
        }
#endif
    }
    break;

        // 0x0F // Echo Message Command
    case CMD_ECHO_MESSAGE:
    {
        PRINTLN(F("CMD_ECHO_MESSAGE"));
#if (RF_433_ENABLE || RF_24_ENABLE)
        uint8_t radio = cmd_payload[0];
        uint8_t times = cmd_payload[1];
        if (times == 0 || times > MSG_RELAY_MAX_TIMES)
        {
            times = 1;
        }
        for (uint8_t i = 0; i < times; i++)
        {
            if (radio == lora_433)
            {
#if RF_433_ENABLE
                wrap_message_433(cmd_payload + 2, cmd_size - 2, type_echo);
                delay(SEC_1);
#endif
            }
            else
            {
#if RF_24_ENABLE
                wrap_message_24(cmd_payload + 2, cmd_size - 2, type_echo);
                delay(SEC_1);
#endif
            }
        }
#endif
    }
    break;

        // 0x25 // Echo Message Command in X minutes
        // rb4250000010522334455
        // rb40f000522334455
    case CMD_ECHO_MESSAGE_X_MNT:
    {
        PRINTLN(F("CMD_ECHO_MESSAGE_X_MNT"));
        // wrap_message_433(cmd_payload, cmd_size, type_echo);
        should_echo_msg_delay_flag = true;
        start_echo_msg_delay = millis();
        // cmd_payload[0] == radio
        // 1~2: reserved for x minutes
        echo_msg_delay_msec = ((cmd_payload[1] << 8) + cmd_payload[2]) * MINUTE_1;
        PRINT(F("echo_msg_delay_msec:\t"));
        PRINTLN(echo_msg_delay_msec);

        // copy the header and 1 byte of the payload for radio indicator.
        memcpy(command_data_echo_msg.packet, command_data->packet, COMMAND_HEADER_LENGTH + 1);
        // copy the payload but not byte 1 and 2 of the X mnt.
        memcpy(command_data_echo_msg.packet + COMMAND_HEADER_LENGTH + 1, command_data->packet + COMMAND_HEADER_LENGTH + 3, cmd_size - 3);
        command_data_echo_msg.cmd_code = CMD_ECHO_MESSAGE;
        // reset payload size.
        command_data_echo_msg.cmd_size = cmd_size - 2;  // remove 2 bytes from size.
    }
    break;

        // 0x10 // FAT on/off
    case CMD_FAT_ON_OFF:
    {
        PRINTLN(F("CMD_FAT_ON_OFF"));
        // cmd_payload[0] = fat_module, cmd_payload[1] = value 0:off, 1:on
        fat_set_state(cmd_payload[0], (bool)cmd_payload[1]);
    }
    break;

        // 0x11 // Change one global setting. Index, Value
    case CMD_CHG_SETTING:
    {
        PRINTLN(F("CMD_CHG_SETTING"));
        // Code: CMD_CHG_SETTING, cmd_payload[0]: index, cmd_payload[1] value
#if EEPROM_ENABLE
        save_global_variable(cmd_payload[0], cmd_payload[1]);
#endif
    }
    break;

        // 0x12 // Get global setting
    case CMD_GET_SETTING:
    {
        PRINTLN(F("CMD_GET_SETTING"));
#if EEPROM_ENABLE && RF_433_ENABLE
        bool valid = eeprom_get_settings(&setting_data_b);
        wrap_message_433(setting_data_b.packet, SETTING_PACKET_LENGTH, type_setting);
#endif
    }
    break;

        // 0x13 // Change entire global setting
    case CMD_CHG_GLB_SET:
    {
        PRINTLN(F("CMD_CHG_GLB_SET"));
        if (command_data->cmd_size >= SETTING_PACKET_LENGTH)
        {
            memcpy(setting_data_g.packet, command_data->cmd_payload, command_data->cmd_size);
#if EEPROM_ENABLE
            set_global_settings(&setting_data_g);
#endif
        }
        else
        {
            PRINTLN(F("Error: Setting Data is Invalid."));
        }
    }
    break;

        // 0x14 // A request to stop TX
    case CMD_STOP_TX_FLAG:
    {
        PRINTLN(F("CMD_STOP_TX_FLAG"));
        stop_tx_flag = millis();
    }
    break;

        // 0x15 // A request to resume TX
    case CMD_RESUME_TX_FLAG:
    {
        PRINTLN(F("CMD_RESUME_TX_FLAG"));
        stop_tx_flag = 0;
    }
    break;

        // 0x16 // A request to deploy antenna
    case CMD_DEPLOY_ANT:
    {
        PRINTLN(F("CMD_DEPLOY_ANT"));
        uint8_t secs = cmd_payload[0];
        deploy_antenna(secs);
    }
    break;

        // 0x18 // Retreive a specific meta
    case CMD_GET_META:
    {
        PRINTLN(F("CMD_GET_META"));
        // r18005900
#if RPI_ENABLE
        uint16_t mission = (cmd_payload[0] << 8) + cmd_payload[1];
        uint8_t radio = cmd_payload[2];
        bool ret_value = false;

        if (sdcard_is_on)
        {
#if SD_ENABLE
            ret_value = read_meta_file(mission);
#endif
        }

        if (!ret_value || !sdcard_is_on || save_to_flash)
        {
            if (flash_is_on)
            {
#if TNSYFS_ENABLE
                ret_value = fs_read_meta_file(mission);
#endif
            }
        }

        if (ret_value)
        {
            send_ld_meta_packet(radio);
        }
#endif
    }
    break;

        //  0x19 // Retreive a file by mission and index and part #
    case CMD_GET_FILE_PART:
    {
        PRINTLN(F("CMD_GET_FILE_PART"));
        // r1900590001320100
        //  uint16_t mission, uint16_t file_index, uint8_t message_len, uint16_t part_number, uint8_t radio
#if RPI_ENABLE
        uint16_t mission = (cmd_payload[0] << 8) + cmd_payload[1];
        uint16_t file_index = (cmd_payload[2] << 8) + cmd_payload[3];
        uint8_t message_len = cmd_payload[4];
        uint8_t part_number = cmd_payload[5];
        uint8_t radio = cmd_payload[6];

        if (mission > 0 && file_index > 0)
        {
            if (message_len <= 0)
            {
                message_len = 50;
            }
            prepare_and_send_ld_packet_part(mission, file_index, message_len, part_number, radio);
        }
#endif
    }
    break;

        // 0x1A // Retreive a file by mission and index
    case CMD_GET_FILE:
    {
        PRINTLN(F("CMD_GET_FILE"));
        // r1A005900013200
        // r1A005900153200 0x15 = stars
        // uint16_t mission, uint16_t file_index, uint8_t message_len, uint8_t radio
#if RPI_ENABLE
        uint16_t mission = (cmd_payload[0] << 8) + cmd_payload[1];
        // PRINT(F("mission:\t"));
        // PRINTLN(mission);
        uint16_t file_index = (cmd_payload[2] << 8) + cmd_payload[3];
        uint8_t message_len = cmd_payload[4];
        uint8_t radio = cmd_payload[5];
        if (mission > 0 && file_index > 0)
        {
            if (message_len <= 0)
            {
                message_len = 50;
            }
            prepare_and_send_ld_packet(mission, file_index, message_len, radio);
        }
#endif
    }
    break;

        // 0x1B // Retreive last meta
    case CMD_GET_LAST_FILE:
    {
        PRINTLN(F("CMD_GET_LAST_FILE"));
        // r1B00
#if RPI_ENABLE
        uint8_t radio = cmd_payload[0];
        uint16_t mission = 0;
        bool ret_value = false;
        if (sdcard_is_on)
        {
#if SD_ENABLE
            mission = get_misson_index();
            ret_value = read_meta_file(mission);
#endif
        }

        if (!ret_value || !sdcard_is_on || save_to_flash)
        {
            if (flash_is_on)
            {
#if TNSYFS_ENABLE
                mission = fs_get_misson_index();
                ret_value = fs_read_meta_file(mission);
#endif
            }
        }

        if (ret_value)
        {
            send_ld_meta_packet(radio);
        }
#endif
    }
    break;

        // 0x1C // Retreive OUTBOX folder/Meta Files
    case CMD_GET_OUTBOX:
    {
        PRINTLN(F("CMD_GET_OUTBOX"));
        uint8_t file_counter = 0;
        if (sdcard_is_on)
        {
#if SD_ENABLE
            // Check if there is data to be sent
            file_counter = sd_ls_folder(OUTBOX_FOLDER);
#endif
        }

        PRINT(F("File Counter:\t"));
        PRINTLN(file_counter);

        delay(SEC_1); // Let the GS back to listen

        if (file_counter > 0)
        {
            bool ret_value = false;
            if (sdcard_is_on)
            {
#if SD_ENABLE
                ret_value = send_meta_files(OUTBOX_FOLDER);
#endif
            }

            if (ret_value && !LDMETATXbuffer.isEmpty())
            {
                PRINT(F("LDMETATXbuffer Element:\t"));
                PRINTLN(LDMETATXbuffer.numElements());
#if RPI_ENABLE
                send_ld_meta_packet(cmd_payload[0]);
#endif
            }

            if (sdcard_is_on)
            {
#if SD_ENABLE
                clear_outbox();
#endif
            }
        }
    }
    break;

        // 0x4e // Retreive OUTBOX folder/Meta Files
    case CMD_GET_OUTBOX_FS:
    {
        PRINTLN(F("CMD_GET_OUTBOX_FS"));
        uint8_t file_counter = 0;
        if (flash_is_on)
        {
#if TNSYFS_ENABLE
            file_counter = fs_ls_folder(OUTBOX_FOLDER);
#endif
        }
        PRINT(F("File Counter:\t"));
        PRINTLN(file_counter);

        delay(SEC_1); // Let the GS back to listen

        if (file_counter > 0)
        {
            bool ret_value = false;
            if (flash_is_on)
            {
#if TNSYFS_ENABLE
                ret_value = fs_send_meta_files(OUTBOX_FOLDER);
#endif
            }

            if (ret_value && !LDMETATXbuffer.isEmpty())
            {
                PRINT(F("LDMETATXbuffer Element:\t"));
                PRINTLN(LDMETATXbuffer.numElements());
#if RPI_ENABLE
                send_ld_meta_packet(cmd_payload[0]);
#endif
            }

            if (flash_is_on)
            {
#if TNSYFS_ENABLE
                fs_clear_outbox();
#endif
            }
        }
    }
    break;

        // 0x1D // Retreive SENT folder/Meta files that were sent
    case CMD_GET_SENT:
    {
        PRINTLN(F("CMD_GET_SENT"));
        // Check if there is data to be sent
        uint8_t file_counter = 0;
        if (sdcard_is_on)
        {
#if SD_ENABLE
            // Check if there is data to be sent
            file_counter = sd_ls_folder(SENT_FOLDER);
#endif
        }

        PRINT(F("File Counter:\t"));
        PRINTLN(file_counter);

        if (file_counter > 0)
        {
            bool ret_value = false;
            if (sdcard_is_on)
            {
#if SD_ENABLE
                ret_value = send_meta_files(SENT_FOLDER);
#endif
            }

            if (ret_value && !LDMETATXbuffer.isEmpty())
            {
                PRINT(F("LDMETATXbuffer Element:\t"));
                PRINTLN(LDMETATXbuffer.numElements());
#if RPI_ENABLE
                send_ld_meta_packet(cmd_payload[0]);
#endif
            }
        }
    }
    break;

    // 0x1D // Retreive SENT folder/Meta files that were sent
    case CMD_GET_SENT_FS:
    {
        PRINTLN(F("CMD_GET_SENT_FS"));
        // Check if there is data to be sent
        uint8_t file_counter = 0;
        if (flash_is_on)
        {
#if TNSYFS_ENABLE
            file_counter = fs_ls_folder(SENT_FOLDER);
#endif
        }
        PRINT(F("File Counter:\t"));
        PRINTLN(file_counter);

        if (file_counter > 0)
        {
            bool ret_value = false;
            if (flash_is_on)
            {
#if TNSYFS_ENABLE
                ret_value = fs_send_meta_files(SENT_FOLDER);
#endif
            }

            if (ret_value && !LDMETATXbuffer.isEmpty())
            {
                PRINT(F("LDMETATXbuffer Element:\t"));
                PRINTLN(LDMETATXbuffer.numElements());
#if RPI_ENABLE
                send_ld_meta_packet(cmd_payload[0]);
#endif
            }
        }
    }
    break;


        // 0x1F // Set LoRa 433
    case CMD_SET_LORA_433_BSCP:
    {
        PRINTLN(F("CMD_SET_LORA_433_BSCP"));
        // Code: CMD_SET_LORA_433_BSCP, cmd_payload[0]: bw, cmd_payload[1] sf, cmd_payload[2]: cr, cmd_payload[3] pm
#if RF_433_ENABLE
        if (cmd_payload[0] >= 0 && cmd_payload[0] <= 10 && cmd_payload[1] > 0 && cmd_payload[2] > 0 && cmd_payload[3] > 0)
        {
            modem_lora_433_g.modem_bw = cmd_payload[0];
            modem_lora_433_g.modem_sf = cmd_payload[1];
            modem_lora_433_g.modem_cr = cmd_payload[2];
            modem_lora_433_g.modem_power = cmd_payload[3];
            setup_lora_433(&modem_lora_433_g);
            change_radio_lora_433_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x40 // Set LoRa 433 bw
    case CMD_SET_LORA_433_STRUCT:
    {
        PRINTLN(F("CMD_SET_LORA_433_STRUCT"));
        // Code: CMD_SET_LORA_433_STRUCT, cmd_payload[0]: modem_data_t
#if RF_433_ENABLE
        memcpy(modem_lora_433_g.packet, cmd_payload, MODEM_DATA_LENGTH);
        setup_lora_433(&modem_lora_433_g);
        change_radio_lora_433_setting_timer = millis();
#endif
    }
    break;

        // 0x2a // Set LoRa 433 bw
    case CMD_SET_LORA_433_BW:
    {
        PRINTLN(F("CMD_SET_LORA_433_BW"));
        // Code: CMD_SET_LORA_433_BW, cmd_payload[0]: bw
#if RF_433_ENABLE
        if (cmd_payload[0] >= 0 && cmd_payload[0] <= 10)
        {
            modem_lora_433_g.modem_bw = cmd_payload[0];
            setup_lora_433(&modem_lora_433_g);
            change_radio_lora_433_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x2b // Set LoRa 433 sf
    case CMD_SET_LORA_433_SF:
    {
        PRINTLN(F("CMD_SET_LORA_433_SF"));
        // Code: CMD_SET_LORA_433_SF, cmd_payload[0]: sf
#if RF_433_ENABLE
        if (cmd_payload[0] > 0 && cmd_payload[0] <= 12)
        {
            modem_lora_433_g.modem_sf = cmd_payload[0];
            setup_lora_433(&modem_lora_433_g);
            change_radio_lora_433_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x2c // Set LoRa 433 cr
    case CMD_SET_LORA_433_CR:
    {
        PRINTLN(F("CMD_SET_LORA_433_CR"));
        // Code: CMD_SET_LORA_433_CR, cmd_payload[0]: cr
#if RF_433_ENABLE
        if (cmd_payload[0] > 0 && cmd_payload[0] <= 10)
        {
            modem_lora_433_g.modem_cr = cmd_payload[0];
            setup_lora_433(&modem_lora_433_g);
            change_radio_lora_433_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x2d // Set LoRa 433 pm
    case CMD_SET_LORA_433_PM:
    {
        PRINTLN(F("CMD_SET_LORA_433_PM"));
        // Code: CMD_SET_LORA_433_PM, cmd_payload[0]: pm
#if RF_433_ENABLE
        if (cmd_payload[0] > 0 && cmd_payload[0] <= 20)
        {
            modem_lora_433_g.modem_power = cmd_payload[0];
            setup_lora_433(&modem_lora_433_g);
            change_radio_lora_433_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x2e // Set LoRa 433 pl
    case CMD_SET_LORA_433_PL:
    {
        PRINTLN(F("CMD_SET_LORA_433_PL"));
        // Code: CMD_SET_LORA_433_PL, cmd_payload[0]: pl
#if RF_433_ENABLE
        if (cmd_payload[0] > 0 && cmd_payload[0] <= 20)
        {
            modem_lora_433_g.modem_pl = cmd_payload[0];
            setup_lora_433(&modem_lora_433_g);
            change_radio_lora_433_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x3A // Set LoRa 433 sw
    case CMD_SET_LORA_433_SW:
    {
        PRINTLN(F("CMD_SET_LORA_433_SW"));
        // Code: CMD_SET_LORA_433_SW, cmd_payload[0]: sw
#if RF_433_ENABLE
        uint32_t sw = (cmd_payload[0] << 24) + (cmd_payload[1] << 16) + (cmd_payload[2] << 8) + cmd_payload[3]; //  Forcing a byte order
        PRINT(F("SW:"));
        PRINTLN(sw, HEX);
        if (sw > 0)
        {
            modem_lora_433_g.modem_sw = sw;
            setup_lora_433(&modem_lora_433_g);
            change_radio_lora_433_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x2f // Set LoRa 433 ldro
    case CMD_SET_LORA_433_LDRO:
    {
        PRINTLN(F("CMD_SET_LORA_433_LDRO"));
        // Code: CMD_SET_LORA_433_LDRO, cmd_payload[0]: ldro
#if RF_433_ENABLE
        if (cmd_payload[0] >= 0 && cmd_payload[0] <= 2)
        {
            modem_lora_433_g.modem_ldro = cmd_payload[0];
            setup_lora_433(&modem_lora_433_g);
            change_radio_lora_433_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x30 // Set LoRa 433 crc
    case CMD_SET_LORA_433_CRC:
    {
        PRINTLN(F("CMD_SET_LORA_433_CRC"));
        // Code: CMD_SET_LORA_433_CRC, cmd_payload[0]: crc
#if RF_433_ENABLE
        if (cmd_payload[0] >= 0 && cmd_payload[0] <= 1)
        {
            modem_lora_433_g.modem_crc = cmd_payload[0];
            setup_lora_433(&modem_lora_433_g);
            change_radio_lora_433_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x3d // LoRa 433 offset
    case CMD_SET_LORA_433_OFFSET:
    {
        PRINTLN(F("CMD_SET_LORA_433_OFFSET"));
        // Code: CMD_SET_LORA_433_OFFSET, cmd_payload[0]: offset
#if RF_433_ENABLE
        int32_t offset = (cmd_payload[0] << 24) + (cmd_payload[1] << 16) + (cmd_payload[2] << 8) + cmd_payload[3]; //  Forcing a byte order
        PRINT(F("offset:"));
        PRINTLN(offset);
        modem_lora_433_g.modem_offset = offset;
        setup_lora_433(&modem_lora_433_g);
        change_radio_lora_433_setting_timer = millis();
#endif
    }
    break;

        // 0x3f // Set LoRa 433 Frequency
    case CMD_SET_LORA_433_FREQUENCY:
    {
        PRINTLN(F("CMD_SET_LORA_433_FREQUENCY"));
        // Code: CMD_SET_LORA_433_FREQUENCY, cmd_payload[0]: Frequency
#if RF_433_ENABLE
        uint32_t frequency = (cmd_payload[0] << 24) + (cmd_payload[1] << 16) + (cmd_payload[2] << 8) + cmd_payload[3]; //  Forcing a byte order
        PRINT(F("frequency:"));
        PRINTLN(frequency);
        modem_lora_433_g.modem_frequency = frequency;
        setup_lora_433(&modem_lora_433_g);
        change_radio_lora_433_setting_timer = millis();
#endif
    }
    break;

        // 0x31 // Set FSK 433 bw
    case CMD_SET_FSK_433_BW:
    {
        PRINTLN(F("CMD_SET_FSK_433_BW"));
        // Code: CMD_SET_FSK_433_BW, cmd_payload[0]: bw
#if RF_433_ENABLE
        if (cmd_payload[0] >= 0 && cmd_payload[0] <= 10)
        {
            modem_fsk_433_g.modem_bw = cmd_payload[0];
            setup_fsk_433(&modem_fsk_433_g);
            change_radio_fsk_433_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x32 // Set FSK 433 sf
    case CMD_SET_FSK_433_SF:
    {
        PRINTLN(F("CMD_SET_FSK_433_SF"));
        // Code: CMD_SET_FSK_433_SF, cmd_payload[0]: sf
#if RF_433_ENABLE
        if (cmd_payload[0] > 0 && cmd_payload[0] <= 12)
        {
            modem_fsk_433_g.modem_sf = cmd_payload[0];
            setup_fsk_433(&modem_fsk_433_g);
            change_radio_fsk_433_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x33 // Set FSK 433 cr
    case CMD_SET_FSK_433_CR:
    {
        PRINTLN(F("CMD_SET_FSK_433_CR"));
        // Code: CMD_SET_FSK_433_CR, cmd_payload[0]: cr
#if RF_433_ENABLE
        if (cmd_payload[0] > 0 && cmd_payload[0] <= 10)
        {
            modem_fsk_433_g.modem_cr = cmd_payload[0];
            setup_fsk_433(&modem_fsk_433_g);
            change_radio_fsk_433_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x34 // Set FSK 433 pm
    case CMD_SET_FSK_433_PM:
    {
        PRINTLN(F("CMD_SET_FSK_433_PM"));
        // Code: CMD_SET_FSK_433_PM, cmd_payload[0]: pm
#if RF_433_ENABLE
        if (cmd_payload[0] > 0 && cmd_payload[0] <= 20)
        {
            modem_fsk_433_g.modem_power = cmd_payload[0];
            setup_fsk_433(&modem_fsk_433_g);
            change_radio_fsk_433_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x35 // Set FSK 433 pl
    case CMD_SET_FSK_433_PL:
    {
        PRINTLN(F("CMD_SET_FSK_433_PL"));
        // Code: CMD_SET_FSK_433_PL, cmd_payload[0]: pl
#if RF_433_ENABLE
        if (cmd_payload[0] > 0 && cmd_payload[0] <= 20)
        {
            modem_fsk_433_g.modem_pl = cmd_payload[0];
            setup_fsk_433(&modem_fsk_433_g);
            change_radio_fsk_433_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x36 // Set FSK 433 ldro
    case CMD_SET_FSK_433_LDRO:
    {
        PRINTLN(F("CMD_SET_FSK_433_LDRO"));
        // Code: CMD_SET_FSK_433_LDRO, cmd_payload[0]: ldro
#if RF_433_ENABLE
        if (cmd_payload[0] >= 0 && cmd_payload[0] <= 2)
        {
            modem_fsk_433_g.modem_ldro = cmd_payload[0];
            setup_fsk_433(&modem_fsk_433_g);
            change_radio_fsk_433_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x37 // Set FSK 433 crc
    case CMD_SET_FSK_433_CRC:
    {
        PRINTLN(F("CMD_SET_FSK_433_CRC"));
        // Code: CMD_SET_FSK_433_CRC, cmd_payload[0]: crc
#if RF_433_ENABLE
        if (cmd_payload[0] >= 0 && cmd_payload[0] <= 1)
        {
            modem_fsk_433_g.modem_crc = cmd_payload[0];
            setup_fsk_433(&modem_fsk_433_g);
            change_radio_fsk_433_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x38 // Set FSK 433 ds
    case CMD_SET_FSK_433_DS:
    {
        PRINTLN(F("CMD_SET_FSK_433_DS"));
        // Code: CMD_SET_FSK_433_DS, cmd_payload[0]: ds // RADIOLIB_SHAPING_0_5
#if RF_433_ENABLE
        if (cmd_payload[0] > 0 && cmd_payload[0] <= 4)
        {
            modem_fsk_433_g.modem_ds = cmd_payload[0];
            setup_fsk_433(&modem_fsk_433_g);
            change_radio_fsk_433_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x39 // Set FSK 433 br
    case CMD_SET_FSK_433_BR:
    {
        PRINTLN(F("CMD_SET_FSK_433_BR"));
        // Code: CMD_SET_FSK_433_BR, cmd_payload[0]: br
#if RF_433_ENABLE
        uint32_t val = (cmd_payload[0] << 24) + (cmd_payload[1] << 16) + (cmd_payload[2] << 8) + cmd_payload[3]; //  Forcing a byte order
        float br = *((float *)&val);
        PRINT(F("br:"));
        PRINTLN(br);
        if (br > 0)
        {
            modem_fsk_433_g.modem_br = br;
            setup_fsk_433(&modem_fsk_433_g);
            change_radio_fsk_433_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x3b // Set FSK 433 sw
    case CMD_SET_FSK_433_SW:
    {
        PRINTLN(F("CMD_SET_FSK_433_SW"));
        // Code: CMD_SET_FSK_433_SW, cmd_payload[0]: sw
#if RF_433_ENABLE
        uint32_t sw = (cmd_payload[0] << 24) + (cmd_payload[1] << 16) + (cmd_payload[2] << 8) + cmd_payload[3]; //  Forcing a byte order
        PRINT(F("SW:"));
        PRINTLN(sw, HEX);
        if (sw > 0)
        {
            modem_fsk_433_g.modem_sw = sw;
            setup_fsk_433(&modem_fsk_433_g);
            change_radio_fsk_433_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x3c // Set FSK 433 deviation
    case CMD_SET_FSK_433_DEVIATION:
    {
        PRINTLN(F("CMD_SET_FSK_433_DEVIATION"));
        // Code: CMD_SET_FSK_433_DEVIATION, cmd_payload[0]: deviation
#if RF_433_ENABLE
        int32_t deviation = (cmd_payload[0] << 24) + (cmd_payload[1] << 16) + (cmd_payload[2] << 8) + cmd_payload[3]; //  Forcing a byte order
        PRINT(F("deviation:"));
        PRINTLN(deviation);
        setup_fsk_433(&modem_fsk_433_g);
        change_radio_fsk_433_setting_timer = millis();
#endif
    }
    break;

        // 0x3e // Set FSK 433 Frequency
    case CMD_SET_FSK_433_FREQUENCY:
    {
        PRINTLN(F("CMD_SET_FSK_433_FREQUENCY"));
        // Code: CMD_SET_FSK_433_FREQUENCY, cmd_payload[0]: Frequency
#if RF_433_ENABLE
        uint32_t frequency = (cmd_payload[0] << 24) + (cmd_payload[1] << 16) + (cmd_payload[2] << 8) + cmd_payload[3]; //  Forcing a byte order
        PRINT(F("frequency:"));
        PRINTLN(frequency);

        modem_fsk_433_g.modem_frequency = frequency;
        setup_fsk_433(&modem_fsk_433_g);
        change_radio_fsk_433_setting_timer = millis();
#endif
    }
    break;

        // 0x41 // Set LoRa 433 bw
    case CMD_SET_FSK_433_STRUCT:
    {
        PRINTLN(F("CMD_SET_FSK_433_STRUCT"));
        // Code: CMD_SET_FSK_433_STRUCT, cmd_payload[0]: modem_data_t
#if RF_433_ENABLE
        memcpy(modem_fsk_433_g.packet, cmd_payload, MODEM_DATA_LENGTH);
        setup_fsk_433(&modem_fsk_433_g);
        change_radio_fsk_433_setting_timer = millis();
#endif
    }
    break;

        // 0x42 // Set LoRa 24 struct
    case CMD_SET_LORA_24_STRUCT:
    {
        PRINTLN(F("CMD_SET_LORA_24_STRUCT"));
        // Code: CMD_SET_LORA_24_STRUCT, cmd_payload[0]: modem_data_t
#if RF_24_ENABLE
        memcpy(modem_lora_24_g.packet, cmd_payload, MODEM_DATA_LENGTH);
        setup_lora_24(&modem_lora_24_g);
        change_radio_lora_24_setting_timer = millis();
#endif
    }
    break;

    // 0x20 // Set LoRa 24
    case CMD_SET_LORA_24_BSCP:
    {
        PRINTLN(F("CMD_SET_LORA_24_BSCP"));
        // Code: CMD_SET_LORA_24_BSCP, cmd_payload[0]: bw, cmd_payload[1] sf, cmd_payload[2]: cr, cmd_payload[3] pm
#if RF_24_ENABLE
        if (cmd_payload[0] >= 0 && cmd_payload[0] <= 10 && cmd_payload[1] > 0 && cmd_payload[2] > 0 && cmd_payload[3] > 0)
        {
            modem_lora_24_g.modem_bw = cmd_payload[0];
            modem_lora_24_g.modem_sf = cmd_payload[1];
            modem_lora_24_g.modem_cr = cmd_payload[2];
            modem_lora_24_g.modem_power = cmd_payload[3];
            setup_lora_24(&modem_lora_24_g);
            change_radio_lora_24_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x43 // Set LoRa 24 bw
    case CMD_SET_LORA_24_BW:
    {
        PRINTLN(F("CMD_SET_LORA_24_BW"));
        // Code: CMD_SET_LORA_24_BW, cmd_payload[0]: bw
#if RF_24_ENABLE
        if (cmd_payload[0] >= 0 && cmd_payload[0] <= 10)
        {
            modem_lora_24_g.modem_bw = cmd_payload[0];
            setup_lora_24(&modem_lora_24_g);
            change_radio_lora_24_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x44 // Set LoRa 24 sf
    case CMD_SET_LORA_24_SF:
    {
        PRINTLN(F("CMD_SET_LORA_24_SF"));
        // Code: CMD_SET_LORA_24_SF, cmd_payload[0]: sf
#if RF_24_ENABLE
        if (cmd_payload[0] > 0 && cmd_payload[0] <= 12)
        {
            modem_lora_24_g.modem_sf = cmd_payload[0];
            setup_lora_24(&modem_lora_24_g);
            change_radio_lora_24_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x45 // Set LoRa 24 cr
    case CMD_SET_LORA_24_CR:
    {
        PRINTLN(F("CMD_SET_LORA_24_CR"));
        // Code: CMD_SET_LORA_24_CR, cmd_payload[0]: cr
#if RF_24_ENABLE
        if (cmd_payload[0] > 0 && cmd_payload[0] <= 10)
        {
            modem_lora_24_g.modem_cr = cmd_payload[0];
            setup_lora_24(&modem_lora_24_g);
            change_radio_lora_24_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x46 // Set LoRa 24 pm
    case CMD_SET_LORA_24_PM:
    {
        PRINTLN(F("CMD_SET_LORA_24_PM"));
        // Code: CMD_SET_LORA_24_PM, cmd_payload[0]: pm
#if RF_24_ENABLE
        if (cmd_payload[0] > 0 && cmd_payload[0] <= 20)
        {
            modem_lora_24_g.modem_power = cmd_payload[0];
            setup_lora_24(&modem_lora_24_g);
            change_radio_lora_24_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x47 // Set LoRa 24 pl
    case CMD_SET_LORA_24_PL:
    {
        PRINTLN(F("CMD_SET_LORA_24_PL"));
        // Code: CMD_SET_LORA_24_PL, cmd_payload[0]: pl
#if RF_24_ENABLE
        if (cmd_payload[0] > 0 && cmd_payload[0] <= 20)
        {
            modem_lora_24_g.modem_pl = cmd_payload[0];
            setup_lora_24(&modem_lora_24_g);
            change_radio_lora_24_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x48 // Set LoRa 24 sw
    case CMD_SET_LORA_24_SW:
    {
        PRINTLN(F("CMD_SET_LORA_24_SW"));
        // Code: CMD_SET_LORA_24_SW, cmd_payload[0]: sw
#if RF_24_ENABLE
        uint32_t sw = (cmd_payload[0] << 24) + (cmd_payload[1] << 16) + (cmd_payload[2] << 8) + cmd_payload[3]; //  Forcing a byte order
        PRINT(F("SW:"));
        PRINTLN(sw, HEX);
        if (sw > 0)
        {
            modem_lora_24_g.modem_sw = sw;
            setup_lora_24(&modem_lora_24_g);
            change_radio_lora_24_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x49 // Set LoRa 24 ldro
    case CMD_SET_LORA_24_LDRO:
    {
        PRINTLN(F("CMD_SET_LORA_24_LDRO"));
        // Code: CMD_SET_LORA_24_LDRO, cmd_payload[0]: ldro
#if RF_24_ENABLE
        if (cmd_payload[0] >= 0 && cmd_payload[0] <= 2)
        {
            modem_lora_24_g.modem_ldro = cmd_payload[0];
            setup_lora_24(&modem_lora_24_g);
            change_radio_lora_24_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x4a // Set LoRa 24 crc
    case CMD_SET_LORA_24_CRC:
    {
        PRINTLN(F("CMD_SET_LORA_24_CRC"));
        // Code: CMD_SET_LORA_24_CRC, cmd_payload[0]: crc
#if RF_24_ENABLE
        if (cmd_payload[0] >= 0 && cmd_payload[0] <= 1)
        {
            modem_lora_24_g.modem_crc = cmd_payload[0];
            setup_lora_24(&modem_lora_24_g);
            change_radio_lora_24_setting_timer = millis();
        }
#endif
    }
    break;

        // 0x4b // LoRa 24 offset
    case CMD_SET_LORA_24_OFFSET:
    {
        PRINTLN(F("CMD_SET_LORA_24_OFFSET"));
        // Code: CMD_SET_LORA_24_OFFSET, cmd_payload[0]: offset
#if RF_24_ENABLE
        int32_t offset = (cmd_payload[0] << 24) + (cmd_payload[1] << 16) + (cmd_payload[2] << 8) + cmd_payload[3]; //  Forcing a byte order
        PRINT(F("offset:"));
        PRINTLN(offset);
        modem_lora_24_g.modem_offset = offset;
        setup_lora_24(&modem_lora_24_g);
        change_radio_lora_24_setting_timer = millis();
#endif
    }
    break;

        // 0x4c // Set LoRa 24 Frequency
    case CMD_SET_LORA_24_FREQUENCY:
    {
        PRINTLN(F("CMD_SET_LORA_24_FREQUENCY"));
        // Code: CMD_SET_LORA_24_FREQUENCY, cmd_payload[0]: Frequency
#if RF_24_ENABLE
        uint32_t frequency = (cmd_payload[0] << 24) + (cmd_payload[1] << 16) + (cmd_payload[2] << 8) + cmd_payload[3]; //  Forcing a byte order
        PRINT(F("frequency:"));
        PRINTLN(frequency);
        modem_lora_24_g.modem_frequency = frequency;
        setup_lora_24(&modem_lora_24_g);
        change_radio_lora_24_setting_timer = millis();
#endif
    }
    break;

        // 0x21 // Set SAT into SLAVE for ranging 24
    case CMD_RANGING_SLAVE_24:
    {
        PRINTLN(F("CMD_RANGING_SLAVE_24"));
#if RF_24_ENABLE
        bool calibration_flag = cmd_payload[0];
        ranging_24_slave(calibration_flag);
#endif
    }
    break;

        // 0x24 // Start Master Ranging Execution
    case CMD_RANGING_RUN:
    {
        PRINTLN(F("CMD_RANGING_RUN"));
#if RF_24_ENABLE
        ranging_24_master_run(destination);
#endif
    }
    break;

        // 0x26 // Set SAT into SLAVE for ranging 24 calibration
    case CMD_RANGING_CALIB:
    {
        PRINTLN(F("CMD_RANGING_CALIB"));
#if RF_24_ENABLE
        ranging_24_master_calibration();
#endif
    }
    break;

        // 0x27 // Set SAT into SLAVE for ranging 24 calibration
    case CMD_SET_DATE_TIME:
    {
        PRINTLN(F("CMD_SET_DATE_TIME"));
#if RTC_ENABLE
        // cmd_payload[0-4] = hmdmy. dmy > 0
        if (cmd_payload[2] > 0 && cmd_payload[3] > 0 && cmd_payload[4] > 0)
        {
            tmElements_t tme;
            tme.Hour = cmd_payload[0];
            tme.Minute = cmd_payload[1];

            tme.Day = cmd_payload[2];
            tme.Month = cmd_payload[3];
            tme.Year = cmd_payload[4];

            Teensy3Clock.set(makeTime(tme));
            setTime(makeTime(tme)); // set Ardino system clock to compiled time

            digitalClockDisplay();
        }
#endif
    }
    break;

        // 0x28 reset date and time
    case CMD_RST_DATE_TIME:
    {
        PRINTLN(F("CMD_RST_DATE_TIME"));
#if RTC_ENABLE
        rtc_setup();
        digitalClockDisplay();
#endif
    }
    break;

        // 0x29 FSK beacon
    case CMD_SEND_BEACON_FSK:
    {
        PRINTLN(F("CMD_SEND_BEACON_FSK"));
        uint16_t file_count = 0;
        if (sdcard_is_on)
        {
#if SD_ENABLE
            file_count = sd_ls(&sd_data_g);
#endif
        }

        if (file_count < 1 || !sdcard_is_on || save_to_flash)
        {
            if (flash_is_on)
            {
#if TNSYFS_ENABLE
                fs_ls(&sd_data_g);
#endif
            }
        }

        handle_beacon(beacon_type_long, true);
        // switch to GFSK mode.
        set_modem(modem_fsk_433);
        // send beacon
        send_beacon(beacon_type_long, lora_433);
    }
    break;

        // 0x4D set overide sve to flash
    case CMD_SET_SAVE_TO_FLASH:
    {
        PRINTLN(F("CMD_SET_SAVE_TO_FLASH"));
        save_to_flash = cmd_payload[0];
    }
    break;

        // 0x55 // RPI Command
    case CMD_RPI_1_COMMAND:
    {
        PRINTLN(F("2. CMD_RPI_1_COMMAND"));
#if RPI_ENABLE
        bool rpi1_command_ack = rpi_mission(command_data, rpi1_serial);
        if (rpi1_command_ack)
        {
            rpi1_command_noack_flag = false;
        }
#endif
    }
    break;

        // 0x56 // RPI turn on in X minutes command
    case CMD_RPI_1_COMMAND_X_MNT:
    {
        PRINTLN(F("CMD_RPI_1_COMMAND_X_MNT"));
#if RPI_ENABLE
        should_turn_rpi1_delay_flag = true;
        turn_rpi1_delay = millis();
        // 0~1: reserved for x minutes
        turn_rpi1_msec = ((cmd_payload[0] << 8) + cmd_payload[1]) * MINUTE_1;
        PRINT(F("turn_rpi1_msec:\t"));
        PRINTLN(turn_rpi1_msec);

        memcpy(command_data_rpi1_g.packet, command_data->packet, COMMAND_HEADER_LENGTH + cmd_size);
        command_data_rpi1_g.cmd_code = CMD_RPI_1_COMMAND;
#endif
    }
    break;

    default:
        PRINT(F("Info: Unknown Command: \t"));
        PRINTLN(cmd_code, HEX);
        break;
    }
    return CMD_COMPLETED;
}
