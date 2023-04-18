/**
    @file SAT0_Master.ino
    @brief Master file for SATLLA0.
    This contains the main setup and main loop of the SAT.
    Board: Teensy 3.6
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

#define programversion "SATLLA0"
#define Serial_Monitor_Baud 115200

#include "settings.h"

/* ============ */
/* Setup        */
/* ============ */

void setup()
{
    Serial.begin(Serial_Monitor_Baud);
    delay(SEC_1);

    PRINTLN("************* START SETUP *************");

    PRINTLN();
    PRINT(__TIME__);
    PRINT(F(" "));
    PRINTLN(__DATE__);
    PRINTLN(programversion);
    PRINTLN(SAT_NAME);
    PRINT("F_CPU: ");
    PRINTLN(ltoa(F_CPU, buffer_g, 10));
    if (F_CPU > 24000000)
    {
        PRINTLN("Warning: CPU speed should be reduced to keep power consumption low!");
    }
    PRINTLN();

#if RTC_ENABLE
    // set the Time library to use Teensy 3.0's RTC to keep time
    setSyncProvider(getTeensy3Time);

    if (timeStatus() == timeNotSet)
    {
        // setTime(00,00,00,01,01,2023);
        rtc_setup();
        Teensy3Clock.set(now());
    }
    digitalClockDisplay();
#endif

    // SPI
    PRINTLN(F("SCK PIN: 13"));
//     SPI.setSCK(13); // For teensy 3.6/4.1 - Default
    SPI.begin();
    delay(TENTH_SEC);

    Wire.begin();
    delay(TENTH_SEC);

    sat_state = sat_high_power;

    // FATs
    setup_fat_pin_out();

    // Watchdog Setup
#if WD_ENABLE
    wd_setup();
#endif

    // SD Card Setup
#if SD_ENABLE
    SD_Card_setup();
    delay(TENTH_SEC);
#endif

    // Large FS if Teensy 4.1
#if TNSYFS_ENABLE
    tnsyfs_setup();
#endif

    // Mode LED
#if MODELED_ENABLE
    modeled_setup();
#endif

    // Blink LED
#if MAINLED_ENABLE
    LED_setup();
#endif

    // Blink Laser
#if LASER_ENABLE
    laser_setup();
#endif

    // Global Settings Setup
#if EEPROM_ENABLE
    get_global_settings();
    // reset_global_setting();
    delay(TENTH_SEC);
#endif

    // IMU
#if IMU_ENABLE
    // IMU - LSM6DS33
    imu_turn_on(); // calling sensor_imu_setup()
    // imu_read();
#if IMU_LSM6DS33_ENABLE
    // sensor_imu_setup(); // calling sensor_imu_setup()
    imu_read();
#endif
    delay(TENTH_SEC);
#endif

    // GPS
#if GPS_ENABLE
    // turn on
    gps_turn_on();
    gps_setup();
    delay(TENTH_SEC);
#endif

    // NTC's
#if NTC_ENABLE
    sensor_ntc_setup();
#endif

    // RPI 0
#if RPI_ENABLE
    rpi_serial_setup();
    delay(TENTH_SEC);
#endif

    // LoRa 433 Setup
#if RF_433_ENABLE
    lora_433_turn_on();
    delay(TENTH_SEC);
    modem_fsk_433_setup(); // Need to set FSK as well

    lora_433_setup();
#endif

    // LoRa SX1280
#if RF_24_ENABLE
    lora_24_turn_on();
    delay(TENTH_SEC);

    lora_24_setup();
    lora_433_setup(); // fix the issue of timeout -5
#endif

    // SF BBSTR
#if BBSTR_BQ27441_ENABLE
    setup_BQ27441();
    delay(TENTH_SEC);
    battery_read();
    check_battery_status();
#endif

    // initial Boot include antenna deploy
    initial_boot();

    // set flags to false
    received_433_flag = false;

    // increment restart index
#if SD_ENABLE
    if (sdcard_is_on)
    {
        inc_reset_index();
    }
#endif

#if TNSYFS_ENABLE
    if (!sdcard_is_on || save_to_flash)
    {
        if (flash_is_on)
        {
            fs_inc_reset_index();
        }
    }
#endif

    PRINTLN("**************************  END SETUP ************************** ");
}

/* ============ */
/* Loop         */
/* ============ */

