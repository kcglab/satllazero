/**
    @file LoRa_RL_SX1278.ino
    @brief LoRa 433 utils functions for SATLLA0 GS.

    This file contains LoRa 433 Radio functionality for the SATLLA0 GS.

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

#if LORA_RL_SX1278_ENABLE

#define SX127X_FSK_OOK 0b00000000 //  7     7     FSK/OOK mode
#define SX127X_LORA 0b10000000    //  7     7     LoRa mode

#include <RadioLib.h>

// SX1278 has the following connections:
// NSS, DIO0, RESET, DIO1
SX1278 radio_433_1 = new Module(LORA_433_NSS_PIN, LORA_433_DIO0_PIN, LORA_433_RST_PIN, -1);

uint8_t lora_433_tx_buf[LORA_RX_TX_BYTE];
uint8_t lora_433_tx_buf_size = 0;
uint8_t lora_433_rx_buf[LORA_RX_TX_BYTE];
uint8_t lora_433_rx_buf_size = 0;

#define BW_433_SIZE 10
const float bw_433_table[] = {7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125, 250, 500};
/* ============ */
/* FAT          */
/* ============ */

void lora_433_set_state(bool state)
{
    PRINTLN("Func: lora_433_set_state()");
    if (state)
    {
        lora_433_turn_on();
    }
    else
    {
        lora_433_turn_off();
    }
}

void lora_433_turn_on()
{
    PRINTLN("Func: lora_433_turn_on()");
    //     digitalWrite(FAT_LORA_433_PIN, HIGH);
    lora_433_is_on = millis();
    lora_433_is_sleep = 0;
}

void lora_433_turn_off()
{
    PRINTLN("Func: lora_433_turn_off()");
    //     digitalWrite(FAT_LORA_433_PIN, LOW);
    lora_433_is_on = 0L;
    lora_433_is_sleep = 0;
}

void lora_433_sleep()
{
    PRINTLN("Func: lora_433_sleep()");
    if (!lora_433_is_on)
    {
        return;
    }
    radio_433_1.sleep();
    lora_433_is_sleep = 1;
}

void lora_433_resume()
{
    PRINTLN("Func: lora_433_resume()");
    if (!lora_433_is_on)
    {
        return;
    }
    uint8_t status = radio_433_1.getModemStatus();
    if (status == 0x00)
    {
        PRINTLN("Func: lora_433_resume()::modem failed");
        radio_433_1 = new Module(LORA_433_NSS_PIN, LORA_433_DIO0_PIN, LORA_433_RST_PIN, -1);
        lora_433_setup();
    }
    else
    {
        radio_433_1.startReceive();
    }
    lora_433_is_sleep = 0;
}

/* ============ */
/* Setup        */
/* ============ */

void lora_433_setup()
{
    PRINTLN(F("radio_433_1: LoRa Setup"));

    // carrier frequency:           915.0 MHz
    // bandwidth:                   500.0 kHz
    // spreading factor:            6
    // coding rate:                 5
    // sync word:                   0x14
    // output power:                2 dBm
    // preamble length:             20 symbols
    // amplifier gain:              1 (maximum gain)

    // int state = radio_433_1.begin(437.250,62.5,10,5,0x2a,20,8,1);
    int state = radio_433_1.begin();
    if (state == RADIOLIB_ERR_NONE)
    {
        PRINTLN("radio_433_1: Setup Successed.");

        // call the following method
        // RX enable:   4
        // TX enable:   5
        // radio_433_1.setRfSwitchPins(LORA_433_RX_EN_PIN, LORA_433_TX_EN_PIN);
        setup_lora_433_default();
        // set the function that will be called
        // when new packet is received
        radio_433_1.setDio0Action(on_receive_flag);

        lora_433_is_on = millis();
    }
    else
    {
        PRINTLN("radio_433_1: Setup Failed!");
        // while (1); //TODO: Revised
        // lora_433_turn_off();

        PRINT(F("radio_433_1 433: Failed: code:"));
        PRINTLN(state);

        lora_433_is_on = 0L;
    }
}

void setup_lora_433_default()
{
    PRINTLN(F("Func: setup_lora_433_default()"));

    int state = radio_433_1.begin();
    if (state == RADIOLIB_ERR_NONE)
    {
        PRINTLN("[FSK] radio_433_1: Setup Successed.");

        modem_lora_433_setup();

        setup_lora_433(&modem_lora_433_g);
        radio_433_1.setDio0Action(on_receive_flag);

        lora_433_is_on = millis();
    }
}

