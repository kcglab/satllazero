/**
    @file LoRaSX1280.ino
    @brief LoRa 2.4 utils functions for SATLLA0 GS.

    This file contains LoRa 2.4 Radio functionality for the SATLLA0 GS.

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

#if LORA_SX128X_ENABLE

// include the library
#include <SX128XLT.h>

#define LORA_DEVICE DEVICE_SX1280 // we need to define the device we are using

SX128XLT LT;

uint8_t lora_24_tx_buf[LORA_RX_TX_BYTE];
uint8_t lora_24_rx_buf[LORA_RX_TX_BYTE];
uint8_t lora_24_rx_buf_size = 0;

uint32_t tx_24_counter;
uint32_t rx_24_counter;
uint32_t errors_24_counter;

uint8_t rx_packet_length_24; // stores length of packet received
int16_t packet_RSSI_24;      // stores RSSI of received packet
int8_t packet_SNR_24;        // stores signal to noise ratio of received packet

uint16_t rangeing_error_count, rangeing_valid_count, rangeing_valid_results;
uint16_t IrqStatus;
uint32_t startrangingmS, endwaitmS, range_result, range_result_sum, range_result_average, response_sent;
float distance, distance_sum, distance_average;
bool ranging_error;

/* ============ */
/* Setup        */
/* ============ */

void lora_24_setup(void)
{
    PRINTLN(F("LoRa SX128XLT: Receiver Setup"));

    pinMode(LORA_24_TCXO_PIN, OUTPUT);
    digitalWrite(LORA_24_TCXO_PIN, HIGH);
    // delay(10);
    if (!LT.begin(LORA_24_NSS_PIN, LORA_24_RST_PIN, LORA_24_BSY_PIN, LORA_24_DIO1_PIN, -1, -1, LORA_24_RX_EN_PIN, LORA_24_TX_EN_PIN, LORA_DEVICE))
    {
        PRINTLN(F("LoRa SX128XLT: Setup Failed!"));
        lora_24_is_on = 0L;
    }
    else
    {
        PRINTLN("LoRa SX128XLT: Setup Successes.");
        setup_lora_24_default();
        // put the radio into receive mode
        setup_lora_24_rx();
        lora_24_is_on = millis();
    }
}

void setup_lora_24_default()
{
    PRINTLN(F("Func: setup_lora_24_default()"));

    modem_lora_24_setup();

    setup_lora_24(&modem_lora_24_g);
}

void setup_lora_24(modem_data_t *modem_data)
{
    PRINTLN("Func: setup_lora_24()");

    PRINTLN(F("LoRa LT: "));
    print_modem(modem_data);

    // put radion into standby.
    LT.setMode(MODE_STDBY_RC);
    // start settings
    LT.setRfFrequency(modem_data->modem_frequency, modem_data->modem_offset);
    // sequence is spreading factor, bandwidth, coding rate.
    LT.setModulationParams(modem_data->modem_sf, modem_data->modem_bw, modem_data->modem_cr);
    // sequence is PreambleLength, HeaderType, PayloadLength, CRC, InvertIQ/chirp invert
    LT.setPacketParams(modem_data->modem_pl, LORA_PACKET_VARIABLE_LENGTH, 255, modem_data->modem_crc, LORA_IQ_NORMAL, 0, 0);
    uint8_t use_ldo = (modem_data->modem_ldro == 1); // USE_LDO = 0x00
    LT.setRegulatorMode(use_ldo);
    LT.setPacketType(PACKET_TYPE_LORA);
    LT.setBufferBaseAddress(0, 0);
    LT.setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);
    LT.setSyncWord1(modem_data->modem_sw);
    LT.setTxParams(modem_data->modem_power, RADIO_RAMP_02_US);
    LT.setHighSensitivity();

#ifdef PRINT_FUNC_DEBUG
    LT.printModemSettings(); // reads and prints the configured LoRa settings, useful check
    PRINTLN();
    LT.printOperatingSettings();
    PRINTLN();
    LT.printRegisters(0x00, 0x4F); // print contents of device registers, normally 0x00 to 0x4F
    PRINTLN();
#endif
}

void setup_lora_24_rx()
{
    PRINTLN(F("Func: setup_lora_24_rx()"));

    LT.setDioIrqParams(IRQ_RADIO_ALL, (IRQ_RX_DONE + IRQ_HEADER_ERROR + IRQ_CRC_ERROR + IRQ_RX_TX_TIMEOUT), 0, 0);
    LT.clearIrqStatus(IRQ_RADIO_ALL); // clear all interrupt flags
    LT.setRx(0);                      // set no SX1280 RX timeout
    LT.rxEnable();
    lora_24_receive();
}