void loop()
{
    PRINTLN("************************** START LOOP **************************");
    PRINT("Func: loop()");
    loop_start_millis = millis();
    PRINT(", Secs:\t");
    PRINTLN(loop_start_millis / SEC_1);

#if RTC_ENABLE
#ifdef DEBUG
    digitalClockDisplay();
#endif
#endif
    // WD pulse
#if WD_ENABLE
    wd_heartbeat();
#endif

    // battery read
#if BBSTR_BQ27441_ENABLE
    battery_read();
    // set sat power: panic, med, high
    check_battery_status();
#endif

    // check health
    set_sat_health();

    // if battery is high or 1 hour pass from last beacon, enable sending beacon
    if (sat_state > sat_panic_power || loop_start_millis - last_sent_beacon_time > HOUR_1) // > 3500
    {
        // send beacon procedure:
        // send 433. wait 1 sec. send 2.4. wait for Rx 20 sec.
        // sleep
        if (loop_start_millis - last_sent_beacon_time > send_beacon_threshold)
        {
            // set long or short beacons
            b_type = (beacon_type_e)switch_beacon_type();
            // prepare the beacon
            handle_beacon(b_type, false); // (beacon, save to log flag).
#if RF_433_ENABLE
            // send via 433
            send_beacon(b_type, lora_433);
#endif
            last_sent_beacon_time = millis();

#if RF_24_ENABLE
            if (sat_state > sat_med_power) // > 3500
            {
                // smart delay between beacons
                smart_delay(delay_between_beacons); // 2 secs
                // send via 24
                send_beacon(b_type, lora_24);
            }
#endif
        }
    }

    // smart delay: keep Radio and GPS feed
    // listen for incoming messages
    smart_delay(ts_loop_runtime); // default 20 secs

    // general method to handle tasks.
    // if command arrive, it will be after the smart delay
    handle_tasks();

    // if mission started, then don't sleep during that period.
    if (!in_mission())
    {
#if RF_433_ENABLE
        // if GFSK is allowed, send a GFSK beacon.
        if (gfsk_beacon && sat_state > sat_panic_power) // > 3500
        {
            // switch to GFSK mode.
            set_modem(modem_fsk_433);
            // send beacon
            send_beacon(b_type, lora_433);
            // no wait for incoming GFSK so switch back to LoRa
            set_modem(modem_lora_433);
        }
#endif

#if MODELED_ENABLE
        stop_modeled_thread();
#endif

        unsigned long min_sleep_delay = min(sleep_delay_non_panic, millis() - last_sent_beacon_time);
        if (sat_state <= sat_panic_power)
        {
            min_sleep_delay = sleep_delay_panic;
        }
        // else, sleep for 15 secs.
        sleep_delay(min_sleep_delay); // milliseconds

#if MODELED_ENABLE
        start_modeled_thread();
#endif
    }

    PRINTLN("************************** END LOOP **************************");
}

/* ============ */
/* smart_delay  */
/* ============ */

static void smart_delay(const unsigned long ms)
{
    PRINTLN("************* START SMART_DELAY *************");
    PRINT("Func: smart_delay() For:\t");
    PRINTLN(ms / SEC_1);

    unsigned long start = millis();

    PRINT("Secs:\t");
    PRINTLN(start / SEC_1);

    // loop smart delay
    while (millis() - start < ms && start)
    {

#if GPS_ENABLE
        // feed gps
        if (gps_is_on)
        {
            feed_gps1();
        }
#endif

#if RF_433_ENABLE
        // message recevied, break and handle
        if (received_433_flag)
        {
            // break the loop
            break;
        }
#endif

#if RF_24_ENABLE
        // message recevied, break and handle
        if (received_24_flag)
        {
            // break the loop
            break;
        }
#endif

#ifdef DEBUG
        // serial recevied, break and handle
        if (Serial.available() || ant_button_state)
        {
            break;
        }
#endif

// watch dog
#if WD_ENABLE
        wd_heartbeat();
#endif
    }
    PRINTLN("************* END SMART_DELAY *************");

    // smart delay completed. if message arrived, process.
#if RF_433_ENABLE
    if (received_433_flag)
    {
        received_433_flag = false;
        delay(TENTH_SEC);
        handle_433_message();

#ifdef DEBUG
        // blink one long if got a message
        modeled_blink_period = MODELED_LONG;
        modeled_blink_count = 1;
#endif
    }
#endif

#if RF_24_ENABLE
    if (received_24_flag)
    {
        received_24_flag = false;
        delay(TENTH_SEC);
        on_receive_24();
        handle_24_message();
        setup_lora_24_rx();

#ifdef DEBUG
        // blink one long if got a message
        modeled_blink_period = MODELED_LONG;
        modeled_blink_count = 1;
#endif
    }
#endif

    // if stop tx requested, check if it pass threshold and reset flag
    if (stop_tx_flag > 0 && millis() - stop_tx_flag > STOP_TX_FLAG_THRESHOLD)
    {
        stop_tx_flag = 0;
    }

    // if restart requested wait 5 secs and restart
    if (restart_requested_flag)
    {
#ifdef DEBUG
        // blink one long if got a message
        modeled_blink_period = MODELED_LONG;
        modeled_blink_count = 3;
#endif
        restart_requested_flag = false;
        delay(SECS_3);
        CPU_RESTART;
    }

#ifdef DEBUG
    if (Serial.available() || ant_button_state)
    {
        serial_event();
    }
#endif
}

/* ============ */
/* sleep delay  */
/* ============ */

static void sleep_delay(const unsigned long ms)
{
    PRINTLN("************* START SLEEP_DELAY *************");
    PRINT("Func: sleep_delay() For:\t");
    PRINTLN(ms / SEC_1);

    unsigned long start = millis();

    PRINT("Secs:\t");
    PRINTLN(start / SEC_1);

#if IMU_ENABLE
    if (imu_is_on && !imu_is_sleep)
    {
        imu_sleep();
    }
#endif

#if RF_433_ENABLE
    if (lora_433_is_on && !lora_433_is_sleep)
    {
        lora_433_sleep();
    }
#endif

#if RF_24_ENABLE
    if (lora_24_is_on && !lora_24_is_sleep)
    {
        lora_24_sleep();
    }
#endif

#if GPS_ENABLE
    if (gps_is_on && !gps_is_sleep)
    {
        gps_sleep();
    }
#endif

// watchdog
#if WD_ENABLE
    wd_heartbeat();
#endif

    delay(ms);

    PRINTLN("************* RESUME SLEEP DELAY *************");

#if IMU_ENABLE
    if (imu_is_on && imu_is_sleep)
    {
        imu_resume();
    }
#endif

#if RF_433_ENABLE
    if (lora_433_is_on && lora_433_is_sleep)
    {
        lora_433_resume();
    }
#endif

#if RF_24_ENABLE
    if (lora_24_is_on && lora_24_is_sleep)
    {
        lora_24_resume();
    }
#endif

#if GPS_ENABLE
    if (gps_is_on && gps_is_sleep)
    {
        gps_resume();
    }
#endif

#ifdef DEBUG
    if (Serial.available())
    {
        serial_event();
    }
#endif

#if WD_ENABLE
    wd_heartbeat();
#endif

    PRINTLN("************* END SLEEP_DELAY *************");
}