void setup_lora_433(modem_data_t *modem_data)
{
    PRINTLN(F("Func: setup_lora_433()"));

    double freq_e = modem_data->modem_frequency;
    if (freq_e < 434E6 || freq_e > 470E6)
    {
        freq_e = LORA_433_BAND;
    }

    if (modem_data->modem_offset < -500E3 || modem_data->modem_offset > 500E3)
    {
        modem_data->modem_offset = LORA_433_OFFSET;
    }

    float bwe = 62.5;
    if (modem_data->modem_bw < 0 || modem_data->modem_bw > BW_433_SIZE)
    {
        bwe = bw_433_table[modem_data->modem_bw];
    }

    PRINTLN(F("LoRa SX127X: "));
    print_modem(modem_data);

    int state = 0;

    // set carrier frequency to 437.250 MHz
    if ((state = radio_433_1.setFrequency(freq_e / 1E6)) == RADIOLIB_ERR_INVALID_FREQUENCY)
    {
        PRINTLN(F("Error: Selected frequency is invalid for this module!"));
    }

    // -- no set offset.

    // set bandwidth to 62.5 kHz
    if ((state = radio_433_1.setBandwidth(bwe)) == RADIOLIB_ERR_INVALID_BANDWIDTH)
    {
        PRINTLN(F("Error: radio_433_1.setBandwidth()"));
    }

    // set spreading factor to 10
    if ((state = radio_433_1.setSpreadingFactor(modem_data->modem_sf)) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR)
    {
        PRINTLN(F("Error: radio_433_1.setSpreadingFactor()"));
    }

    // set coding rate to 5
    if ((state = radio_433_1.setCodingRate(modem_data->modem_cr)) == RADIOLIB_ERR_INVALID_CODING_RATE)
    {
        PRINTLN(F("Error: radio_433_1.setCodingRate()"));
    }

    // set LoRa sync word to 0x2A
    if ((state = radio_433_1.setSyncWord(modem_data->modem_sw)) == RADIOLIB_ERR_INVALID_SYNC_WORD)
    {
        PRINTLN(F("Error: radio_433_1.setSyncWord()"));
    }

    // set output power to 10 dBm (accepted range is -3 - 17 dBm)
    // NOTE: 20 dBm value allows high power operation, but transmission
    //       duty cycle MUST NOT exceed 1%
    if ((state = radio_433_1.setOutputPower(modem_data->modem_power)) == RADIOLIB_ERR_INVALID_OUTPUT_POWER)
    {
        PRINTLN(F("Error: radio_433_1.setOutputPower()"));
    }

    // set over current protection limit to 80 mA (accepted range is 45 - 240 mA)
    // NOTE: set value to 0 to disable overcurrent protection
    if ((state = radio_433_1.setCurrentLimit(240)) == RADIOLIB_ERR_INVALID_CURRENT_LIMIT)
    {
        PRINTLN(F("Error: radio_433_1.setCurrentLimit()"));
    }

    // set LoRa preamble length to 8 symbols (accepted range is 6 - 65535)
    if ((state = radio_433_1.setPreambleLength(modem_data->modem_pl)) == RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH)
    {
        PRINTLN(F("Error: radio_433_1.setPreambleLength()"));
    }

    // set amplifier gain to 1 (accepted range is 1 - 6, where 1 is maximum gain)
    // NOTE: set value to 0 to enable automatic gain control
    //       leave at 0 unless you know what you're doing
    if ((state = radio_433_1.setGain(1)) == RADIOLIB_ERR_INVALID_GAIN)
    {
        PRINTLN(F("Error: radio_433_1.setGain()"));
    }

    bool ldro_e = (modem_data->modem_ldro != 0);
    if ((state = radio_433_1.forceLDRO(ldro_e)) == RADIOLIB_ERR_WRONG_MODEM)
    {
        PRINTLN(F("Error: radio_433_1.forceLDRO()"));
    }

    if (state != RADIOLIB_ERR_NONE)
    {
        PRINT(F("radio_433_1 433: Unable to set configuration, code :"));
        PRINTLN(state);

        fsk_433_is_on = 0L;
    }
    else
    {
        radio_433_1.startReceive();
    }
}

void setup_lora_433_rx()
{
    // put the radio into receive mode
    received_433_flag = false; // somehow turened on during send.
    radio_433_1.setDio0Action(on_receive_flag);
    radio_433_1.startReceive();
}

