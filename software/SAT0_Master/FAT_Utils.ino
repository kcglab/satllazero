/**
    @file FAT_Utils.ino
    @brief FSAT Settings and Consoling function for SATLLA0.

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

void setup_fat_pin_out()
{
    PRINTLN("Func: setup_fat_pin_out()");

    pinMode(FAT_IMU_PIN, OUTPUT);
    pinMode(FAT_GPS_PIN, OUTPUT);
    pinMode(FAT_LORA_433_PIN, OUTPUT);
    pinMode(FAT_RPI_1_PIN, OUTPUT);
    pinMode(FAT_LORA_24_PIN, OUTPUT);
    pinMode(FAT_LASER_PIN, OUTPUT);
    pinMode(FAT_MAIN_LED_PIN, OUTPUT);
    pinMode(FAT_ANT_PIN, OUTPUT);
    pinMode(FAT_GEN2_PIN, OUTPUT);

    digitalWrite(FAT_IMU_PIN, LOW);
    digitalWrite(FAT_GPS_PIN, LOW);
    digitalWrite(FAT_LORA_433_PIN, LOW);
    digitalWrite(FAT_RPI_1_PIN, LOW);
    digitalWrite(FAT_LORA_24_PIN, LOW);
    digitalWrite(FAT_LASER_PIN, LOW);
    digitalWrite(FAT_MAIN_LED_PIN, LOW);
    digitalWrite(FAT_ANT_PIN, LOW);
    digitalWrite(FAT_GEN2_PIN, LOW);
}

void fat_close_all()
{
    PRINTLN("Func: fat_close_all()");

    digitalWrite(FAT_IMU_PIN, LOW);
    digitalWrite(FAT_GPS_PIN, LOW);
    digitalWrite(FAT_LORA_433_PIN, LOW);
    digitalWrite(FAT_RPI_1_PIN, LOW);
    digitalWrite(FAT_LORA_24_PIN, LOW);
    digitalWrite(FAT_LASER_PIN, LOW);
    digitalWrite(FAT_MAIN_LED_PIN, LOW);
    digitalWrite(FAT_ANT_PIN, LOW);
    digitalWrite(FAT_GEN2_PIN, LOW);
}

void fat_set_state(fat_module module, bool state)
{
    switch (module)
    {
    case fat_imu:
#if IMU_ENABLE
        imu_set_state(state);
#endif
        break;

    case fat_gps:
#if GPS_ENABLE
        gps_set_state(state);
#endif
        break;

    case fat_433:
#if RF_433_ENABLE
        lora_433_set_state(state);
#endif
        break;

    case fat_rpi:
#if RPI_ENABLE
        rpi1_set_state(state);
#endif
        break;

    case fat_24:
#if RF_24_ENABLE
        lora_24_set_state(state);
#endif
        break;

    case fat_none:
    default:
        break;
    }
}