void setup_lora_24_tx()
{
    PRINTLN(F("Func: setup_lora_24_tx()"));

    detachInterrupt(digitalPinToInterrupt(LORA_24_DIO1_PIN));

    LT.setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0); // set for IRQ on TX done and timeout on DIO1
    LT.txEnable();
}

/* ============ */
/* on Receive   */
/* ============ */

void lora_24_receive()
{
    PRINTLN("Func: lora_24_receive()");
    // LT.clearIrqStatus(0xFFFF);    //Clear all the IRQ flags
    LT.setRx(0); // set no SX1280 RX timeout
    attachInterrupt(digitalPinToInterrupt(LORA_24_DIO1_PIN), on_receive_24_interrupt, RISING);
}

void on_receive_24_interrupt()
{
    PRINTLN("Func: on_receive_24_interrupt()");
    received_24_flag = true;
}

void on_receive_24()
{
    PRINTLN("Func: on_receive_24()");

    detachInterrupt(digitalPinToInterrupt(LORA_24_DIO1_PIN));

    unsigned long start = millis();
    while (!digitalRead(LORA_24_DIO1_PIN))
    {
        if (millis() - start > SECS_4)
        {
            break;
        }
    }; // wait for RxDone or timeout interrupt activating DIO1

    delay(10);
    rx_packet_length_24 = LT.readRXPacketL();
    packet_RSSI_24 = LT.readPacketRSSI();
    packet_SNR_24 = LT.readPacketSNR();

    if (LT.readIrqStatus() == (IRQ_RX_DONE + IRQ_HEADER_VALID + IRQ_PREAMBLE_DETECTED))
    {
        packet_is_OK();
        handle_24_message();
    }
    else
    {
        packet_24_is_error();
    }
    PRINTLN();
    setup_lora_24_rx();
}

/* ============ */
/* LoRa 2.4     */
/* ============ */

bool send_message_lora_24(uint8_t *message, uint8_t messageLength)
{
    PRINTLN("Func: send_message_lora_24()");
    // lora_24_enable_interrupt = false;
    //  set params for TX
    setup_lora_24_tx();

    memset(lora_24_tx_buf, 0x00, LORA_RX_TX_BYTE);
    memcpy(lora_24_tx_buf, message, messageLength);
    print_message(lora_24_tx_buf, messageLength, lora_24);

    unsigned long start = millis();
    PRINT("Sending Time():\t");
    int state = LT.transmit(lora_24_tx_buf, messageLength, SECS_4, modem_lora_24_g.modem_power, true);
    PRINT(millis() - start);
    PRINTLN(" ms");
    if (state > 0)
    {
        // the packet was successfully transmitted
        tx_24_counter++;
        PRINTLN(F("LoRa SX128XLT: Sending Succeed!"));
    }
    else
    {
        PRINTLN(F("LoRa SX128XLT: Sending Failed!"));
        packet_24_is_error();
    }
    PRINTLN();

    delay(TENTH2_SEC);

    // lora_24_enable_interrupt = true;

    // put the radio into receive mode
    setup_lora_24_rx();

    return state;
}

/* ============ */
/* 2.4 Packet   */
/* ============ */

