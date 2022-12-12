/**
    @file LoRa_SX1278.ino
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

#if LORA_SX1278_ENABLE

uint8_t lora_433_tx_buf[LORA_RX_TX_BYTE];
uint8_t lora_433_rx_buf[LORA_RX_TX_BYTE];
volatile uint8_t lora_433_rx_buf_size = 0;

#define BW_433_SIZE 9
const long bw_433_table[BW_433_SIZE] = {7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3};

/* ============ */
/* Setup        */
/* ============ */

void lora_433_setup()
{
    PRINTLN("Func: lora_433_setup()");

#if (DEVCIE == 1) // Teensy
    LoRa.setPins(LORA_433_NSS_PIN, LORA_433_RST_PIN, LORA_433_DIO0_PIN);
#elif (DEVCIE == 2) // TTGO
    LoRa.setPins(NSS, RST, DIO);
#endif

    if (!LoRa.begin(LORA_433_BAND, LORA_433_PABOOST))
    {
        PRINTLN("LoRa 433: Setup Failed!");
    }
    else
    {
        PRINTLN("LoRa 433: Setup Successed.");
        PRINT("LoRa 433 Freq: ");
        PRINTLN(LORA_433_BAND, 0);
    }
    setup_lora_433();

    LoRa.disableCrc();
    // register the receive callback
    LoRa.onReceive(on_receive_433);
    // put the radio into receive mode
    LoRa.receive();
}

/* ============ */
/* Settings     */
/* ============ */
void setup_lora_433()
{
    PRINTLN(F("Func: setup_lora_433()"));
    set_lora_433(lora_433_bw, lora_433_sf, lora_433_cr, lora_433_tx_rx_power);
}

void set_lora_433(int bw, int sf, int cr, int pm)
{
    PRINTLN("Func: set_lora_433()");
    if (bw < 0 || bw > BW_433_SIZE)
    {
        bw = LORA_433_BW;
    }

    if (sf < 6 || sf > 12)
    {
        sf = LORA_433_SF;
    }

    if (cr < 5 || cr > 8)
    {
        cr = LORA_433_CR;
    }

    if (pm < 5 || pm > 20)
    {
        pm = LORA_433_TX_RX_POWER;
    }

    PRINT("LoRa 433: ");
    PRINT("bw[");
    PRINT(bw);
    PRINT("]:");
    PRINT(bw_433_table[bw]);
    PRINT(", sf: 0x");
    PRINT(sf, HEX);
    PRINT(", cr: 0x");
    PRINT(cr, HEX);
    PRINT(", pm: 0x");
    PRINTLN(pm, HEX);

    LoRa.sleep();
    LoRa.setFrequency(LORA_433_BAND);
    LoRa.setSignalBandwidth1(bw);
    LoRa.setSpreadingFactor(sf); // from 6 to 12
    LoRa.setCodingRate4(cr);     // from 5 to 8
    LoRa.setTxPowerMax(pm);      // 5~20
    LoRa.setSyncWord(LORA_433_SW);
    LoRa.disableCrc(); // enableCrc();
    LoRa.idle();

    LoRa.receive();
}

void set_modem_433(unsigned long freq, float bw, int sf, int cr, int pm, int sw, int pl)
{
    PRINTLN("Func: set_modem_433()");
    PRINT("LoRa 433: ");
    PRINT("Freq: ");
    PRINT(freq);
    PRINT(", bw: ");
    PRINT(bw);
    PRINT(", sf:");
    PRINT(sf);
    PRINT(", cr:");
    PRINT(cr);
    PRINT(", pm:");
    PRINT(pm);
    PRINT(", sw:");
    PRINT(sw);
    PRINT(", pl:");
    PRINTLN(pl);

    LoRa.sleep();
    LoRa.setFrequency(freq);
    if (bw > BW_433_SIZE)
    {
        LoRa.setSignalBandwidth(bw);
    }
    else
    {
        LoRa.setSignalBandwidth1(bw);
    }
    LoRa.setSpreadingFactor(sf);
    LoRa.setCodingRate4(cr);
    LoRa.setTxPowerMax(pm); // 5~20
    LoRa.setSyncWord(sw);
    LoRa.setPreambleLength(pl);
    LoRa.disableCrc(); // enableCrc();
    LoRa.idle();

    LoRa.receive();
}

void set_lora_433_bw(int bw)
{
    PRINTLN("Func: set_lora_433_bw()");
    PRINT("LoRa 433 bw:\t");
    PRINTLN(bw_433_table[bw]);

    LoRa.sleep();
    LoRa.setSignalBandwidth1(bw);
    LoRa.idle();

    LoRa.receive();
}

/* ============ */
/* LoRa 433     */
/* ============ */

