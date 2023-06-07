/**
    @file Init_Utils.ino
    @brief Data Struct and enums for SATLLA0.

    This file contains initial sequence function for the SATLLA0.

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

/* ============ */
/* Initial Boot */
/* ============ */
#define INITIAL_BOOT_EEPROM_ADRS 0x00
#define INITIAL_BOOT_EEPROM_FLAG 0x01

void initial_boot()
{
    PRINTLN("Func: initial_boot()");

    // Define pin #26 as input and activate the internal pull-up resistor
    pinMode(ANT_SW_PIN, INPUT_PULLUP);
    // Pressed: 1, Released: 0
    PRINT("Ant Button State:");
    PRINTLN(digitalRead(ANT_SW_PIN));

    if (!is_initial_boot_completed())
    {
        // turn off devices.
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

#if RF_24_ENABLE
        if (lora_24_is_on)
        {
            lora_24_turn_off();
        }
#endif

        // first boot. wait 4 min.
        delay(SEC_1);
        unsigned long start = millis();
        int timer = 0;
        // unsigned long last = millis();
        PRINT("Func: Initial Boot Delay For:\t");
        PRINTLN(INITIAL_BOOT_TIME / SEC_1);
        while (millis() - start < INITIAL_BOOT_TIME && start)
        {
            if (sat_state == sat_panic_power)
            {
                modeled_blink_count = 3;
            }
            else if (sat_state == sat_med_power)
            {
                modeled_blink_count = 2;
            }
            else
            {
                modeled_blink_count = 1;
            }

            delay(HALF_SEC);
            timer++;
            if (timer > 60)
            {
                timer = 0;
                // watchdog
#if WD_ENABLE
                wd_heartbeat();
#endif

                // update sat_state
#if BBSTR_BQ27441_ENABLE
                battery_read();
                check_battery_status();
#endif
            }
        }

        // watchdog
#if WD_ENABLE
        wd_heartbeat();
#endif

        if (millis() - start >= INITIAL_BOOT_TIME)
        {
            initial_boot_completed();
            delay(TEN_MS); // let eeprom complete
        }

#if GPS_ENABLE
        if (!gps_is_on)
        {
            gps_turn_on();
            delay(TEN_MS);
        }
#endif

#if IMU_ENABLE
        if (!imu_is_on)
        {
            imu_turn_on();
            delay(TEN_MS);
        }
#endif
        // Save first beacon before opening antennas
        handle_beacon(beacon_type_long, true);

        // Deploy antennas
        bool state = deploy_antenna(ANTENNA_DEPLOY_PERIOD); // default

        // if didn't opened, try X 2.
        if (!state)
        {
            state = deploy_antenna(ANTENNA_DEPLOY_PERIOD * 2);
        }

        // if still didn't opened, try X 3.
        if (!state)
        {
            state = deploy_antenna(ANTENNA_DEPLOY_PERIOD * 3);
        }
    }

    // initial boot completed
    modeled_blink_count = 5;

    delay(SECS_2);

#if BBSTR_BQ27441_ENABLE
    battery_read();
    check_battery_status();
#endif

    // set_sat_health(); // only from main loop
    if (sat_state > sat_panic_power)
    {
#if RF_433_ENABLE
        // turn on radio 433
        if (!lora_433_is_on)
        {
            lora_433_turn_on();
            delay(TEN_MS);
            lora_433_setup();
            delay(TEN_MS);
        }
#endif
        // Each restart, send I'm alive message
        i_am_alive_message();
    }
}

/* ============== */
/* Deploy Antenna */
/* ============== */

bool deploy_antenna(uint8_t secs)
{
    PRINTLN("Func: deploy_antenna()");
    bool deploy_antenna_flag = false;

    // read the state of the pushbutton value:
    int buttonState = digitalRead(ANT_SW_PIN);
    PRINT("Ant Button State:");
    PRINTLN(buttonState);

#if WD_ENABLE
    wd_heartbeat();
#endif

    unsigned long deploy_period = SEC_1 * ANTENNA_DEPLOY_PERIOD;

    // if buttonState is HIGH, then it is still connected.
    if (buttonState == HIGH) // Pressed
    {
        PRINTLN(F("Ant-Sw is ON"));

        unsigned long start = millis();
        digitalWrite(FAT_ANT_PIN, HIGH);
        while (digitalRead(ANT_SW_PIN) == HIGH || millis() - start < deploy_period)
        {
            // PRINT(F("."));
        }
        digitalWrite(FAT_ANT_PIN, LOW);
        deploy_antenna_flag = !digitalRead(ANT_SW_PIN);
    }
    else
    {
        PRINTLN(F("Ant-Sw is OFF"));

        digitalWrite(FAT_ANT_PIN, HIGH);
        delay(deploy_period);
        digitalWrite(FAT_ANT_PIN, LOW);

        deploy_antenna_flag = true;
    }

    return deploy_antenna_flag;
}

void i_am_alive_message()
{
    PRINTLN("Func: i_am_alive_message()");
    // lora_data_t iamalive;
    // memset(lora_data_g.packet, 0, LORA_PACKET_LENGTH);
    lora_data_g.local_address = local_address;
    lora_data_g.destination = destination;
    lora_data_g.msg_type = type_text;

    lora_data_g.msg_index = tx_counter;
    lora_data_g.msg_time = millis() / SEC_1;
    lora_data_g.msg_ack_req = 0;

    uint8_t len = sprintf(buffer_g, "%s: %s", SAT_NAME, "I'M ALIVE!");
    memcpy(lora_data_g.msg_payload, buffer_g, len);
    lora_data_g.msg_size = len;
#if RF_433_ENABLE
    delay(SEC_1);
    send_message_lora_433((uint8_t *)lora_data_g.packet, LORA_HEADER_LENGTH + lora_data_g.msg_size);
#endif
    
#if RF_24_ENABLE
    delay(SEC_1);
    send_message_lora_24((uint8_t *)lora_data_g.packet, LORA_HEADER_LENGTH + lora_data_g.msg_size);
#endif
}

void initial_boot_completed()
{
    PRINTLN("Func: initial_boot_completed()");
#if EEPROM_ENABLE
    eeprom_system_update(INITIAL_BOOT_EEPROM_ADRS, INITIAL_BOOT_EEPROM_FLAG);
#endif
}

bool is_initial_boot_completed()
{
    PRINTLN("Func: is_initial_boot_completed()");
    uint8_t value = INITIAL_BOOT_EEPROM_FLAG;
#if EEPROM_ENABLE
    value = eeprom_system_read(INITIAL_BOOT_EEPROM_ADRS);
#endif
    return (value == INITIAL_BOOT_EEPROM_FLAG);
}