void packet_is_OK()
{
    PRINTLN(F("Func: packet_is_OK()"));
    uint16_t IRQStatus;

    rx_24_counter++;
    // lora_24_rx_buf_size = LT.readPacketLoRa(lora_24_rx_buf, LORA_RX_TX_BYTE); //read the actual packet, maximum size specified in LORA_RX_TX_BYTE
    // uint8_t receive(uint8_t *rxbuffer, uint8_t size, uint16_t timeout, uint8_t wait);
    lora_24_rx_buf_size = LT.receive(lora_24_rx_buf, LORA_RX_TX_BYTE, SECS_4, WAIT_RX);

    // packet was successfully received
    PRINTLN("LoRa SX128XLT: Receiving...");
    memcpy(lora_data_g.packet, lora_24_rx_buf, lora_24_rx_buf_size);

    PRINTLN();
    PRINT(F("Packet_24_Ok,RSSI"));
    rssi_g = packet_RSSI_24;
    PRINT(packet_RSSI_24);
    PRINT(F("dBm,SNR,"));
    snr_g = packet_SNR_24;
    PRINT(packet_SNR_24);
    PRINT(F("dB,Length,"));
    PRINT(rx_packet_length_24);
    PRINT(F(",Packets,"));
    PRINT(rx_24_counter);
    PRINT(F(",Errors,"));
    PRINT(errors_24_counter);
    IRQStatus = LT.readIrqStatus();
    PRINT(F(",IRQreg,"));
    PRINT(IRQStatus, HEX);
    PRINTLN();

    uint8_t idx = get_satid_by_call_sign(lora_24_rx_buf[0]);

    if (idx == 0) // Satlla1
    {
        if (lora_24_rx_buf[2] == type_sbeacon) // msg_type = short_beacon
        {
            // lora_data_t lora_data;
            memset(short_beacon_g.packet, 0x00, SHRT_BCN_PACKET_LENGTH);
            memcpy(short_beacon_g.packet, lora_24_rx_buf, lora_24_rx_buf_size);

            print_message(short_beacon_g.packet, SHRT_BCN_PACKET_LENGTH, lora_24);
        }
        else
        {
            // lora_data_t lora_data;
            memset(lora_data_g.packet, 0x00, LORA_PACKET_LENGTH);
            memcpy(lora_data_g.packet, lora_24_rx_buf, lora_24_rx_buf_size);

            print_message(lora_data_g.packet, lora_data_g.msg_size + LORA_HEADER_LENGTH, lora_24);
        }
    }
    else // if (idx == 1) // Norbi
    {
        print_other_message(lora_24_rx_buf, lora_24_rx_buf_size, lora_24, idx);
    }
}

void packet_24_is_error()
{
    PRINTLN("Func: packet_24_is_error()");
    uint16_t IRQStatus;

    IRQStatus = LT.readIrqStatus(); // get the IRQ status
    errors_24_counter++;
    PRINT(F("PacketError,RSSI"));
    PRINT(packet_RSSI_24);
    PRINT(F("dBm,SNR,"));
    PRINT(packet_SNR_24);
    PRINT(F("dB,Length,"));
    PRINT(rx_packet_length_24);
    PRINT(F(",IRQreg,"));
    PRINT(IRQStatus, HEX);
    LT.printIrqStatus();
    PRINTLN();
}

/* =================== */
/* Handle 24 Message  */
/* =================== */

void handle_24_message()
{
    PRINTLN("Func: handle_24_message");

#ifdef PRINT_FUNC_DEBUG
    print_message(lora_data_g.packet, LORA_HEADER_LENGTH + lora_data_g.msg_size);
#endif

    // delay(TENTH_SEC);

    // switch (lora_data_g.msg_type)
    // {
    // case type_command:
    //     memcpy(command_data_g.packet, lora_data_g.msg_payload, lora_data_g.msg_size);
    //     parse_and_activate_command(&command_data_g);
    //     break;

    // default:
    //     break;
    // }
}

/* ============ */
/* Reply Ack    */
/* ============ */

void reply_ack_24(lora_data_t *rcve_message)
{
    PRINTLN("Func: reply_ack_24");
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

    radio_data_g.rssi = (uint8_t)(abs(packet_RSSI_24));
    radio_data_g.snr = (int8_t)(packet_SNR_24);

    memcpy(reply_ack_g.msg_payload, radio_data_g.packet, RADIO_DATA_LENGTH);
    send_message_lora_24(reply_ack_g.packet, LORA_HEADER_LENGTH + reply_ack_g.msg_size);
    delay(TENTH_SEC);
}

/*************************** RANGING ************************/

/* ============== */
/* Setup Ranging  */
/* ============== */

void setup_ranging_24(bool master)
{
    PRINTLN("Func: setup_ranging_24()");

    detachInterrupt(digitalPinToInterrupt(LORA_24_DIO1_PIN));

    uint8_t master_slave = (master) ? RANGING_MASTER : RANGING_SLAVE;

    LT.setupRanging(LORA_24_BAND, LORA_24_OFFSET, LORA_24_SF_RNG, LORA_24_BW_RNG, LORA_24_CR_RNG, LORA_24_RNG_ADDR, master_slave);
}

/* ============ */
/* Ranging      */
/* ============ */