/* ============== */
/* SAT In Mission */
/* ============== */

bool in_mission()
{
    PRINT("Func: in_mission():\t");
    bool ret_value = (mission_rpi_flag || mission_laser_blink_flag || mission_led_blink_flag);
    PRINTLN((ret_value) ? "true" : "false");
    return ret_value;
}

/* ============== */
/* SAT Health     */
/* ============== */

void set_sat_health()
{
    PRINTLN("Func: set_sat_health()");

    switch (sat_state)
    {
    case sat_panic_power:
    {
        PRINTLN("Sat Health: PANIC");
        // turn off all
#if GPS_ENABLE
        if (gps_is_on)
        {
            gps_turn_off();
        }
#endif

#if IMU_ENABLE
        if (imu_is_on)
        {
            imu_turn_off();
        }
#endif

#if RF_433_ENABLE
        if (lora_433_is_on)
        {
            lora_433_turn_off(); // TODO: preferred sleep
        }
        gfsk_beacon = false;
#endif

#if RF_24_ENABLE
        if (lora_24_is_on)
        {
            lora_24_turn_off();
        }
#endif

        modeled_blink_count = 3;
    }
    break;

    case sat_med_power:
    {
        PRINTLN("Sat Health: MEDIUM");
        // turn on LoRa, BNO and keep GPS off

#if GPS_ENABLE
        if (gps_is_on)
        {
            gps_turn_off();
        }
#endif

#if IMU_ENABLE
        if (!imu_is_on)
        {
            imu_turn_on();
        }
#endif

#if RF_433_ENABLE
        if (!lora_433_is_on)
        {
            lora_433_turn_on();
            delay(TENTH_SEC);
            lora_433_setup(); // after tuning on, need setup
        }
        gfsk_beacon = false;
#endif

#if RF_24_ENABLE
        if (lora_24_is_on)
        {
            lora_24_turn_off();
        }
#endif

        modeled_blink_count = 2;
    }
    break;

    case sat_high_power:
    {
        PRINTLN("Sat Health: HIGH");
        // turn on all
#if GPS_ENABLE
        if (!gps_is_on)
        {
            gps_turn_on();
        }
#endif

#if IMU_ENABLE
        if (!imu_is_on)
        {
            imu_turn_on();
        }
#endif

#if RF_433_ENABLE
        if (!lora_433_is_on)
        {
            lora_433_turn_on();
            delay(TEN_MS);    // let FATs times to go to HIGH
            lora_433_setup(); // after tuning on, need setup
        }
        gfsk_beacon = true;
#endif

#if RF_24_ENABLE
        if (!lora_24_is_on)
        {
            lora_24_turn_on();
            delay(TEN_MS); // let FATs times to go to HIGH
        }
        // resume checking the device. if problem occur will call setup.
        lora_24_resume();
#endif

        modeled_blink_count = 1;
    }
    break;

    default:
        break;
    }

#if GPS_ENABLE
    gps_health();
#endif
}

/* ================ */
/* Beacon Type */
/* ================ */
uint8_t switch_beacon_type()
{
    PRINTLN("Func: switch_beacon_type()");
    beacon_type_e b_type = beacon_type_long;
    // if mission started, send only long beacons.
    if (sat_state >= sat_med_power || in_mission())
    {
        b_type = beacon_type_long;
    }
    else
    {
        // if low power, then switch to short but keep 5 long each hr
        b_type = beacon_type_short;
        if (millis() - last_sent_long_beacon > HOUR_1)
        {
            b_type = beacon_type_long;
            sent_long_beacon_counter++;
            if (sent_long_beacon_counter > 5)
            {
                sent_long_beacon_counter = 0;
                last_sent_long_beacon = millis();
            }
        }
    }
    return b_type;
}

/* ============ */
/* Switch Modem */
/* ============ */

int16_t set_modem(modem_e modem)
{
    PRINTLN("Func: set_modem()");

    int16_t state = 0;

    if (current_modem == modem)
    {
        return state;
    }

    // initialize requested modem
    switch (modem)
    {
    case modem_lora_433:
#if RF_433_ENABLE
        setup_lora_433(&modem_lora_433_g);
#endif
        break;

    case modem_fsk_433:
    {
#if RF_433_ENABLE
        setup_fsk_433(&modem_fsk_433_g);
#endif
    }
    break;

    default:
        PRINT(F("Unknown modem: "));
        PRINTLN(modem);
        setup_lora_433(&modem_lora_433_g);
    }

    // radio.setWhitening(true, WHITENING_INITIAL);

    // handle possible error codes
    PRINT(F("Radio init "));
    PRINTLN(state);
    delay(TEN_MS);
    if (state != 0)
    {
        // radio chip failed, restart
    }

    // save current modem
    current_modem = modem;
    return (state);
}

/* ================ */
/* Auto Tasks IL  */
/* ================ */

