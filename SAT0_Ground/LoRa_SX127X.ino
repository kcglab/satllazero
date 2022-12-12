/**
    @file LoRa_SX127X.ino
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

#if LORA_SX127X_ENABLE

// include the library
// #include <SX127XLT.h>
#define LORA_DEVICE DEVICE_SX1276_PABOOST // we need to define the device we are using

SX127XLT SX1278LT;

uint8_t lora_433_tx_buf[LORA_RX_TX_BYTE];
uint8_t lora_433_rx_buf[LORA_RX_TX_BYTE];
uint8_t lora_433_rx_buf_size = 0;

#define BW_433_SIZE 10
// 0     1      2      3      4      5      6      7       8       9
// 7810, 10420, 15630, 20830, 31250, 41670, 62500, 125000, 250000, 500000
const long bw_433_table[BW_433_SIZE] = {0, 16, 32, 48, 64, 80, 96, 112, 128, 144};

uint32_t tx_433_counter;
uint32_t rx_433_counter;
uint32_t errors_433_counter;
uint8_t rx_packet_length_433;       // stores length of packet received
int16_t packet_rssi_433; // stores RSSI of received packet
int8_t packet_snr_433;   // stores signal to noise ratio of received packet

uint16_t rangeing_error_count, rangeing_valid_count, rangeing_valid_results;
uint16_t IrqStatus;
uint32_t startrangingmS, endwaitmS, range_result, range_result_sum, range_result_average, response_sent;
float distance, distance_sum, distance_average;
bool ranging_error;

/* ============ */
/* Setup        */
/* ============ */

void lora_433_setup(void)
{
    PRINTLN(F("LoRa SX127X: Receiver Setup"));

    //  bool begin(int8_t pinNSS, int8_t pinNRESET, int8_t pinDIO0, uint8_t device);
    if (!SX1278LT.begin(LORA_433_NSS_PIN, LORA_433_RST_PIN, LORA_433_DIO0_PIN, -1, -1, LORA_DEVICE))
    {
        PRINTLN(F("LoRa SX127X: Setup Failed!"));
        // lora_433_is_on = 0L;
    }
    else
    {
        PRINTLN(F("LoRa SX127X: Setup Successed."));
        setup_lora_433_default();
        SX1278LT.rxtxInit(LORA_433_RX_EN_PIN, LORA_433_TX_EN_PIN); //  RXEN,  TXEN
        // put the radio into receive mode
        setup_lora_433_rx();
        // lora_433_is_on = millis();
    }
}

void set_modem_433(unsigned long freq, float bw, int sf, int cr, int pm, int sw, int pl)
{
    PRINTLN("Func: set_modem_433()");
    setup_lora_433(freq, lora_433_offset, bw, sf, cr, pm, lora_433_ldro, pl, sw, lora_433_crc);
}

void setup_lora_433()
{
    setup_lora_433_default();
}

void setup_lora_433_default()
{
    PRINTLN(F("Func: setup_lora_433_default()"));
    setup_lora_433(LORA_433_BAND, LORA_433_OFFSET, LORA_433_BW, LORA_433_SF, LORA_433_CR, LORA_433_TX_RX_POWER, LORA_433_LDRO, LORA_433_PL, LORA_433_SW, LORA_433_CRC);
}

void set_lora_433(uint8_t bw, uint8_t sf, uint8_t cr, uint8_t pm)
{
    PRINTLN("Func: set_lora_433()");
    setup_lora_433(lora_433_frequency, lora_433_offset, bw, sf, cr, pm, lora_433_ldro, lora_433_pl, lora_433_sw, lora_433_crc);
}