void ranging_24_master_run(uint8_t destination)
{
    PRINTLN("Func: ranging_24_master_run()");
    ranging_execute_start = true;

    // LoRa header
    memset(rng_request_g.packet, 0x00, LORA_PACKET_LENGTH);
    rng_request_g.msg_type = type_command;
    rng_request_g.local_address = local_address;
    rng_request_g.destination = destination;
    rng_request_g.msg_index = tx_counter;
    rng_request_g.msg_time = millis() / SEC_1;
    rng_request_g.msg_ack_req = 0x01;

    // Command Request
    memset(command_data_g.packet, 0x00, COMMAND_PACKET_LENGTH);
    command_data_g.cmd_code = CMD_RANGING_SLAVE_24;
    command_data_g.cmd_type = cmd_type_none;
    command_data_g.cmd_size = 1;
    uint8_t cmd_size = command_data_g.cmd_size;
    command_data_g.cmd_chksum = checksum_func(command_data_g.cmd_code, command_data_g.cmd_type, command_data_g.cmd_size);
    command_data_g.cmd_payload[0] = 0x00;
    print_command(&command_data_g);
    encrypt_decrypt(&command_data_g);

    memcpy(rng_request_g.msg_payload, command_data_g.packet, COMMAND_HEADER_LENGTH + cmd_size);
    rng_request_g.msg_size = COMMAND_HEADER_LENGTH + cmd_size;

#if RF_433_ENABLE
    send_message_lora_433(rng_request_g.packet, LORA_HEADER_LENGTH + rng_request_g.msg_size);
#endif
    // indicator for ack.
    ranging_request_ack = false;
    // enter into a smart delay to wait for ack.
    unsigned long start = millis();
    received_433_flag = false;
    while (millis() - start < ranging_threshold)
    {
#if RF_433_ENABLE
        if (received_433_flag)
        {
            received_433_flag = false;
            delay(TENTH_SEC);
#if LORA_SX126X_ENABLE || LORA_SX1262_ENABLE
            on_receive_433();
#endif
            handle_433_message();
#if LORA_SX126X_ENABLE || LORA_SX1262_ENABLE
            setup_lora_433_rx();
#endif
            if (ranging_request_ack)
            {
                ranging_24_master(false, 2);
                break;
            }
        }
#endif
    }
    ranging_execute_start = false;
}

void ranging_24_master(bool reset_flag, uint8_t delayS)
{
    PRINTLN("Func: ranging_24_master()");
    uint8_t index;
    distance_sum = 0;
    range_result_sum = 0;
    rangeing_valid_count = 0;
    // Calibration = LORA_24_RNG_CALIB_SF10BW400;

    setup_ranging_24(true);

    delay(delayS * SEC_1);

    PRINT("LORA_24_RNG_ADDR: ");
    PRINT(LORA_24_RNG_ADDR);
    PRINT(", TXPower: ");
    PRINT(RangingTXPower);
    PRINT("dBm,");
    PRINT(F("CalibrationValue: "));
    // LT.setRangingCalibration(LORA_24_RNG_CALIB_SF10BW400);
    PRINTLN(LT.getSetCalibrationValue());

    for (index = 1; index <= LORA_24_RNG_COUNT; index++)
    {
        // if reset, then reset device for each measure
        if (reset_flag)
        {
            PRINTLN("ResetDevice,");
            LT.resetDevice();
            setup_ranging_24(true);
        }

        startrangingmS = millis();

        LT.transmitRanging(LORA_24_RNG_ADDR, LORA_24_RNG_TX_TIMEOUT_MS, RangingTXPower, WAIT_TX);

        IrqStatus = LT.readIrqStatus();

        if (IrqStatus & IRQ_RANGING_MASTER_RESULT_VALID)
        {
            rangeing_valid_count++;

            PRINT("Valid");
            range_result = LT.getRangingResultRegValue(RANGING_RESULT_RAW);
            PRINT(F(",Register,"));
            PRINT(range_result);
            if (range_result > 800000)
            {
                range_result = 0;
            }
            range_result_sum = range_result_sum + range_result;
            distance = LT.getRangingDistance(RANGING_RESULT_RAW, range_result, distance_adjustment);
            distance_sum = distance_sum + distance;

            PRINT(",Distance,");
            PRINT(distance, 1);
            PRINT("m");
            PRINT(",Time,");
            PRINT(millis() - startrangingmS);
            PRINT("mS");
            PRINTLN();

            delay(TENTH_SEC); // extra delay if no display
        }
        else
        {
            rangeing_error_count++;
            distance = 0;
            range_result = 0;
            PRINT("NotValid");
            PRINT(",Irq,");
            PRINT(IrqStatus, HEX);
            PRINTLN();
        }
    }
    PRINTLN();
    delay(LORA_24_RNG_DELAY_MS);

    if (rangeing_valid_count <= 0)
    {
        PRINT("Range Result Failed");
        PRINTLN();
    }
    else
    {
        range_result_average = (range_result_sum / rangeing_valid_count);
        distance_average = (distance_sum / rangeing_valid_count);

        PRINTLN();
        PRINT("Address:\t\t\t");
        PRINT(LORA_24_RNG_ADDR);
        PRINTLN();
        PRINT("Average Range Result:\t\t");
        PRINT(range_result_average);
        PRINTLN();
        PRINT("Average Distance:\t\t");
        PRINT(distance_average, 1);
        PRINTLN();
        PRINT(F("TotalValid,"));
        PRINT(rangeing_valid_count);
        PRINT(F(",TotalErrors,"));
        PRINTLN(rangeing_error_count);
        PRINTLN();
    }

    // put the radio into receive mode
    setup_lora_24_rx();
}