int8_t handle_auto_tasks()
{
    PRINTLN("Func: handle_auto_tasks()");
    int8_t mission = 0x00;

    // Auto: Once a day
    memset(command_data_g.packet, 0x00, COMMAND_PACKET_LENGTH);

    unsigned long auto_task = millis() - last_auto_task;

    if (auto_task > DAY_1 && auto_task <= DAYS_2)
    {
        PRINTLN("Func: handle_auto_tasks()::rpi_cmd_take_photo");
        memcpy(command_data_g.packet, rpi_cmd_take_photo, 5); // 0x5500010004
        mission = 0x04;
    }
    else if (auto_task > DAYS_2 && auto_task <= DAYS_3)
    {
        PRINTLN("Func: handle_auto_tasks()::rpi_cmd_new_take_photo");
        memcpy(command_data_g.packet, rpi_cmd_new_take_photo, 5); // 0x5500010004
        mission = 0x0f;
    }
    else if (auto_task > DAYS_3 && auto_task <= DAYS_4)
    {
        PRINTLN("Func: handle_auto_tasks()::rpi_cmd_sit");
        memcpy(command_data_g.packet, rpi_cmd_sit, 5); // 0x5500010004
        mission = 0x05;
    }
    else if (auto_task > DAYS_4 && auto_task <= DAYS_5)
    {
        PRINTLN("Func: handle_auto_tasks()::rpi_cmd_rwcs");
        memcpy(command_data_g.packet, rpi_cmd_rwcs, 5); // 0x5500010004
        mission = 0x07;
    }
    else if (auto_task > DAYS_5 && auto_task <= DAYS_6)
    {
        PRINTLN("Func: handle_auto_tasks()::rpi_cmd_seek_laser");
        memcpy(command_data_g.packet, rpi_cmd_seek_laser, 5); // 0x5500010004
        mission = 0x06;
    }
    else if (auto_task > DAYS_6 && auto_task <= WEEK_1)
    {
#if TNSYFS_ENABLE        
        PRINTLN("Func: handle_auto_tasks()::Last Mission");
        uint16_t mission = fs_get_misson_index();
        uint16_t file_index = 0x01;
        uint8_t message_len = 0x32;
        uint8_t radio = 0x00; // 433
        prepare_and_send_ld_packet(mission, file_index, message_len, radio);
#endif
    } 
    else if (auto_task > WEEKS_4)
    {
        PRINTLN("Func: handle_auto_tasks()");
        // Reser last_auto_task after a month
        last_auto_task = millis();
    }

    return mission;
}

/* ============== */
/* Handle Tasks   */
/* ============== */
void handle_tasks()
{
    PRINTLN("Func: handle_tasks()");

    if (millis() - last_save_oper_half_days > HALF_DAY)
    {
        last_save_oper_half_days = millis();
        uint8_t error = 0;

        if (sdcard_is_on)
        {
#if SD_ENABLE
            error = increase_operation_half_days_file();
#endif
        }

        if (error || !sdcard_is_on || save_to_flash)
        {
            if (flash_is_on)
            {
#if TNSYFS_ENABLE
                error = fs_increase_operation_half_days_file();
#endif
            }
        }

        last_ranging_24 = 0L;
    }

#if RF_433_ENABLE
    // return to default radio after 1 day
    if (millis() - change_radio_lora_433_setting_timer > DAY_1 && change_radio_lora_433_setting_timer)
    {
        change_radio_lora_433_setting_timer = 0;
        modem_lora_433_setup();
    }

    // return to default radio after 1 day
    if (millis() - change_radio_fsk_433_setting_timer > DAY_1 && change_radio_fsk_433_setting_timer)
    {
        change_radio_fsk_433_setting_timer = 0;
        modem_fsk_433_setup();
    }
#endif

#if RF_24_ENABLE
    if (millis() - change_radio_lora_24_setting_timer > DAY_1 && change_radio_lora_24_setting_timer)
    {
        change_radio_lora_24_setting_timer = 0;
        modem_lora_24_setup();
    }
#endif

    if (!in_mission() && sat_state >= sat_med_power)
    {
        uint8_t mission = handle_auto_tasks();
        if (mission > 0)
        {
#if RPI_ENABLE
            rpi1_turn_on();
            rpi1_command_noack_flag = true;
#endif
        }
    }

#if RPI_ENABLE
    // Check if RPI is ready to send data
    if (rpi1_is_on)
    {
        if (rpi1_command_noack_flag)
        {
            if (rpi1_command_tries < RPI_COMMAND_TRIES) // add tries. if tries > x, then stop trying.
            {
                PRINTLN("RPI: rpi1_command_noack_flag is true");
                activate_command(&command_data_g);
                rpi1_command_tries++;
                PRINT("RPI: rpi1_command_tries:\t");
                PRINTLN(rpi1_command_tries);
            }
            else
            {
                PRINTLN("RPI: Turn off RPI via FAT");
                rpi1_turn_off();
                // reset flags
                rpi1_command_noack_flag = false;
                rpi_task_completed_flag = false;
                rpi1_command_tries = 0;
            }
        }
        else
        {
            PRINTLN("RPI: rpi1_command_noack_flag is false");
            unsigned long time_on = (loop_start_millis > rpi1_is_on) ? loop_start_millis - rpi1_is_on : rpi1_is_on - loop_start_millis; // if start in the same loop then this will be minus.
            // PRINTLN(time_on);
            PRINT("RPI: RPI_1 Is On: ");
            PRINT(time_on / SEC_1);
            PRINTLN(" secs");

            // if rpi is on, check status
            check_rpi_status(rpi1_serial);

            // TODO: Remove this comment
            if (time_on >= MINUTES_3 && time_on < MINUTES_4 || rpi_task_completed_flag)
            {
                PRINTLN("RPI: Sending halt command");
                // command_data_t shoutdown_cmd_g;
                memset(shoutdown_cmd_g.packet, 0x00, COMMAND_PACKET_LENGTH);
                shoutdown_cmd_g.cmd_code = CMD_RPI_1_COMMAND;
                shoutdown_cmd_g.cmd_type = cmd_type_param;
                shoutdown_cmd_g.cmd_size = 1;
                shoutdown_cmd_g.cmd_payload[0] = RPI_CMD_SHUT_DOWN;
                int commandStatus = activate_command(&shoutdown_cmd_g);
                rpi_task_completed_flag = false;
                mission_rpi_flag = false;
            }
            else if (time_on >= MINUTES_4)
            {
                PRINTLN("RPI: Turn off RPI via FAT");
                rpi1_command_noack_flag = false;
                rpi_task_completed_flag = false;
                rpi1_command_tries = 0;
                rpi1_turn_off();
            }
        }
    }
#endif

#if RPI_ENABLE
    // If should turn rpi with dealay
    if (should_turn_rpi1_delay_flag)
    {
        if (millis() - turn_rpi1_delay > turn_rpi1_msec)
        {
            // Turn on RPI
            rpi1_turn_on();
            rpi1_command_noack_flag = true;
            memcpy(command_data_g.packet, command_data_rpi1_g.packet, COMMAND_HEADER_LENGTH + command_data_rpi1_g.cmd_size);
            // sat_state = sat_mission;
            should_turn_rpi1_delay_flag = false;
        }
    }
#endif

    // If should echo msg with delay is on, then wait.
    if (should_echo_msg_delay_flag)
    {
        if (millis() - start_echo_msg_delay > echo_msg_delay_msec)
        {
            // Execute echo message
            should_echo_msg_delay_flag = false;

#if (RF_433_ENABLE || RF_24_ENABLE)
            uint8_t radio = command_data_echo_msg.cmd_payload[0];
            uint8_t times = command_data_echo_msg.cmd_payload[1];
            if (times == 0 || times > MSG_RELAY_MAX_TIMES)
            {
                times = 1;
            }
            for (uint8_t i = 0; i < times; i++)
            {
                if (radio == lora_433)
                {
#if RF_433_ENABLE
                    wrap_message_433(command_data_echo_msg.cmd_payload + 2, command_data_echo_msg.cmd_size - 2, type_echo);
                    delay(SEC_1);
#endif
                }
                else
                {
#if RF_24_ENABLE
                    wrap_message_24(command_data_echo_msg.cmd_payload + 2, command_data_echo_msg.cmd_size - 2, type_echo);
                    delay(SEC_1);
#endif
                }
            }
#endif
            delay(SEC_1);
#if RF_433_ENABLE
            wrap_message_24(command_data_echo_msg.cmd_payload, command_data_echo_msg.cmd_size, type_echo);
#endif
        }
    }

#if MAINLED_ENABLE
    blink_LED_task();
#endif

#if LASER_ENABLE
    blink_LASER_task();
#endif
}