/* ============ */
/* Tx 433     */
/* ============ */

bool send_message_lora_433(uint8_t *message, uint8_t messageLength)
{
    PRINTLN("Func: send_message_lora_433()");
    bool error = false;

    if (!lora_433_is_on)
    {
        PRINTLN(F("LoRa 433: Sending Failed!"));
        PRINTLN(F("LoRa 433: Radio is off!"));
        error = true;
        return error;
    }

    // turn off dio0 so onReceive won't work
    radio_433_1.clearDio0Action();

    // reset RF params
    rssi_g = 0;
    snr_g = 0;

    lora_433_tx_buf_size = messageLength;
    if (lora_433_tx_buf_size > 0 && lora_433_tx_buf_size < 70)
    {
        lora_433_tx_buf_size += 10;
    }

    // lora_433_enable_interrupt = false;
    memset(lora_433_tx_buf, 0x00, LORA_RX_TX_BYTE);
    memcpy(lora_433_tx_buf, message, messageLength);
    print_message(lora_433_tx_buf, messageLength);

    unsigned long start = millis();
    // send packet
    PRINTLN(F("LoRa 433: transmit()"));
    int state = radio_433_1.transmit(lora_433_tx_buf, lora_433_tx_buf_size);
    // int state = radio_433_1.startTransmit(lora_433_tx_buf, lora_433_tx_buf_size);

    PRINT("Sending Time():\t");
    PRINT(millis() - start);
    PRINTLN(" ms");
    PRINTLN();

    if (state == RADIOLIB_ERR_NONE)
    {
        // the packet was successfully transmitted
        tx_counter++;
        PRINTLN(F("LoRa 433: Sending Succeed!"));
    }
    else if (state == RADIOLIB_ERR_PACKET_TOO_LONG)
    {
        // the supplied packet was longer than 256 bytes
        PRINTLN(F("LoRa 433: Filed: Packet for LoRa 433 too long!"));
    }
    else if (state == RADIOLIB_ERR_TX_TIMEOUT)
    {
        // timeout occured while transmitting packet
        PRINT(F("LoRa 433: Failed: timeout!:  code:"));
        PRINTLN(state);
        error = true;
    }
    else
    {
        // some other error occurred
        PRINT(F("LoRa 433: Failed: code:"));
        PRINTLN(state);
        error = true;
    }

    delay(TENTH_SEC);

    setup_lora_433_rx();

    return error;
}

/* ============ */
/* on Receive   */
/* ============ */

void on_receive_flag()
{
    PRINTLN("Func: on_receive_flag()");

    received_433_flag = true;
    rx_counter++;
}

/* =================== */
/* Handle 433 Message  */
/* =================== */

