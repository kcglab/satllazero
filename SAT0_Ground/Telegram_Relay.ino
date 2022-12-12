/**
    @file Telegram_Relay.ino
    @brief Telegram relay utils file function for SATLLA0 GS.

    Telegram implementation to send and receive messages from Telegram bot.

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
Telegram Bot bot_commands
 relay - Relay a message: </relay:msg>
 delay - Relay a message with time delay: </time:mnt,message>
*/


#if TELEGRAM_BOT_ENABLE && WIFI_ENABLE

#include <HTTPClient.h>
#include "CTBot.h"

CTBot relayBot;
// TELEGRAM BOT TOKEN
String relay_bot_token = "mytokenid:token";

#define BOT_COMMANDS_SUM 1
char bot_commands[BOT_COMMANDS_SUM][11] = {"relay"};

/* =============== */
/* Setup           */
/* =============== */

void tlgrelay_setup()
{
    // connect the ESP8266 to the desired access point
    // relayBot.wifiConnect(WFSSID1, WFPASS1);

    // set the telegram bot token
    relayBot.setTelegramToken(relay_bot_token);

    // check if all things are ok
    if (relayBot.testConnection())
    {
        Serial.println("\ntestConnection OK");
    }
    else
    {
        Serial.println("\ntestConnection NOK");
    }

    randomSeed(analogRead(0));
}

/* =============== */
/* Read            */
/* =============== */

void tlgrelay_read()
{
    // PRINTLN("Func: tlg_read()");
    wifi_connect();

    // a variable to store telegram message data
    TBMessage msg;
    // check if there is a new incoming message
    if (relayBot.getNewMessage(msg))
    {
        // check if the message is a text message
        if (msg.messageType == CTBotMessageText)
        {
            Serial.println(
                "\nFirstName: " + msg.sender.firstName +
                "\nID: " + msg.sender.id +
                "\nMessage: " + msg.text +
                "\nLength: " + msg.text.length());

            uint8_t message_length = msg.text.length();

            if (message_length > 0)
            {
                msg.text = msg.text.substring(1, message_length);
                int16_t idx = msg.text.indexOf(':');
                if (idx > 0)
                {
                    String command = msg.text.substring(0, idx);
                    if (command.startsWith("relay")) //
                    {
                        String relay_message = msg.text.substring(idx + 1, message_length);
                        memset(buffer_g, 0x00, COMMAND_PACKET_LENGTH);
                        relay_message.toCharArray(buffer_g + 1, relay_message.length() + 1);

                        build_command(msg.sender.id, CMD_ECHO_MESSAGE, (uint8_t *)buffer_g, relay_message.length() + 1);
                    }
                    if (command.startsWith("delay")) //
                    {
                        int16_t split_idx =  msg.text.indexOf(',');
                        String time_delay = msg.text.substring(idx + 1, split_idx);
                        String relay_message = msg.text.substring(split_idx + 1, message_length);
                        memset(buffer_g, 0x00, COMMAND_PACKET_LENGTH);
                        buffer_g[1] = atol(time_delay.c_str());
                        relay_message.toCharArray(buffer_g + 2, relay_message.length() + 1);

                        build_command(msg.sender.id, CMD_ECHO_MESSAGE_X_MNT, (uint8_t *)buffer_g, relay_message.length() + 2);
                    }
                }
            }
        }
    }
}


void build_command(int64_t target_id, uint8_t cmd_code, uint8_t *message, uint8_t message_length)
{
    PRINTLN("Func: build_command()");
    // PRINTLN(message);
    if (message_length < 2)
    {
        return;
    }

    command_data_t command_data;
    memset(command_data.packet, 0x00, COMMAND_PACKET_LENGTH);

    command_data.cmd_code = cmd_code;
    command_data.cmd_type = cmd_type_param;
    command_data.cmd_size = message_length;
    memcpy(command_data.cmd_payload, buffer_g, message_length);
    print_command(&command_data);

    lora_data_t send_command;
    message_prepare(&send_command, &command_data);

    String text = message2hex(send_command.packet, send_command.msg_size + LORA_HEADER_LENGTH);

    return_message(target_id, text);
}

void message_prepare(lora_data_t *send_command, command_data_t *command_data)
{
    PRINTLN("Func: message_prepare()");

    uint8_t cmd_code = command_data->cmd_code;
    uint8_t cmd_type = command_data->cmd_type;
    uint8_t cmd_size = command_data->cmd_size;
    uint8_t checksum = checksum_func(cmd_code, cmd_type, cmd_size);
    command_data->cmd_chksum = checksum;

    encrypt_decrypt(command_data);

    send_command->local_address = 0xFF;
    send_command->destination = 0xB2;
    send_command->msg_type = type_command;
    send_command->msg_size = COMMAND_HEADER_LENGTH + cmd_size;
    send_command->msg_index = tx_counter;
    send_command->msg_time = millis() / SEC_1;
    send_command->msg_ack_req = 0x00;
    memcpy(send_command->msg_payload, command_data->packet, COMMAND_HEADER_LENGTH + cmd_size);
}

void return_message(int64_t target_id, String message)
{
    PRINTLN("Func: return_message()");
    relayBot.setParseMode(CTBotParseModeHTML);
    String text = "Copy/Paste to Send TX on your station page as BINARY / \'Hex text\':\n" + message;
    relayBot.sendMessage(target_id, text);
    delay(500); // wait 500 milliseconds
}

#endif