/**
    @file RPI_Integration.ino
    @brief Integration with RPI for SATLLA0.

    This file contains functions to send and receive data from the RPI for the SATLLA0.

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

#if RPI_ENABLE

// Defines
#define RPI_BUSY 0x00
#define RPI_READY 0x01

#define RPI_NO_DATA 0x00
#define RPI_ACK 0x03
#define RPI_NONE 0x09

#define RPI_GET_STATE 0x01
#define RPI_GET_DATA 0x02
#define RPI_MISSION 0x03
#define RPI_EOL '\n'

#define RPI_SERIAL_BAUDRATE 115200

// #define SERIAL1_RX_BUFFER_SIZE 255

/* ============ */
/* Setup          */
/* ============ */

void rpi_serial_setup()
{
    PRINTLN("Func: rpi_serial_setup()");
    rpi1_serial.begin(RPI_SERIAL_BAUDRATE);
    rpi1_serial.setTimeout(SEC_1);
}

/* ============ */
/* FAT          */
/* ============ */

/* RPI1 */
void rpi1_set_state(bool state)
{
    PRINTLN("Func: rpi1_set_state()");
    if (state)
    {
        rpi1_turn_on();
    }
    else
    {
        rpi1_turn_off();
    }
}

void rpi1_turn_on()
{
    PRINTLN("Func: rpi1_turn_on()");
    digitalWrite(FAT_RPI_1_PIN, HIGH);
    rpi1_is_on = millis();
    mission_rpi_flag = true;
}

void rpi1_turn_off()
{
    PRINTLN("Func: rpi1_turn_off()");
    digitalWrite(FAT_RPI_1_PIN, LOW);
    rpi1_is_on = 0;

    // if RPI is turned off then no mission
    mission_rpi_flag = false;
}

/* ============ */
/* RPI Status   */
/* ============ */