/* ============== */
/* Handle Beacon  */
/* ============== */

void handle_beacon(beacon_type_e b_type, bool save_to_log_flag)
{
    PRINTLN("Func: handle_beacon()");

    // battery read
#if BBSTR_BQ27441_ENABLE
    battery_read();
#endif

    // IMU read
#if IMU_ENABLE
    imu_read();
#endif

    // NTC Temp read
#if NTC_ENABLE
    ntc_read();
#endif

    // load gps data into struct
#if GPS_ENABLE
    gps_prepare_data();
#endif

    if (sdcard_is_on)
    {
#if SD_ENABLE
        sd_ls(&sd_data_g);
#endif
    }

    if (!sdcard_is_on || save_to_flash)
    {
        if (flash_is_on)
        {
#if TNSYFS_ENABLE
            fs_ls(&sd_data_g);
#endif
        }
    }

    // Prepare Infodata
    info_data_prepare(&info_data_g);

    // Prepare Metadata
    prepare_meta2beacon();

    // Prepare the beacon struct
    if (b_type == beacon_type_short)
    {
        prepare_short_beacon(&short_beacon_g);
    }
    else
    {
        prepare_long_beacon(&beacon_g);

        // Save beacon to SD Card
        if ((millis() - last_save_to_log > save_to_log_threshold) || save_to_log_flag)
        {
            last_save_to_log = millis();

            if (sdcard_is_on)
            {
#if SD_ENABLE
                save_to_log((uint8_t *)beacon_g.msg_payload, beacon_g.msg_size);
#endif
            }
            // don't save to flash
        }
    }

    delay(TENTH_SEC);
}

void send_beacon(beacon_type_e b_type, lorafreq_e freq)
{
    PRINTLN(F("Func: send_beacon()"));

    if (stop_tx_flag > 0)
    {
        return;
    }

#ifdef DEBUG
    // blink one long before sending beacon
    modeled_blink_period = MODELED_LONG;
    modeled_blink_count = 1;
#endif

    if (b_type == beacon_type_long)
    {
        PRINTLN("Func: send_beacon()::Long");
        if (freq == lora_433)
        {
#if RF_433_ENABLE
            if (lora_433_is_on)
            {
                PRINTLN("Func: send_beacon()::LoRa 433");
                bool is_error = send_message_lora_433(beacon_g.packet, LORA_HEADER_LENGTH + beacon_g.msg_size);
                if (is_error)
                {
                    // reset 433
                    lora_433_setup();
                }
            }
#endif
        }
        else
        {
#if RF_24_ENABLE
            if (lora_24_is_on)
            {
                PRINTLN("Func: send_beacon()::LoRa 24");
                send_message_lora_24(beacon_g.packet, LORA_HEADER_LENGTH + beacon_g.msg_size);
            }
#endif
        }
    }
    else
    {
        if (freq == lora_433)
        {
#if RF_433_ENABLE
            if (lora_433_is_on)
            {
                PRINTLN("Func: send_beacon()::LoRa 433");
                bool is_error = send_message_lora_433(short_beacon_g.packet, SHRT_BCN_PACKET_LENGTH);
                if (is_error)
                {
                    // reset 433
                    lora_433_setup();
                }
            }
#endif
        }
        else
        {
#if RF_24_ENABLE
            if (lora_24_is_on)
            {
                PRINTLN("Func: send_beacon()::LoRa 24");
                send_message_lora_24(short_beacon_g.packet, SHRT_BCN_PACKET_LENGTH);
            }
#endif
        }
    }
}