/* ============= */
/* Ranging Slave */
/* ============= */

void ranging_24_slave(bool calibration_flag)
{
    PRINTLN("Func: ranging_24_slave()");

    setup_ranging_24(false);

    PRINT("LORA_24_RNG_ADDR: ");
    PRINT(LORA_24_RNG_ADDR);
    PRINT(", TXPower: ");
    PRINT(RangingTXPower);
    PRINT("dBm,");
    PRINT(F("CalibrationValue: "));
    // LT.setRangingCalibration(LORA_24_RNG_CALIB_SF10BW400);
    PRINTLN(LT.getSetCalibrationValue());

    unsigned long start = millis();
    while (millis() - start < ranging_threshold)
    {
        LT.receiveRanging(LORA_24_RNG_ADDR, 0, RangingTXPower, NO_WAIT);

        endwaitmS = millis() + LORA_24_WAIT_TIME_MS;

        while (!digitalRead(LORA_24_DIO1_PIN) && (millis() <= endwaitmS))
            ; // wait for Ranging valid or timeout

        if (millis() >= endwaitmS)
        {
            PRINTLN("Error - Ranging Receive Timeout!!");
        }
        else
        {
            IrqStatus = LT.readIrqStatus();
            if (IrqStatus & IRQ_RANGING_SLAVE_RESPONSE_DONE)
            {
                response_sent++;
                if (calibration_flag)
                {
                    start = millis(); // keep listen
                }
                PRINT(response_sent);
                PRINT(" Response sent");
            }
            else
            {
                PRINT("Slave error,");
                PRINT(",Irq,");
                PRINT(IrqStatus, HEX);
                LT.printIrqStatus();
            }
            PRINTLN();
        }
    }

    // put the radio into receive mode
    setup_lora_24_rx();
}

/* =================== */
/* Ranging Calibration */
/* =================== */

void ranging_24_master_calibration(uint8_t destination)
{
    PRINTLN("ranging_24_master_calibration");

    // LoRa header
    memset(rng_request_g.packet, 0x00, LORA_PACKET_LENGTH);
    rng_request_g.msg_type = type_command;
    rng_request_g.local_address = local_address;
    rng_request_g.destination = destination;
    rng_request_g.msg_index = tx_counter;
    rng_request_g.msg_time = millis() / SEC_1;
    rng_request_g.msg_ack_req = 0x01;

    // Command Request
    memset(command_data_g.packet, 0x00, COMMAND_PACKET_LENGTH);
    command_data_g.cmd_code = CMD_RANGING_SLAVE_24;
    command_data_g.cmd_type = cmd_type_none;
    command_data_g.cmd_size = 1;
    uint8_t cmd_size = command_data_g.cmd_size;
    command_data_g.cmd_chksum = checksum_func(command_data_g.cmd_code, command_data_g.cmd_type, command_data_g.cmd_size);
    command_data_g.cmd_payload[0] = 0x01;
    print_command(&command_data_g);
    encrypt_decrypt(&command_data_g);

    memcpy(rng_request_g.msg_payload, command_data_g.packet, COMMAND_HEADER_LENGTH + cmd_size);
    rng_request_g.msg_size = COMMAND_HEADER_LENGTH + cmd_size;

#if RF_433_ENABLE
    send_message_lora_433(rng_request_g.packet, LORA_HEADER_LENGTH + rng_request_g.msg_size);
#endif
    // indicator for ack.
    ranging_request_ack = false;
    // enter into a smart delay to wait for ack.
    unsigned long start = millis();
    received_433_flag = false;
    while (millis() - start < SECS_10)
    {
#if RF_433_ENABLE
        if (received_433_flag)
        {
            received_433_flag = false;
            delay(TENTH_SEC);
#if LORA_SX126X_ENABLE || LORA_SX1262_ENABLE
            on_receive_433();
#endif
            handle_433_message();
#if LORA_SX126X_ENABLE || LORA_SX1262_ENABLE
            setup_lora_433_rx();
#endif
            if (ranging_request_ack)
            {
                ranging_24_calibration(2);
                break;
            }
        }
#endif
    }
}