void check_rpi_status(HardwareSerial &refSer)
{
    PRINTLN("Func: check_rpi_status()");
    // if (refSer.availableForWrite())
    // {
    uint8_t bytes_sent = 0;
    uint8_t input_type = 0;
    int bytes_received = 0;
    uint16_t incoming_bytes = 0;
    uint16_t file_index = 0;
    uint16_t mission = 0x00;
    uint8_t file_type = 0;

    // clear the buffer
    rpi_serial_flush(refSer);
    // 1. ask for status.
    PRINTLN("1. RPI: RPI_GET_STATE");
    bytes_sent = refSer.write(RPI_GET_STATE);
    bytes_sent = refSer.write(RPI_EOL);
    delay(SEC_1);

    // 2. if recevied answer within 1 sec, check. else, return.
    if (refSer.available() > 0)
    {
        // read the incoming byte:
        incoming_bytes = refSer.read();
        PRINT("2. RPI: INCOMING BYTES:\t");
        PRINTLN(incoming_bytes);

        // 3. if the RPI is ready, ask for data. else, skip.
        // 0: Busy, 1: Ready
        if (incoming_bytes == RPI_READY)
        {
            PRINTLN("3. RPI: RPI_READY");

            unsigned long start_transfer = millis();
            int get_data_tries = 0;
            while (millis() - start_transfer < SECS_30) // add timeout check
            {
                // rpi_serial_flush(refSer);
                // delay(TENTH_SEC);
                // 4. ask for data (0x02)
                PRINTLN("4. RPI: RPI_GET_DATA");
                bytes_sent = refSer.write(RPI_GET_DATA);
                bytes_sent = refSer.write(RPI_EOL);
                delay(SECS_3); // looks like 15000 bytes beeds more time.
                bytes_received = refSer.available();
                // 5. if data received, then save to file.
                if (bytes_received > 0)
                {
                    PRINT("5. RPI: BYTES_RECEIVED:\t");
                    PRINTLN(bytes_received);
                    if (bytes_received > LRG_BUFFER_SIZE_MAX)
                    {
                        continue;
                    }

                    if (bytes_received == 1)
                    {
                        incoming_bytes = refSer.read();

                        PRINT("5. RPI: INCOMING BYTES:\t");
                        PRINTLN(incoming_bytes);
                        if (incoming_bytes == RPI_NO_DATA || incoming_bytes == RPI_NONE)
                        {
                            PRINTLN("6. RPI: RPI_NO_DATA/RPI_NONE");
                            rpi_task_completed_flag = true;
                            // clear the buffer
                            rpi_serial_flush(refSer);
                            return;
                        }
                    }
                    else if (bytes_received > 1)
                    {
                        // start getting files
                        memset(lrg_buffer_g, 0x00, LRG_BUFFER_SIZE_MAX);
                        incoming_bytes = refSer.readBytes(lrg_buffer_g, bytes_received);
                        PRINT("5. RPI: INCOMING BYTES:\t");
                        PRINTLN(incoming_bytes);

                        if (incoming_bytes > 3)
                        {
                            mission = (lrg_buffer_g[1] << 8) + lrg_buffer_g[0];
                            PRINT("6. Mission:\t");
                            PRINTLN(mission, DEC);

                            file_type = (lrg_buffer_g[2]);
                            PRINT("7. File type:\t");
                            PRINTLN(file_type, DEC);

                            if (file_type == ld_type_meta)
                            {
#ifdef DEBUG
                                print_buffer(lrg_buffer_g, incoming_bytes);
#endif
                                // save meta include header.
                                uint8_t error = 0;
                                if (sdcard_is_on)
                                {
#if SD_ENABLE
                                    error = save_meta_file(lrg_buffer_g, incoming_bytes, mission);
#endif
                                }

                                if (error || !sdcard_is_on || save_to_flash)
                                {
                                    if (flash_is_on)
                                    {
#if TNSYFS_ENABLE
                                        fs_save_meta_file(lrg_buffer_g, incoming_bytes, mission);
#endif
                                    }
                                }

                                // save the mission number in the last index file
                                if (sdcard_is_on)
                                {
#if SD_ENABLE
                                    error = set_misson_index(mission);
#endif
                                }

                                if (error || !sdcard_is_on || save_to_flash)
                                {
                                    if (flash_is_on)
                                    {
#if TNSYFS_ENABLE
                                        fs_set_misson_index(mission);
#endif
                                    }
                                }
                            }
                            else
                            {
                                // save all other files under outbox
                                file_index = (file_type != ld_type_text) ? file_type : file_index_g++;
                                uint8_t error = 0;
                                if (sdcard_is_on)
                                {
#if SD_ENABLE
                                    error = save_large_file(lrg_buffer_g + 3, incoming_bytes - 3, mission, file_index);
#endif
                                }
                                if (error || !sdcard_is_on || save_to_flash)
                                {
                                    if (flash_is_on)
                                    {
#if TNSYFS_ENABLE
                                        fs_save_large_file(lrg_buffer_g + 3, incoming_bytes - 3, mission, file_index);
#endif
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    PRINTLN("8. RPI: RPI_DONE");
                    // clear the buffer
                    rpi_serial_flush(refSer);
                    get_data_tries++;
                    if (get_data_tries > 2)
                    {
                        // assume no more data
                        rpi_task_completed_flag = true;
                        return;
                    }
                }
            }
        }
        else /*Busy*/
        {
            PRINTLN("3. RPI: RPI_BUSY");
            // clear the buffer
            rpi_serial_flush(refSer);
        }
    }
    else /*Busy*/
    {
        PRINTLN("2. RPI: RPI_BUSY");
        // clear the buffer
        rpi_serial_flush(refSer);
    }
}

bool rpi_mission(command_data_t *command_data, HardwareSerial &refSer)
{
    PRINTLN("Func: rpi_mission()");

    bool command_ack = false;
    // sat_state = sat_mission;

    uint8_t cmd_code = command_data->cmd_code;
    uint8_t cmd_type = command_data->cmd_type;
    uint8_t cmd_size = command_data->cmd_size;
    uint8_t cmd_payload[COMMAND_DATA_LENGTH] = {0};

    if (cmd_size > 0)
    {
        memcpy(cmd_payload, command_data->cmd_payload, cmd_size);
    }
    else
    {
        return; // no command arrived.
    }

    // if (refSer.availableForWrite())
    // {
    PRINTLN("RPI: Serial Available");
    uint8_t bytes_sent = 0;
    uint8_t input_type = 0;
    int bytes_received = 0;
    uint16_t incoming_bytes = 0;

    // clear the buffer
    rpi_serial_flush(refSer);
    delay(TENTH_SEC);
    // 1. ask for status.
    PRINTLN("1. RPI: RPI_GET_STATE");
    bytes_sent = refSer.write(RPI_GET_STATE);
    bytes_sent = refSer.write(RPI_EOL);
    delay(SEC_1);

    // 2. if recevied answer within 0.5 sec, check. else, return.
    if (refSer.available() > 0)
    {
        // read the incoming byte:
        incoming_bytes = refSer.read();
        PRINT("2. RPI: INCOMING BYTES:\t");
        PRINTLN(incoming_bytes);

        // 0: Busy, 1: Ready
        if (incoming_bytes == RPI_READY)
        {
            PRINTLN("3. RPI: RPI_READY");
            // clear the buffer
            rpi_serial_flush(refSer);
            delay(TENTH_SEC);

            // CMD_NONE = 0
            // CMD_GET_STATE = 1
            // CMD_GET_DATA = 2
            // CMD_POWER_ON = 3
            // CMD_TAKE_PHOTO = 4  # take photo (smart)
            // CMD_SIT = 5  # Satellite Image Transfer
            // CMD_SEEK_LASER = 6  # seek laser
            // CMD_RWCS = 7  # Reaction Wheel Control System
            // # technical missions
            // CMD_POWER_OFF = 8
            // CMD_DROP_OUTBOX = 9

            PRINT("4. RPI: SEND MISSION:\t");
            memset(buffer_g, 0x00, BUFFER_SIZE_MAX);
            for (uint8_t i = 0; i < cmd_size; i++)
            {
                if (cmd_payload[i] < 16)
                {
                    PRINT(0);
                }
                PRINT(cmd_payload[i], HEX);
                refSer.write(cmd_payload[i]);
                // sprintf(buffer_g + (i * 2), "%02X", cmd_payload[i]);
            }
            delay(TENTH_SEC);
            bytes_sent = refSer.write(RPI_EOL);
            PRINTLN();
            // uint8_t len = (cmd_size * 2) + 2;
            // sprintf(buffer_g + len, "\n");
            // bytes_sent = refSer.write(buffer_g, len);
            delay(SEC_1);
            if (refSer.available() > 0)
            {
                input_type = refSer.read();
                // If != None there is data to be sent
                // if (input_type == RPI_ACK) //0x08
                // {
                PRINTLN("5. RPI: ANSWER = RPI_ACK");
                // Recevied Ack
                command_ack = true;
                // }
            }
            else
            {
                PRINT("5. RPI: ANSWER = NOACK. input_type=");
                PRINTLN(input_type);
                // Didn't recevied Ack
                // TODO: Need to do something
                command_ack = false;
            }
        }
        else
        {
            // Busy. Skip.
        }
    }
    else
    {
        PRINTLN("RPI: no bytes recevied");
        // N/A. Skip.
    }
    // }
    PRINTLN("Func: rpi_mission(): End");
    return command_ack;
}

void rpi_serial_flush(HardwareSerial &refSer)
{
    PRINTLN("Func: rpi_serial_flush()");
    refSer.flush();
    if (refSer.available() > 0)
    {
        uint8_t byte_received = refSer.readBytes(buffer_g, LD_PACKET_LENGTH);
        PRINT("byte_received:\t");
        PRINTLN(byte_received);
    }
}

bool prepare_and_send_ld_packet(uint16_t mission, uint16_t file_index, uint8_t message_len, uint8_t radio)
{
    PRINTLN("Func: prepare_and_send_ld_packet");
    return prepare_and_send_ld_packet_part(mission, file_index, message_len, 0, radio);
}

bool prepare_and_send_ld_packet_part(uint16_t mission, uint16_t file_index, uint8_t message_len, uint16_t part_number, uint8_t radio)
{
    PRINTLN("Func: prepare_and_send_ld_packet_part");
    bool ret_value = false;
    // reset global buffer
    memset(lrg_buffer_g, 0x00, LRG_BUFFER_SIZE_MAX);
    int file_size = 0;
    if (sdcard_is_on)
    {
#if SD_ENABLE
        file_size = read_large_file(lrg_buffer_g, mission, file_index);
#endif
    }

    if (!file_size || !sdcard_is_on || save_to_flash)
    {
        if (flash_is_on)
        {
#if TNSYFS_ENABLE
            file_size = fs_read_large_file(lrg_buffer_g, mission, file_index);
#endif
        }
    }
    PRINT("file_size:\t");
    PRINTLN(file_size);
    uint8_t data_size = message_len - LD_HEADER_LENGTH;
    PRINT("data_size:\t");
    PRINTLN(data_size);
    PRINT("Total Messages:\t");
    PRINTLN(file_size / data_size);
    if (file_size > 0)
    {
        // if we need specific file part, else, send the entire file in parts
        if (part_number > 0)
        {
            PRINT("File Idx:\t");
            PRINTLN(part_number);

            uint16_t packets = file_size / data_size;
            // if requested unexisting part then return.
            if (part_number > packets)
            {
                return ret_value;
            }
            uint8_t reminder = file_size % data_size;
            // if requested the last part
            if (part_number == packets)
            {
                PRINT("Reminder:\t");
                PRINTLN(reminder);
                data_size = reminder;
            }

            memset(ld_data_g.packet, 0x00, LD_PACKET_LENGTH);
            ld_data_g.ld_file_index = mission;
            ld_data_g.ld_file_seq = part_number;
            ld_data_g.ld_type = ld_type_image; // image
            ld_data_g.ld_size = data_size;
            memcpy(ld_data_g.ld_payload, lrg_buffer_g + (part_number * data_size), data_size);

            send_ld_packet(radio);

            // wathc dog
#if WD_ENABLE
            wd_heartbeat();
#endif
        }
        else
        {
            uint16_t packets = file_size / data_size;

            for (int i = 0; i < packets; i++)
            {
                PRINT("File Idx:\t");
                PRINTLN(i);

                memset(ld_data_g.packet, 0x00, LD_PACKET_LENGTH);
                ld_data_g.ld_file_index = mission;
                ld_data_g.ld_file_seq = i;
                ld_data_g.ld_type = ld_type_image; // image
                ld_data_g.ld_size = data_size;
                memcpy(ld_data_g.ld_payload, lrg_buffer_g + (i * data_size), data_size);

                send_ld_packet(radio);

                // wathc dog
#if WD_ENABLE
                wd_heartbeat();
#endif
            }

            // if there is a reminder, add 1.
            uint8_t reminder = file_size % data_size;
            PRINT("Reminder:\t");
            PRINTLN(reminder);

            if (reminder)
            {
                PRINT("File Idx:\t");
                PRINTLN(packets);

                memset(ld_data_g.packet, 0x00, LD_PACKET_LENGTH);
                ld_data_g.ld_file_index = mission;
                ld_data_g.ld_file_seq = packets;
                ld_data_g.ld_type = ld_type_image; // image
                ld_data_g.ld_size = reminder;
                memcpy(ld_data_g.ld_payload, lrg_buffer_g + (packets * data_size), reminder);

                send_ld_packet(radio);

                // wathc dog
#if WD_ENABLE
                wd_heartbeat();
#endif
            }
            ret_value = true;
        }
    }
    return ret_value;
}

void send_ld_meta_packet(lorafreq_e freq)
{
    PRINTLN("Func: send_ld_meta_packet()");
    memset(rpi_packet_g.packet, 0x00, LORA_PACKET_LENGTH);
    rpi_packet_g.local_address = local_address;
    rpi_packet_g.destination = destination;
    rpi_packet_g.msg_type = type_rpi;
    rpi_packet_g.msg_ack_req = 0;

    while (!LDMETATXbuffer.isEmpty())
    {
        LDMETATXbuffer.pull(&ld_meta_data_g);
        rpi_packet_g.msg_size = ld_meta_data_g.ld_size; // already include RPI_HEADER_LENGTH
        rpi_packet_g.msg_index = tx_counter;
        rpi_packet_g.msg_time = millis() / SEC_1;

        memcpy(rpi_packet_g.msg_payload, ld_meta_data_g.packet, LD_META_HEADER_LENGTH + ld_meta_data_g.ld_size);

        if (freq == lora_24)
        {
#if RF_24_ENABLE
            send_message_lora_24((uint8_t *)rpi_packet_g.packet, LORA_HEADER_LENGTH + rpi_packet_g.msg_size);
#endif
        }
        else
        {
#if RF_433_ENABLE
            send_message_lora_433((uint8_t *)rpi_packet_g.packet, LORA_HEADER_LENGTH + rpi_packet_g.msg_size);
#endif
        }
        // wathc dog
#if WD_ENABLE
        wd_heartbeat();
#endif
        delay(TEN_MS);
        PRINT("LDMETATXbuffer Element:\t");
        PRINTLN(LDMETATXbuffer.numElements());
    }
}

void send_ld_packet(lorafreq_e freq)
{
    PRINTLN("Func: send_ld_packet()");
    memset(rpi_packet_g.packet, 0x00, LORA_PACKET_LENGTH);
    rpi_packet_g.local_address = local_address;
    rpi_packet_g.destination = destination;
    rpi_packet_g.msg_type = type_rpi;
    rpi_packet_g.msg_ack_req = 0;
    rpi_packet_g.msg_size = ld_data_g.ld_size; // already include RPI_HEADER_LENGTH
    rpi_packet_g.msg_index = tx_counter;
    rpi_packet_g.msg_time = millis() / SEC_1;

    memcpy(rpi_packet_g.msg_payload, ld_data_g.packet, LD_HEADER_LENGTH + ld_data_g.ld_size);

    if (freq == lora_24)
    {
#if RF_24_ENABLE
        send_message_lora_24((uint8_t *)rpi_packet_g.packet, LORA_HEADER_LENGTH + rpi_packet_g.msg_size);
#endif
    }
    else
    {
#if RF_433_ENABLE
        send_message_lora_433((uint8_t *)rpi_packet_g.packet, LORA_HEADER_LENGTH + rpi_packet_g.msg_size);
#endif
    }

    delay(TEN_MS);
}

#endif