/* ============== */
/* Prepare Info Data */
/* ============== */

void info_data_prepare(info_data_t *info_data)
{
    PRINTLN("Func: info_data_prepare()");

    if (sdcard_is_on)
    {
#if SD_ENABLE
        info_data->reset_index = (uint8_t)get_reset_index();
#endif
    }

    if (!sdcard_is_on || save_to_flash)
    {
        if (flash_is_on)
        {
#if TNSYFS_ENABLE
            info_data->reset_index = (uint8_t)fs_get_reset_index();
#endif
        }
    }
    info_data->rx_sat_counter = rx_sat_counter;
    info_data->msg_received = rx_counter;
}

void prepare_meta2beacon()
{
    if (LDMETATXbuffer.isEmpty())
    {
        uint8_t file_counter = 0;
        if (sdcard_is_on)
        {
#if SD_ENABLE
            // Check if there is data to be sent
            file_counter = sd_ls_folder(OUTBOX_FOLDER);
            delay(TENTH_SEC);

            if (file_counter > 0)
            {
                send_meta_files(OUTBOX_FOLDER);
            }
#endif
        }

        // add file_counter so if no meta on SD force try FS
        if (!sdcard_is_on || save_to_flash || !file_counter)
        {
            if (flash_is_on)
            {
#if TNSYFS_ENABLE
                // Check if there is data to be sent
                file_counter = fs_ls_folder(OUTBOX_FOLDER);
                delay(TENTH_SEC);

                if (file_counter > 0)
                {
                    fs_send_meta_files(OUTBOX_FOLDER, size_last_meta);
                }
#endif
            }
        }
    }
    else
    {
        // If there are meta pending, pull another one.
        PRINT(F("LDMETATXbuffer Element:\t"));
        PRINTLN(LDMETATXbuffer.numElements());

        memset(ld_meta_data_b.packet, 0x00, LD_META_DATA_LENGTH);
        LDMETATXbuffer.pull(&ld_meta_data_b);
    }
}

/* ============== */
/* Prepare Beacon */
/* ============== */

void prepare_long_beacon(lora_data_t *beacon)
{
    PRINTLN(F("Func: prepare_long_beacon()"));

    beacon->local_address = local_address;
    beacon->destination = destination;
    beacon->msg_type = type_becon;
    beacon->msg_size = (GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH + SD_DATA_LENGTH + INFO_DATA_LENGTH);
    beacon->msg_index = tx_counter;
    beacon->msg_time = millis() / SEC_1;
    beacon->msg_ack_req = 0;

    memcpy(beacon->msg_payload, gps_data_g.packet, GPS_PACKET_LENGTH);
    memcpy(beacon->msg_payload + GPS_PACKET_LENGTH, sns_data_g.packet, SNS_PACKET_LENGTH);
    memcpy(beacon->msg_payload + GPS_PACKET_LENGTH + SNS_PACKET_LENGTH, battery_data_g.packet, BATTERY_PACKET_LENGTH);
    memcpy(beacon->msg_payload + GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH, sd_data_g.packet, SD_DATA_LENGTH);
    memcpy(beacon->msg_payload + GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH + SD_DATA_LENGTH, info_data_g.packet, INFO_DATA_LENGTH);

    if (ld_meta_data_b.ld_size > 0)
    {
        memcpy(beacon->msg_payload + GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH + SD_DATA_LENGTH + INFO_DATA_LENGTH, ld_meta_data_b.packet, LD_META_HEADER_LENGTH + ld_meta_data_b.ld_size);
        beacon->msg_size += (LD_META_HEADER_LENGTH + ld_meta_data_b.ld_size);
    }
}

/* ==================== */
/* Prepare Short Beacon */
/* ==================== */

void prepare_short_beacon(shrt_bcn_data_t *beacon)
{
    PRINTLN(F("Func: prepare_short_beacon()"));

    beacon->local_address = local_address;
    beacon->msg_type = type_sbeacon;
    beacon->msg_index = tx_counter;
    beacon->sd_outbox = sd_data_g.sd_outbox;
    beacon->msg_received = rx_counter;

    beacon->battery_volts = battery_data_g.battery_volts;
    beacon->battery_current = battery_data_g.battery_current;

    beacon->sns_ntc1 = sns_data_g.sns_ntc1;
    beacon->sns_ntc2 = sns_data_g.sns_ntc2;
}

/* ================== */
/* Print Short Beacon */
/* ================== */

void print_short_beacon(uint8_t *message, uint8_t messageLength)
{
    PRINTLN("Func: print_short_beacon()");

    memcpy(prnt_short_beacon_g.packet, message, messageLength);

    PRINTLN("***********************");
    PRINT("L.Addr:\t\t0x");
    PRINTLN(prnt_short_beacon_g.local_address, HEX);
    PRINT("Type:\t\t0x");
    PRINTLN(prnt_short_beacon_g.msg_type, HEX);
    PRINT("Index:\t\t");
    PRINTLN(prnt_short_beacon_g.msg_index);
    PRINT("Bat Voltage:\t");
    PRINTLN(prnt_short_beacon_g.battery_volts);
    PRINT("Current:\t");
    PRINTLN(prnt_short_beacon_g.battery_current);
    PRINT("SD Outbox:\t");
    PRINTLN(prnt_short_beacon_g.sd_outbox);
    PRINT("Msg Received:\t");
    PRINTLN(prnt_short_beacon_g.msg_received);
    PRINT("NTC1:\t\t");
    PRINTLN(prnt_short_beacon_g.sns_ntc1);
    PRINT("NTC2:\t\t");
    PRINTLN(prnt_short_beacon_g.sns_ntc2);

    PRINTLN("Message: ");
    print_buffer(prnt_short_beacon_g.packet, SHRT_BCN_PACKET_LENGTH);
    PRINTLN();

    PRINTLN("***********************");
}
/* ============= */
/* Print Message */
/* ============= */