uint16_t ranging_24_calibration(uint8_t delayS)
{
    PRINTLN("Func: ranging_24_calibration()");

    bool calibration_completed = false;
    uint16_t index, Calibration;
    uint16_t distance_zero_count = 0;
    rangeing_error_count = 0;

    setup_ranging_24(true);

    delay(delayS * SEC_1);

    PRINT("LORA_24_RNG_ADDR, ");
    PRINT(LORA_24_RNG_ADDR);
    PRINT(", TXPower, ");
    PRINT(RangingTXPower);
    PRINT(",dBm");
    PRINT(", Calibration,");
    PRINTLN(LT.getSetCalibrationValue());

    uint16_t calvalue = LT.getSetCalibrationValue();
    uint16_t remainder = calvalue / 10;
    uint16_t calibrationstart = (remainder * 10) - 2000;
    uint16_t calibrationend = (remainder * 10) + 2000;

    PRINT(F("CalibrationStart,"));
    PRINT(calibrationstart);
    PRINT(F(",CalibrationEnd,"));
    PRINTLN(calibrationend);
    PRINTLN();

    for (index = calibrationstart; index <= calibrationend; index = index + 10)
    {
        LT.setRangingCalibration(index);

        PRINT(F("Transmit Ranging, Calibration,"));
        PRINT(index);
        LT.transmitRanging(LORA_24_RNG_ADDR, 0, RangingTXPower, NO_WAIT);

        endwaitmS = millis() + LORA_24_WAIT_TIME_MS;
        startrangingmS = millis();

        while (!(digitalRead(LORA_24_DIO1_PIN)) && (millis() < endwaitmS))
            ; // wait for Ranging valid or timeout

        delay(10);

        IrqStatus = LT.readIrqStatus();
        PRINT(F(",IRQ,"));
        PRINT(IrqStatus, HEX);

        if (IrqStatus & IRQ_RANGING_MASTER_RESULT_TIMEOUT)
        {
            rangeing_error_count++;
            PRINT(F(", Ranging Timeout!  "));
        }
        if (rangeing_error_count > 40)
        {
            break;
        }

        if (millis() > endwaitmS)
        {
            PRINT(F(", Program Timeout"));
        }

        if (IrqStatus & IRQ_RANGING_MASTER_RESULT_VALID)
        {
            rangeing_valid_count++;

            PRINT(F(",Valid"));
            range_result = LT.getRangingResultRegValue(RANGING_RESULT_RAW);
            PRINT(F(",RAW,"));
            PRINT(range_result, HEX);

            distance = LT.getRangingDistance(RANGING_RESULT_RAW, range_result, 1);

            PRINT(F(",Distance,"));
            PRINT(distance, 1);
            PRINT(F("m"));
            PRINT(F(",Time,"));
            PRINT(millis() - startrangingmS);
            PRINT("mS");

            if (distance > 20)
            {
                index += 100;
            }
        }

        PRINT(F(",OK-Count,"));
        PRINT(rangeing_valid_count);
        PRINT(F(",Error-Count,"));
        PRINT(rangeing_error_count);

        if (distance <= 0.5)
        {
            PRINT(F(", Distance is Zero!"));
            distance_zero_count++;
        }

        if (distance_zero_count > 2)
        {
            calibration_completed = true;
            Calibration = index;
            break;
        }

        PRINTLN();
        delay(LORA_24_RNG_DELAY_MS);
    }
    PRINTLN();

    if (calibration_completed)
    {
        return Calibration;
    }
    return 0;
}

#endif