void handle_433_message()
{
    PRINTLN("Func: handle_433_message");

    PRINTLN();
    PRINT(F("Packet_433_Ok,RSSI"));
    rssi_g = radio_433_1.getRSSI();
    PRINT(rssi_g);
    PRINT(F("dBm,SNR,"));
    snr_g = radio_433_1.getSNR();
    PRINT(snr_g);
    PRINT(F("dB,Frequency error,"));
    PRINT(radio_433_1.getFrequencyError());
    PRINT(F("Hz,Packets,"));
    PRINT(rx_counter);
    PRINT(F(",Errors,"));
    PRINT(errors_counter);
    PRINTLN();

    received_433_flag = false;

    lora_433_rx_buf_size = radio_433_1.getPacketLength();
    PRINT(F("Length:\t"));
    PRINTLN(lora_433_rx_buf_size);

    int state = radio_433_1.readData(lora_433_rx_buf, lora_433_rx_buf_size);
    if (state == RADIOLIB_ERR_NONE)
    {
        if (lora_433_rx_buf[2] == type_sbeacon) // msg_type = short_beacon
        {
            // lora_data_t lora_data;
            memset(short_beacon_g.packet, 0x00, SHRT_BCN_PACKET_LENGTH);
            memcpy(short_beacon_g.packet, lora_433_rx_buf, lora_433_rx_buf_size);
        }
        else
        {
            // lora_data_t lora_data;
            memset(lora_data_g.packet, 0x00, LORA_PACKET_LENGTH);
            memcpy(lora_data_g.packet, lora_433_rx_buf, lora_433_rx_buf_size);
        }

        // uint8_t idx = get_satid_by_call_sign(lora_433_rx_buf[0]);

        // if (idx == 0) // Satlla1
        // {
        if (lora_433_rx_buf[2] == type_sbeacon) // msg_type = short_beacon
        {
            // lora_data_t lora_data;
            print_message(short_beacon_g.packet, SHRT_BCN_PACKET_LENGTH, lora_433);
        }
        else
        {
            // lora_data_t lora_data;
            print_message(lora_data_g.packet, lora_data_g.msg_size + LORA_HEADER_LENGTH, lora_433);

            // Origin. Skip.
            if (lora_data_g.local_address == local_address)
            {
                PRINTLN("Func: handle_433_message(): Origin!");
                setup_lora_433_rx();
                return;
            }

            // Not for me!. Skip.
            if (lora_data_g.destination >> 4 != 0x0F)
            {
                PRINTLN("Func: handle_433_message(): Not for me!");
                PRINTLN(lora_data_g.destination >> 4);
                setup_lora_433_rx();
                return;
            }

            // if message is for me and replied ack.
            if (lora_data_g.destination  >> 4 == 0x0F &&
                lora_data_g.msg_type == type_ack)
            {
                PRINTLN("Func: handle_433_message(): ack replied!");
                if (ranging_execute_start)
                {
                    ranging_request_ack = true;
                }
                setup_lora_433_rx();
                return;
            }

            // if message is for me and ack requested, reply.
            if (lora_data_g.destination  >> 4 == 0x0F &&
                lora_data_g.msg_ack_req > NO_ACK)
            {
                PRINTLN("Func: handle_433_message(): ACK");
                reply_ack_433(&lora_data_g);
            }

            switch (lora_data_g.msg_type)
            {
                PRINTLN("Func: handle_433_message(): switch()");
            case type_command:
                memcpy(command_data_g.packet, lora_data_g.msg_payload, lora_data_g.msg_size);
                parse_and_activate_command(&command_data_g, lora_data_g.local_address);
                break;

            default:
                break;
            }
        }
        // }
        // else // if (idx == 1) // Norbi
        // {
        //     print_other_message(lora_433_rx_buf, lora_433_rx_buf_size, lora_433, idx);
        // }
    }
    else if (state == RADIOLIB_ERR_CRC_MISMATCH)
    {
        // packet was received, but is malformed
        PRINTLN(F("LoRa: CRC error!"));
    }
    else
    {
        // some other error occurred
        PRINT(F("LoRA: Failed, code "));
        PRINTLN(state);
    }

    setup_lora_433_rx();

    PRINTLN("End: handle_433_message");
    PRINTLN("***********************");
}

/* ============ */
/* Reply Ack    */
/* ============ */

void reply_ack_433(lora_data_t *rcve_message)
{
    PRINTLN("Func: reply_ack_433()");
    // lora_data_t rcve_message;
    // memcpy(rcve_message.packet, message, messageLength);

    // lora_data_t reply_ack_g;
    // memset(reply_ack_g.packet, 0, LORA_PACKET_LENGTH);
    reply_ack_g.msg_type = type_ack;
    reply_ack_g.local_address = local_address;
    reply_ack_g.destination = rcve_message->local_address;
    reply_ack_g.msg_size = RADIO_DATA_LENGTH;
    reply_ack_g.msg_index = rcve_message->msg_index;
    reply_ack_g.msg_time = millis() / SEC_1;
    reply_ack_g.msg_ack_req = 0x00;

    radio_data_g.rssi = (uint8_t)(abs(rssi_g));
    radio_data_g.snr = (int8_t)(snr_g);

    memcpy(reply_ack_g.msg_payload, radio_data_g.packet, RADIO_DATA_LENGTH);

    send_message_lora_433(reply_ack_g.packet, LORA_HEADER_LENGTH + reply_ack_g.msg_size);
    delay(TENTH_SEC);
}

/* ============ */
/* FSK          */
/* ============ */