void print_message(uint8_t *message, uint8_t messageLength)
{
    PRINTLN("Func: print_message()");

    // lora_data_t lora_message;
    memcpy(prnt_msg_g.packet, message, messageLength);

    if (prnt_msg_g.msg_type == type_sbeacon)
    {
        print_short_beacon(message, messageLength);
        return;
    }

    PRINTLN("***********************");
    PRINT("L.Addr:\t\t0x");
    PRINTLN(prnt_msg_g.local_address, HEX);
    PRINT("Dest:\t\t0x");
    PRINTLN(prnt_msg_g.destination, HEX);
    PRINT("Type:\t\t0x");
    PRINTLN(prnt_msg_g.msg_type, HEX);
    PRINT("Size:\t\t");
    PRINTLN(prnt_msg_g.msg_size);
    PRINT("Total Size:\t");
    PRINTLN(prnt_msg_g.msg_size + LORA_HEADER_LENGTH);
    PRINT("Index:\t\t");
    PRINTLN(prnt_msg_g.msg_index);
    PRINT("Time:\t\t");
    PRINTLN(prnt_msg_g.msg_time);
    PRINT("Ack Req:\t");
    PRINTLN(prnt_msg_g.msg_ack_req);

    // String message_hex;
    PRINTLN("Header: ");
    print_buffer(prnt_msg_g.packet, LORA_HEADER_LENGTH);
    if (prnt_msg_g.msg_size)
    {
        PRINTLN("Payload: ");
        print_buffer(prnt_msg_g.packet + LORA_HEADER_LENGTH, prnt_msg_g.msg_size);
    }
    PRINTLN();
    PRINTLN("***********************");

    memset(gps_data_p.packet, 0, GPS_PACKET_LENGTH);
    memset(sns_data_p.packet, 0, SNS_PACKET_LENGTH);
    memset(energy_data_p.packet, 0, ENERGY_PACKET_LENGTH);
    memset(sd_data_p.packet, 0, SD_DATA_LENGTH);
    memset(radio_data_p.packet, 0, RADIO_DATA_LENGTH);
    memset(info_data_p.packet, 0, INFO_DATA_LENGTH);
    memset(ld_meta_data_p.packet, 0, LD_META_DATA_LENGTH);

    if (prnt_msg_g.msg_size > 0)
    {
        if (prnt_msg_g.msg_type == type_becon)
        {
            memcpy(gps_data_p.packet, prnt_msg_g.msg_payload, GPS_PACKET_LENGTH);
            print_gps_data(&gps_data_p);
            PRINTLN();

            memcpy(sns_data_p.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH, SNS_PACKET_LENGTH);
            print_sensor_data(&sns_data_p);
            PRINTLN();

            memcpy(battery_data_g.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH + SNS_PACKET_LENGTH, BATTERY_PACKET_LENGTH);
            print_battery_stats(&battery_data_g);
            PRINTLN();

            memcpy(sd_data_p.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH, SD_DATA_LENGTH);
            print_sd_data(&sd_data_p);
            PRINTLN();

            memcpy(info_data_p.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH + SD_DATA_LENGTH, INFO_DATA_LENGTH);
            print_info_data(&info_data_p);
            PRINTLN();

            uint8_t ld_size = prnt_msg_g.msg_size - (GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH + SD_DATA_LENGTH + INFO_DATA_LENGTH);
            if (ld_size > 0)
            {
                memcpy(ld_meta_data_b.packet, prnt_msg_g.msg_payload + GPS_PACKET_LENGTH + SNS_PACKET_LENGTH + BATTERY_PACKET_LENGTH + SD_DATA_LENGTH + INFO_DATA_LENGTH, LD_META_HEADER_LENGTH + ld_size);
                print_ld_meta_data(&ld_meta_data_b);
            }
        }

        if (prnt_msg_g.msg_type == type_ack)
        {
            memcpy(radio_data_p.packet, prnt_msg_g.msg_payload, RADIO_DATA_LENGTH);
            print_radio_data(&radio_data_p);
            PRINTLN();
        }
    }
    PRINTLN("***********************");
}

/* ============= */
/* Print Radio   */
/* ============= */

void print_radio_data(radio_data_t *radio_data)
{
    PRINTLN("Func: print_radio_data()");

    PRINT("RSSI:\t");
    int rssi = radio_data->rssi * -1;
    PRINTLN(rssi);
    PRINT("SNR:\t");
    int snr = radio_data->snr;
    PRINTLN(snr);
}

/* ============= */
/* Print Info   */
/* ============= */

void print_info_data(info_data_t *info_data)
{
    PRINTLN("Func: print_info_data()");

    PRINT("Reset index:\t");
    PRINTLN(info_data->reset_index);
    PRINT("RX SAT Counter:\t");
    PRINTLN(info_data->rx_sat_counter);
    PRINT("Message Received:\t");
    PRINTLN(info_data->msg_received);
}

/* ============= */
/* Print SD Data */
/* ============= */