void setup_lora_433(uint32_t frequency, int32_t offset, uint8_t bw, uint8_t sf, uint8_t cr, uint8_t pm, uint8_t ldro, uint8_t pl, uint16_t sw, uint8_t crc)
{
    PRINTLN(F("Func: setup_lora_433()"));

    if (frequency < 434E6 || frequency > 470E6)
    {
        frequency = LORA_433_BAND;
    }

    if (offset < -500E3 || offset > 500E3)
    {
        offset = LORA_433_OFFSET;
    }

    if (bw < 0 || bw > BW_433_SIZE)
    {
        bw = LORA_433_BW; // 62.5
    }

    if (cr < 5 || cr > 8)
    {
        cr = LORA_433_CR;
    }

    if (sf < 6 || sf > 12)
    {
        sf = LORA_433_SF;
    }

    if (pm < 5 || pm > 20)
    {
        pm = LORA_433_TX_RX_POWER;
    }

    if (ldro < 0 || ldro > 0x02)
    {
        ldro = LORA_433_LDRO;
    }

    if (pl < 8 || pm > 20)
    {
        pm = LORA_433_PL;
    }

    // convert idx to val
    uint8_t bw_val = bw_433_table[bw];

    PRINT(F("LoRa SX127X: "));
    PRINT(F("Freq: "));
    PRINT(frequency);
    PRINT(F(" Hz,Offset: "));
    PRINT(offset);
    PRINT(F(",bw["));
    PRINT(bw);
    PRINT(F("]: "));
    PRINT(bw_433_table[bw]);
    PRINT(F(",sf: "));
    PRINT(sf);
    PRINT(F(",cr: "));
    PRINT(cr);
    PRINT(F(",pm: "));
    PRINT(pm);
    PRINT(F(",ldro: "));
    PRINT(ldro);
    PRINT(F(",sw: "));
    PRINT(sw);
    PRINT(F(",pl: "));
    PRINT(pl);
    PRINT(F(",crc: "));
    PRINTLN(crc);

    // setupLoRa(uint32_t Frequency, int32_t Offset, uint8_t modParam1, uint8_t modParam2, uint8_t  modParam3, uint8_t modParam4)
    // order is SpreadingFactor, Bandwidth, CodeRate, Optimisation
    SX1278LT.setMode(MODE_STDBY_RC);
    SX1278LT.setPacketType(PACKET_TYPE_LORA); // set for LoRa transmissions
    SX1278LT.setRfFrequency(frequency, offset);
    SX1278LT.calibrateImage(0);
    SX1278LT.setModulationParams(sf, bw_val, cr, ldro);
    SX1278LT.setSyncWord(sw);
    SX1278LT.setBufferBaseAddress(0x00, 0x00);
    SX1278LT.setHighSensitivity(); // set for highest sensitivity at expense of slightly higher LNA current
    SX1278LT.setPacketParams(pl, LORA_PACKET_VARIABLE_LENGTH, 255, crc, LORA_IQ_NORMAL);
    SX1278LT.setDioIrqParams(IRQ_RADIO_ALL, IRQ_RX_DONE, 0, 0); // set for IRQ on RX done

#ifdef PRINT_FUNC_DEBUG
    SX1278LT.printModemSettings(); // reads and prints the configured LoRa settings, useful check
    PRINTLN();
    SX1278LT.printOperatingSettings();
    PRINTLN();
    SX1278LT.printRegisters(0x00, 0x4F); // print contents of device registers, normally 0x00 to 0x4F
    PRINTLN();
#endif
}

void setup_lora_433_rx()
{
    PRINTLN(F("Func: setup_lora_433_rx()"));
    SX1278LT.rxEnable();

    lora_433_receive();
}

void setup_lora_433_tx()
{
    PRINTLN(F("Func: setup_lora_433_tx()"));

    detachInterrupt(digitalPinToInterrupt(LORA_433_DIO0_PIN));

    SX1278LT.txEnable();
}

/* ============ */
/* on Receive   */
/* ============ */

void lora_433_receive()
{
    PRINTLN(F("Func: lora_433_receive()"));
    SX1278LT.setRx(0); // set no SX127X RX timeout

    attachInterrupt(digitalPinToInterrupt(LORA_433_DIO0_PIN), on_receive_433_interrupt, RISING);
}

void on_receive_433_interrupt()
{
    PRINTLN(F("Func: on_receive_433_interrupt()"));
    detachInterrupt(digitalPinToInterrupt(LORA_433_DIO0_PIN));

    received_433_flag = true;
}

void on_receive_433()
{
    PRINTLN(F("Func: on_receive_433()"));

    unsigned long start = millis();
    while (!digitalRead(LORA_433_DIO0_PIN))
    {
        if (millis() - start > SECS_5)
            break;
    }; //wait for RxDone or timeout interrupt activating DIO0

    // rx_packet_length_433 = SX1278LT.receive(lora_433_rx_buf, LORA_RX_TX_BYTE, SECS_5, WAIT_RX); // if the LT.receive() function detects an error, then it returns 0 
    rx_packet_length_433 = SX1278LT.readPacket(lora_433_rx_buf, LORA_RX_TX_BYTE);

    lora_433_rx_buf_size = SX1278LT.readrx_packet_length_433();
    packet_rssi_433 = SX1278LT.readPacketRSSI();
    packet_snr_433 = SX1278LT.readPacketSNR();

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

    if (rx_packet_length_433)
    {
        rx_433_counter++;
        packet_433_is_ok();
    }
    else
    {
        errors_433_counter++;
        packet_433_is_error();
    }
    PRINTLN();

    // put the radio into receive mode
    setup_lora_433_rx();
}

/* ============ */
/* LoRa 433_1     */
/* ============ */

bool send_message_lora_433(uint8_t *message, uint8_t messageLength)
{
    send_message_lora_433(message, messageLength, false);
}