void fsk_433_setup()
{

    PRINTLN(F("[FSK] radio_433_1: FSK Setup"));

    int state = radio_433_1.beginFSK();
    if (state == RADIOLIB_ERR_NONE)
    {
        PRINTLN("[FSK] radio_433_1: Setup Successed.");

        setup_fsk_433_default();

        fsk_433_is_on = millis();
    }
    else
    {
        PRINTLN("[FSK] radio_433_1: Setup Failed!");
        // while (1); //TODO: Revised
        // lora_433_turn_off();

        PRINT(F("[FSK] radio_433_1 433: Failed: code:"));
        PRINTLN(state);

        fsk_433_is_on = 0L;
    }

    // FSK modulation can be changed to OOK
    // NOTE: When using OOK, the maximum bit rate is only 32.768 kbps!
    //       Also, data shaping changes from Gaussian filter to
    //       simple filter with cutoff frequency. Make sure to call
    //       setDataShapingOOK() to set the correct shaping!
    //   state = radio.setOOK(true);
    //   state = radio.setDataShapingOOK(1);
    //   if (state != RADIOLIB_ERR_NONE) {
    //     Serial.print(F("Unable to change modulation, code "));
    //     Serial.println(state);
    //     while (true);
    //   }
}

void setup_fsk_433_default()
{
    PRINTLN(F("Func: setup_fsk_433_default()"));

    int state = radio_433_1.beginFSK();
    if (state == RADIOLIB_ERR_NONE)
    {
        PRINTLN("[FSK] radio_433_1: Setup Successed.");

        modem_fsk_433_setup();

        setup_fsk_433(&modem_fsk_433_g);

        fsk_433_is_on = millis();
    }
}

void setup_fsk_433(modem_data_t *modem_data)
{
    PRINTLN(F("Func: setup_fsk_433()"));

    double freq_e = modem_data->modem_frequency;
    if (freq_e < 434E6 || freq_e > 470E6)
    {
        freq_e = LORA_433_BAND;
    }

    float bwe = 62.5;
    if (modem_data->modem_bw < 0 || modem_data->modem_bw > BW_433_SIZE)
    {
        bwe = bw_433_table[modem_data->modem_bw];
    }

    PRINT(F("FSK SX127X: "));
    print_modem(modem_data);

    int state = 0;

    // set carrier frequency to 437.250 MHz
    if ((state = radio_433_1.setFrequency(freq_e / 1E6)) == RADIOLIB_ERR_INVALID_FREQUENCY)
    {
        PRINTLN(F("Selected frequency is invalid for this module!"));
    }

    if ((state = radio_433_1.setFrequencyDeviation(modem_data->modem_offset)) == RADIOLIB_ERR_INVALID_FREQUENCY_DEVIATION)
    {
        PRINTLN(F("Selected frequency deviation is invalid for this module!"));
    }

    // set bandwidth to 62.5 kHz
    if ((state = radio_433_1.setRxBandwidth(modem_data->modem_bw)) == RADIOLIB_ERR_INVALID_RX_BANDWIDTH)
    {
        PRINTLN(F("Error: radio_433_1.setRxBandwidth()"));
    }

    // set Bit Rate
    if ((state = radio_433_1.setBitRate(modem_data->modem_br)) == RADIOLIB_ERR_INVALID_BIT_RATE)
    {
        PRINTLN(F("Error: radio_433_1.setBitRate()"));
    }

    // set over current protection limit to 80 mA (accepted range is 45 - 240 mA)
    // NOTE: set value to 0 to disable overcurrent protection
    if ((state = radio_433_1.setCurrentLimit(modem_data->modem_cr)) == RADIOLIB_ERR_INVALID_CURRENT_LIMIT)
    {
        PRINTLN(F("Error: radio_433_1.setCodingRate()"));
    }

    // set sync word to 0x2A
    if ((state = radio_433_1.setSyncWord((uint8_t *)(modem_data->modem_sw), 2)) == RADIOLIB_ERR_INVALID_SYNC_WORD)
    {
        PRINTLN(F("Error: radio_433_1.setSyncWord()"));
    }

    // set preamble length
    if ((state = radio_433_1.setPreambleLength(modem_data->modem_pl)) == RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH)
    {
        PRINTLN(F("Error: radio_433_1.setSyncWord()"));
    }

    // set output power to 10 dBm (accepted range is -3 - 17 dBm)
    // NOTE: 20 dBm value allows high power operation, but transmission
    //       duty cycle MUST NOT exceed 1%
    if ((state = radio_433_1.setOutputPower(modem_data->modem_power)) == RADIOLIB_ERR_INVALID_OUTPUT_POWER)
    {
        PRINTLN(F("Error: radio_433_1.setOutputPower()"));
    }

    if (state != RADIOLIB_ERR_NONE)
    {
        PRINT(F("[FSK] radio_433_1 433: Unable to set configuration, code :"));
        PRINTLN(state);

        fsk_433_is_on = 0L;
    }
    else
    {
        radio_433_1.startReceive();
    }
}

#endif