void print_sd_data(sd_data_t *sd_data)
{
    PRINTLN("Func: print_sd_data()");

    PRINT("Outbox:\t");
    PRINTLN(sd_data->sd_outbox);
    PRINT("Sent:\t");
    PRINTLN(sd_data->sd_sent);
    PRINT("RPI:\t");
    PRINTLN(sd_data->sd_rpi);
    PRINT("Files:\t");
    PRINTLN(sd_data->sd_files);
}

/* ============ */
/* Print GPS    */
/* ============ */

void print_gps_data(gps_data_t *gps_data)
{
    PRINTLN("Func: print_gps_data()");

    memset(buffer_g, 0, BUFFER_SIZE_MAX);
    PRINT("Time:\t\t");
    sprintf(buffer_g, "%02d:%02d" /*:%02d"*/, gps_data->gps_date_time.gps_hour, gps_data->gps_date_time.gps_min /*, gps_data->gps_date_time.gps_sec*/);
    PRINTLN(buffer_g);

    PRINT("Date:\t\t");
    sprintf(buffer_g, "%02d/%02d" /*/%02d"*/, gps_data->gps_date_time.gps_day, gps_data->gps_date_time.gps_month /*, gps_data->gps_date_time.gps_year*/);
    PRINTLN(buffer_g);

    PRINT("Location:\t");
    sprintf(buffer_g, "St:%02d, Lt:%0.5f, Lg:%0.5f, At:%0.2f, Sp:%0.2f, He:%0.2f", gps_data->gps_sat, (float)gps_data->gps_lat, (float)gps_data->gps_lng, (float)gps_data->gps_alt, (float)gps_data->gps_speed, (float)gps_data->gps_course / CONST_100);
    PRINTLN(buffer_g);
}
/* ============== */
/* Print Battery  */
/* ============== */

void print_battery_stats(battery_data_t *battery_data)
{
    PRINTLN("Func: print_battery_stats()");

    PRINT("soc:\t\t");
    PRINT(battery_data->battery_soc);
    PRINTLN("%");

    PRINT("volts:\t\t");
    PRINT(battery_data->battery_volts);
    PRINTLN(" mV");

    PRINT("current:\t");
    PRINT(battery_data->battery_current);
    PRINTLN(" mA");

    PRINT("capacity:\t");
    PRINT(battery_data->battery_capacity);
    PRINTLN(" mAh");

    PRINT("fullCapacity:\t");
    PRINT(battery_data->battery_full_capacity);
    PRINTLN(" mAh");

    PRINT("power:\t\t");
    PRINT(battery_data->battery_power);
    PRINTLN(" mW");

    PRINT("health:\t\t");
    PRINT(battery_data->battery_health);
    PRINTLN("%");
}

/* ================= */
/* Print Sensor Data */
/* ================= */

void print_sensor_data(sns_data_t *sns_data)
{
    PRINTLN("Func: print_sensor_data()");

    // PRINT("Acc:\t\t");
    // PRINTLN((float)(sns_data->sns_mpu_acc) / CONST_100);
    // PRINT("Gyro:\t\t");
    // PRINTLN((float)(sns_data->sns_mpu_gyro) / CONST_100);
    // PRINT("Mag:\t\t");
    // PRINTLN((float)(sns_data->sns_mpu_mag) / CONST_100);

    PRINT("Temp:\t\t");
    PRINTLN((float)(sns_data->sns_bmp_temp_c) / CONST_100);
    // PRINT("Pres:\t\t");
    // PRINTLN(sns_data->sns_bmp_presure);
    // PRINT("Alt:\t\t");
    // PRINTLN(sns_data->sns_bmp_alt_m * CONST_10);

    PRINT("gx:\t\t");
    PRINTLN((float)(sns_data->sns_gx) / CONST_100);
    PRINT("gy:\t\t");
    PRINTLN((float)(sns_data->sns_gy) / CONST_100);
    PRINT("gz:\t\t");
    PRINTLN((float)(sns_data->sns_gz) / CONST_100);

    PRINT("mx:\t\t");
    PRINTLN((float)(sns_data->sns_mx) / CONST_100);
    PRINT("my:\t\t");
    PRINTLN((float)(sns_data->sns_my) / CONST_100);
    PRINT("mz:\t\t");
    PRINTLN((float)(sns_data->sns_mz) / CONST_100);

    PRINT("ntc1:\t\t");
    PRINTLN(sns_data->sns_ntc1);
    PRINT("ntc2:\t\t");
    PRINTLN(sns_data->sns_ntc2);
}

/* ============= */
/* Print Command */
/* ============= */

void print_command(command_data_t *command_data)
{
    PRINTLN("Func: print_command()");
    PRINTLN("***********************");
    PRINT("Code:\t0x");
    PRINTLN(command_data->cmd_code, HEX);
    PRINT("Type:\t0x");
    PRINTLN(command_data->cmd_type, HEX);
    PRINT("Size:\t");
    PRINTLN(command_data->cmd_size);
    PRINT("Chksum:\t");
    PRINTLN(command_data->cmd_chksum, HEX);

    if (command_data->cmd_size)
    {
        PRINTLN("Payload: ");
        print_buffer(command_data->cmd_payload, command_data->cmd_size);
    }
    PRINTLN("***********************");
    PRINTLN();
}

void print_ld_meta_data(ld_meta_data_t *ld_meta_data)
{
    PRINTLN("Func: print_ld_meta_data()");
    PRINTLN("***********************");
    PRINT("ld_type:\t");
    PRINTLN(ld_meta_data->ld_type);
    PRINT("ld_size:\t");
    PRINTLN(ld_meta_data->ld_size);
    PRINTLN("Payload:");
    print_buffer(ld_meta_data->packet + LD_META_HEADER_LENGTH, ld_meta_data->ld_size);
    PRINTLN("***********************");
    PRINTLN();
}