bool send_message_lora_433(uint8_t *message, uint8_t messageLength, bool async)
{
    PRINTLN(F("Func: send_message_lora_433()"));

    // if (!lora_433_is_on)
    // {
    //     PRINTLN(F("LoRa SX127X: Sending Failed - Radio is off!"));
    //     return;
    // }

    // lora_433_enable_interrupt = false;
    //  set params for TX
    setup_lora_433_tx();

    memset(lora_433_tx_buf, 0x00, LORA_RX_TX_BYTE);
    memcpy(lora_433_tx_buf, message, messageLength);
    print_message(lora_433_tx_buf, messageLength, lora_433);

    unsigned long start = millis();
    PRINT(F("Sending Time():\t"));
    uint8_t wait_tx = (async) ? WAIT_TX : NO_WAIT;
    int state = SX1278LT.transmit(lora_433_tx_buf, messageLength, SECS_5, lora_433_tx_rx_power, wait_tx);
    PRINT(millis() - start);
    PRINTLN(F(" ms"));

    if (state > 0)
    {
        // the packet was successfully transmitted
        tx_433_counter++;
        PRINTLN(F("LoRa SX127X: Sending Succeed!"));
    }
    else
    {
        PRINTLN(F("LoRa SX127X: Sending Failed!"));
        packet_433_is_error();
    }
    PRINTLN();

    delay(TENTH2_SEC);

    // lora_433_enable_interrupt = true;

    // put the radio into receive mode
    setup_lora_433_rx();

    return state;
}

/* ============ */
/* 433 Packet   */
/* ============ */

void packet_433_is_ok()
{
    PRINTLN(F("Func: packet_433_is_ok()"));

    PRINTLN();
    PRINT(F("Packet_433_Ok,RSSI,"));
    rssi_g = packet_rssi_433;
    PRINT(packet_rssi_433);
    PRINT(F("dBm,SNR,"));
    snr_g = packet_snr_433;
    PRINT(packet_snr_433);
    PRINT(F("dB,Length,"));
    PRINT(lora_433_rx_buf_size);
    PRINT(F(",Packets,"));
    PRINT(rx_433_counter);
    PRINT(F(",Errors,"));
    PRINT(errors_433_counter);
    uint16_t IRQStatus = SX1278LT.readIrqStatus(); // get the IRQ status
    PRINT(F(",IRQreg,"));
    PRINT(IRQStatus, HEX);
    PRINTLN();
}

void packet_433_is_error()
{
    PRINTLN(F("Func: packet_433_is_error()"));
    
    PRINTLN();
    PRINT(F("PacketError,RSSI,"));
    PRINT(packet_rssi_433);
    PRINT(F("dBm,SNR,"));
    PRINT(packet_snr_433);
    PRINT(F("dB,Length,"));
    PRINT(rx_packet_length_433);
    PRINT(F(",Errors,"));
    PRINT(errors_433_counter);
    uint16_t IRQStatus = SX1278LT.readIrqStatus(); // get the IRQ status
    PRINT(F(",IRQreg,"));
    PRINT(IRQStatus, HEX);
    PRINTLN();
}

/* =================== */
/* Handle 433 Message  */
/* =================== */

void handle_433_message()
{
    PRINTLN("Func: handle_433_message()");

    PRINTLN();
    uint16_t localCRC = SX1278LT.CRCCCITT(lora_433_rx_buf, lora_433_rx_buf_size, 0xFFFF); // calculate the CRC, this is the external CRC calculation of the RXBUFFER

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
}

/* ============ */
/* Reply Ack    */
/* ============ */

void reply_ack_433(lora_data_t *rcve_message)
{
    PRINTLN(F("Func: reply_ack_433"));
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

    radio_data_g.rssi = (uint8_t)(abs(packet_rssi_433));
    radio_data_g.snr = (int8_t)(packet_snr_433);

    memcpy(reply_ack_g.msg_payload, radio_data_g.packet, RADIO_DATA_LENGTH);
    send_message_lora_433(reply_ack_g.packet, LORA_HEADER_LENGTH + reply_ack_g.msg_size);
    delay(TENTH_SEC);
}

/* ============ */
/* Wrap Message */
/* ============ */

void wrap_message_433(uint8_t type)
{
    PRINTLN(F("Func: wrap_message_433()"));

    return wrap_message_433(0x00, 0, type);
}

void wrap_message_433(uint8_t *message, uint8_t size, uint8_t type)
{
    PRINTLN(F("Func: wrap_message_433()"));

    wrap_msg_g.local_address = local_address;
    wrap_msg_g.destination = destination;
    wrap_msg_g.msg_type = type;
    wrap_msg_g.msg_size = 0;
    wrap_msg_g.msg_index = tx_counter;
    wrap_msg_g.msg_time = millis() / SEC_1;
    wrap_msg_g.msg_ack_req = 0x00;
    if (size > 0)
    {
        memcpy(wrap_msg_g.msg_payload, message, size);
        wrap_msg_g.msg_size = size;
    }

    send_message_lora_433(wrap_msg_g.packet, LORA_HEADER_LENGTH + wrap_msg_g.msg_size);
}

#endif