void send_message_lora_433(uint8_t *message, uint8_t messageLength, bool async)
{
    PRINTLN("Func: send_message_lora_433()");

    // lora_433_enable_interrupt = false;
    rssi_g = 0;
    snr_g = 0;

    memset(lora_433_tx_buf, 0x00, LORA_RX_TX_BYTE);
    memcpy(lora_433_tx_buf, message, messageLength);

    print_message(lora_433_tx_buf, messageLength, lora_433);

    if (messageLength > 0 && messageLength < 70)
    {
        messageLength += 10;
    }

    unsigned long start = millis();
    // send packet
    LoRa.beginPacket();
    LoRa.write(lora_433_tx_buf, messageLength);
    int state = LoRa.endPacket(async);
    PRINT("Sending Time():\t");
    PRINT(millis() - start);
    PRINTLN(" ms");
    PRINTLN();

    if (state == 1)
    {
        // the packet was successfully transmitted
        tx_counter++;
        PRINTLN(F("LoRa 433: Sending Succeed!"));

        // print measured data rate
        //    PRINT(F("LoRa 433 Datarate: "));
        //    PRINT(LoRa.getDataRate());
        //    PRINTLN(F(" bps"));
    }
    else if (state == -1)
    {
        // the supplied packet was longer than 256 bytes
        PRINTLN(F("LoRa 433: Filed: Packet for LoRa 433 too long!"));
    }
    else if (state == -2)
    {
        // timeout occured while transmitting packet
        PRINTLN(F("LoRa 433: Failed: timeout!"));
    }

    delay(TENTH_SEC);
    // lora_433_enable_interrupt = true;

    // put the radio into receive mode
    LoRa.receive();
}

/* ============ */
/* on Receive   */
/* ============ */

void on_receive_433(int packetSize)
{
    PRINTLN("Func: on_receive_433()");
    PRINT("packetSize:\t");
    PRINTLN(packetSize);
    if (packetSize <= 0)
    {
        return;
    }

    // packet was successfully received
    PRINTLN(F("LoRa 433: Received packet!"));
    // read packet
    for (uint8_t i = 0; i < packetSize; i++)
    {
        // PRINTLN((char)LoRa.read());
        lora_433_rx_buf[i] = LoRa.read();
        // PRINT(lora_433_rx_buf[i], HEX);
        // PRINT(",");
    }
    lora_433_rx_buf_size = packetSize;

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

    // Put message into RX Buffer
    // if (!RXbuffer.isFull())
    // {
    //     ringbuffer_data_g.lora_data = lora_data_g;
    //     ringbuffer_data_g.rssi = LoRa.packetRssi();
    //     ringbuffer_data_g.snr = LoRa.packetSnr();
    //     ringbuffer_data_g.freq = lora_433;
    //     RXbuffer.add(ringbuffer_data_g);
    //     PRINT("RXbuffer Element:\t");
    //     PRINTLN(RXbuffer.numElements());
    // }

    received_433_flag = true;

    // if (lora_data_g.msg_type == type_becon)
    // {
    //     recieved_beacon = true;
    // }
}

void handle_433_message()
{
    PRINTLN("Func: handle_433_message()");

    PRINTLN();
    PRINT(F("Packet_433_Ok,RSSI"));
    rssi_g = LoRa.packetRssi();
    PRINT(rssi_g);
    PRINT(F("dBm,SNR,"));
    snr_g = LoRa.packetSnr();
    PRINT(snr_g);
    PRINT(F("dB,Length,"));
    PRINT(lora_433_rx_buf_size);
    PRINT(F(",Packets,"));
    PRINT(rx_counter);
    PRINT(F(",Errors,"));
    PRINT(errors_counter);
    PRINTLN();

    uint8_t idx = get_satid_by_call_sign(lora_433_rx_buf[0]);

    if (idx == 0) // Satlla1
    {
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
                return;
            }

            // Not for me!. Skip.
            if (lora_data_g.destination != local_address)
            {
                PRINTLN("Func: handle_433_message(): Not for me!");
                return;
            }

            // if message is for me and replied ack.
            if (lora_data_g.destination == local_address &&
                lora_data_g.msg_type == type_ack)
            {
                PRINTLN("Func: handle_433_message(): ack replied!");
                if (ranging_execute_start)
                {
                    ranging_request_ack = true;
                }
                return;
            }

            // if message is for me and ack requested, reply.
            if (lora_data_g.destination == local_address &&
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
    }
    else // if (idx == 1) // Norbi
    {
        print_other_message(lora_433_rx_buf, lora_433_rx_buf_size, lora_433, idx);
    }

    //     while (!RXbuffer.isEmpty())
    //     {
    //         //RXbuffer.pull(&lora_data_b);
    // #if (DEVCIE == 0)
    //         int rssi = LoRa.packetRssi();
    //         float snr = LoRa.packetSnr();
    //         //post the message on google docs
    //         postToGoogleDoc(lora_data_g.packet, lora_data_g.msg_size + LORA_HEADER_LENGTH, rssi, snr, lora_433);
    // #endif
    //     }
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

    send_message_lora_433(reply_ack_g.packet, LORA_HEADER_LENGTH + reply_ack_g.msg_size, false);
    delay(TENTH_SEC);
}

#endif