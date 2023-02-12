/**
    @file Command_Exec.ino
    @brief Commands utils functions for SATLLA0 GS.
    
    This contains the commands file for the SATLLA0 GS.
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

void cmd_receive(command_data_t *command_data, uint8_t destination)
{
    PRINTLN("Func: cmd_receive()");

    uint8_t chkSum = code_checksum(command_data);
    command_data->cmd_chksum = chkSum;

    int commandStatus = activate_command(command_data, destination);
    // print_command(command_data);

    PRINT("Parse and Command Status: ");
    PRINTLN(commandStatus);
}

/* ============ */
/* Commands     */
/* ============ */

void cmd_send(command_data_t *command_data, uint8_t destination, lorafreq_e freq)
{
    PRINTLN("Func: cmd_send()");

    uint8_t cmd_code = command_data->cmd_code;
    uint8_t cmd_type = command_data->cmd_type;
    uint8_t cmd_size = command_data->cmd_size;
    uint8_t checksum = checksum_func(cmd_code, cmd_type, cmd_size);
    command_data->cmd_chksum = checksum;

    // #ifdef PRINT_FUNC_DEBUG
    print_command(command_data);
    // #endif

    // uint8_t key = millis() / SEC_1;
    encrypt_decrypt(command_data);

    // lora_data_t send_command;
    send_command_g.local_address = local_address;
    send_command_g.destination = destination;
    send_command_g.msg_type = type_command;
    send_command_g.msg_size = COMMAND_HEADER_LENGTH + cmd_size;
    send_command_g.msg_index = tx_counter;
    send_command_g.msg_time = millis() / SEC_1;
    send_command_g.msg_ack_req = 0x01;
    memcpy(send_command_g.msg_payload, command_data->packet, COMMAND_HEADER_LENGTH + cmd_size);

    // send_message_lora_433(send_command_g.packet, LORA_HEADER_LENGTH + COMMAND_HEADER_LENGTH + command_data->cmd_size);

    if (freq == lora_24)
    {
#if RF_24_ENABLE
        send_message_lora_24(send_command_g.packet, LORA_HEADER_LENGTH + COMMAND_HEADER_LENGTH + cmd_size);
#endif
    }
    else if (freq == lora_gs)
    {
#if TINYGS_ENABLE && WIFI_ENABLE
        tinygs_post(send_command_g.packet, LORA_HEADER_LENGTH + COMMAND_HEADER_LENGTH + cmd_size);
#endif
    }
    else
    {
#if RF_433_ENABLE
        send_message_lora_433(send_command_g.packet, LORA_HEADER_LENGTH + COMMAND_HEADER_LENGTH + cmd_size);
#endif
    }

    encrypt_decrypt(command_data);
    activate_send_command(command_data, destination);
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
    PRINTLN("Func: code_checksum()");
    uint8_t cmd_code = command_data->cmd_code;
    uint8_t cmd_type = command_data->cmd_type;
    uint8_t cmd_size = command_data->cmd_size;

    uint8_t checksum = checksum_func(cmd_code, cmd_type, cmd_size);
    PRINT("ANS: ");
    PRINTLN(checksum, HEX);
    return checksum;
}

bool check_if_msg_correct(command_data_t *command_data)
{
    PRINTLN("Func: check_if_msg_correct()");

    encrypt_decrypt(command_data);
    // print_command(command_data);

    uint8_t cmd_code = command_data->cmd_code;
    uint8_t cmd_type = command_data->cmd_type;
    uint8_t cmd_size = command_data->cmd_size;
    uint8_t checksum = command_data->cmd_chksum;

    uint8_t ans = checksum_func(cmd_code, cmd_type, cmd_size);
    PRINT("ANS: ");
    PRINTLN(ans, HEX);
    PRINT("CKS: ");
    PRINTLN(checksum, HEX);
    return (checksum == ans);
}

/* ================= */
/* Encrypt Decrypt   */
/* ================= */

void encrypt_decrypt(command_data_t *command_data)
{
    PRINTLN("Func: encrypt_decrypt()");

    for (uint8_t i = 0; i < COMMAND_HEADER_LENGTH - 1; i++)
    {
        command_data->packet[i] = command_data->packet[i] ^ key[i % KEY_LENGTH];
    }
}

int parse_and_activate_command(command_data_t *command_data, uint8_t destination)
{
    PRINTLN("Func: parse_and_activate_command()");
    cmdCounter++;
    bool checksum = check_if_msg_correct(command_data);
    if (!checksum) // there is error in checksum. Msg is invalid!
    {
        PRINTLN("Error: Message checksum incorrect!");
        return MSG_CHKSUN_ERR;
    }
    cmdGoodCounter++;
    int commandStatus = activate_command(command_data, destination);
    return commandStatus;
}

int activate_send_command(command_data_t *command_data, uint8_t destination)
{
    PRINTLN("Func: activate_send_command()");
    // #ifdef PRINT_FUNC_DEBUG
    //     print_command(command_data);
    // #endif
    uint8_t cmd_code = command_data->cmd_code;
    uint8_t cmd_type = command_data->cmd_type;
    uint8_t cmd_size = command_data->cmd_size;
    uint8_t checksum = command_data->cmd_chksum;
    uint8_t cmd_payload[COMMAND_DATA_LENGTH] = {0x00};

    if (cmd_size > 0)
    {
        memcpy(cmd_payload, command_data->cmd_payload, cmd_size);
    }

    switch (cmd_code)
    {
    case CMD_RANGING_SLAVE_24:
    {
#if RF_24_ENABLE
        ranging_24_master(true, 2);
#endif
    }
    break;

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

    default:
        break;
    }
    return CMD_COMPLETED;
}

int activate_command(command_data_t *command_data, uint8_t destination)
{
    PRINTLN("Func: activate_command()");
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

    switch (cmd_code)
    {
    case CMD_CPU_RESTART:
    {
        restart_requested_flag = millis();
    }
    break;

    case CMD_RANGING_RUN:
    {
#if RF_24_ENABLE
        ranging_24_master_run(destination);
#endif
    }
    break;

    case CMD_RANGING_CALIB:
    {
#if RF_24_ENABLE
        ranging_24_master_calibration(destination);
#endif
    }
    break;

    case CMD_RANGING_SLAVE_24:
    {
#if RF_24_ENABLE
        bool calibration_flag = cmd_payload[0];
        ranging_24_slave(calibration_flag);
#endif
    }
    break;

    default:
        break;
    }
    return CMD_COMPLETED;
}
