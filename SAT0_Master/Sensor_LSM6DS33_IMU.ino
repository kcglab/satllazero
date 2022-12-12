/**
    @file Sensor_LSM6DS33_IMU.ino
    @brief IMU utils functions for SATLLA0.

    This file contains LSM6DS33 and LIS3MDL IMU functionality for the SATLLA0.

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

#if IMU_LSM6DS33_ENABLE

#define IMU_VALID_THRESHOLD 3
uint8_t imu_valid_read_counter = 0;

#include <Adafruit_LSM6DS33.h>
Adafruit_LSM6DS33 lsm6ds;

#include <Adafruit_LIS3MDL.h>
Adafruit_LIS3MDL lis3mdl;

sensors_event_t accel, gyro, mag, temp;

/* ============ */
/* FAT          */
/* ============ */

void imu_set_state(bool state)
{
    PRINTLN("Func: imu_set_state()");
    if (state)
    {
        imu_turn_on();
    }
    else
    {
        imu_turn_off();
    }
}

void imu_turn_on()
{
    PRINTLN("Func: imu_turn_on()");
    digitalWrite(FAT_IMU_PIN, HIGH);
    delay(TEN_MS);

    if (imu_is_sleep) {
        imu_resume();
    }

    imu_is_on = millis();
    imu_is_sleep = 0;
}

void imu_turn_off()
{
    PRINTLN("Func: imu_turn_off()");
    lsm6ds.setAccelDataRate(LSM6DS_RATE_SHUTDOWN);
    lis3mdl.setOperationMode(LIS3MDL_POWERDOWNMODE);

    digitalWrite(FAT_IMU_PIN, LOW);

    delay(TEN_MS);
    imu_is_on = 0;
    imu_is_sleep = 0;
}

void imu_sleep()
{
    PRINTLN("Func: imu_sleep()");

    lsm6ds.setAccelDataRate(LSM6DS_RATE_SHUTDOWN);
    lis3mdl.setOperationMode(LIS3MDL_POWERDOWNMODE);

    delay(TEN_MS);
    imu_is_sleep = 1;
}

void imu_resume()
{
    PRINTLN("Func: imu_resume()");
    imu_is_sleep = 0;

    delay(TEN_MS);
    sensor_imu_setup();
}

void reset_imu()
{
    PRINTLN("Func: reset_imu()");
    imu_turn_off();
    delay(100);
    imu_turn_on();
    delay(100);
    sensor_imu_setup();
}

/* ============ */
/* Setup        */
/* ============ */

void sensor_imu_setup()
{
    PRINTLN("Func: sensor_imu_setup()");
    bool lsm6ds_success, lis3mdl_success;
    
    lis3mdl_success = lis3mdl.begin_I2C();
    if (!lis3mdl_success)
    {
        PRINTLN("LIS3MDL: Init Failed!.\t");
    }
    else
    {
        PRINTLN("LIS3MDL: Setup Succeed.");
    }

    delay(TENTH_SEC);
    
    lsm6ds_success = lsm6ds.begin_I2C();

    /* Initialise the sensor */
    if (!lsm6ds_success)
    {
        PRINTLN("LSM6DS: Init Failed!.\t");
    }
    else
    {
        PRINTLN("LSM6DS: Setup Succeed.");
    }

    lsm6ds.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS);
    lsm6ds.setGyroDataRate(LSM6DS_RATE_12_5_HZ);

    lis3mdl.setDataRate(LIS3MDL_DATARATE_155_HZ);
    lis3mdl.setRange(LIS3MDL_RANGE_4_GAUSS);
    lis3mdl.setPerformanceMode(LIS3MDL_MEDIUMMODE);
    lis3mdl.setOperationMode(LIS3MDL_CONTINUOUSMODE);
}

void imu_read()
{
    PRINTLN("Func: imu_read()");

    /* Get a new normalized sensor event */
    lsm6ds.getEvent(&accel, &gyro, &temp);
    lis3mdl.getEvent(&mag);

    sns_data_g.sns_gx = gyro.gyro.x * CONST_100;
    sns_data_g.sns_gy = gyro.gyro.y * CONST_100;
    sns_data_g.sns_gz = gyro.gyro.z * CONST_100;

    sns_data_g.sns_mx = mag.magnetic.x * CONST_100;
    sns_data_g.sns_my = mag.magnetic.y * CONST_100;
    sns_data_g.sns_mz = mag.magnetic.z * CONST_100;

    sns_data_g.sns_bmp_temp_c = temp.temperature * CONST_100;
#ifdef PRINT_FUNC_DEBUG
    print_imu_raw_data();
#endif

// if no IMU for certain time, reset
    if (sns_data_g.sns_gx == 0.0 && sns_data_g.sns_gy == 0.0 && sns_data_g.sns_gz == 0.0)
    {
        imu_valid_read_counter++;
    }

    if (imu_valid_read_counter > IMU_VALID_THRESHOLD)
    {
        imu_valid_read_counter = 0.0;
        // No imu data received -> Reset imu
        reset_imu();
    }

}

void print_imu_raw_data()
{
    PRINTLN("Func: print_imu_raw_data()");

    /* Display the results (acceleration is measured in m/s^2) */
    Serial.print("\t\tAccel X: ");
    Serial.print(accel.acceleration.x, 4);
    Serial.print(" \tY: ");
    Serial.print(accel.acceleration.y, 4);
    Serial.print(" \tZ: ");
    Serial.print(accel.acceleration.z, 4);
    Serial.println(" \tm/s^2 ");

    /* Display the results (rotation is measured in rad/s) */
    Serial.print("\t\tGyro  X: ");
    Serial.print(gyro.gyro.x, 4);
    Serial.print(" \tY: ");
    Serial.print(gyro.gyro.y, 4);
    Serial.print(" \tZ: ");
    Serial.print(gyro.gyro.z, 4);
    Serial.println(" \tradians/s ");

    /* Display the results (magnetic field is measured in uTesla) */
    Serial.print(" \t\tMag   X: ");
    Serial.print(mag.magnetic.x, 4);
    Serial.print(" \tY: ");
    Serial.print(mag.magnetic.y, 4);
    Serial.print(" \tZ: ");
    Serial.print(mag.magnetic.z, 4);
    Serial.println(" \tuTesla ");

    Serial.print("\t\tTemp   :\t\t\t\t\t");
    Serial.print(temp.temperature);
    Serial.println(" \tdeg C");
    Serial.println();
    delay(1000);
}

#endif
