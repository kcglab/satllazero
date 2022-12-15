# SATLLA0 Code

## 1. Setup
- 1.1 RTC
- 1.2 SPI Pin: 13 or 14 (Based on Teensy)
- 1.3 Wire
- 1.4 Watchdog
- 1.5 SD Card
- 1.6 Flash FS
- 1.7 Mode Led
- 1.8 Main LED
- 1.9 Laser Setup
- 1.10 EEPROM
- 1.11 IMU (BNO or LSM)
- 1.12 LoRa 433
- 1.13 LoRa 2.4
- 1.14 GPS
- 1.15 NTC (1 and 2)
- 1.16 Babysitter (BQ27441)
- 1.17 Initial Boot (Antenna Deploy)
- 1.18 Increase Reset index

## 2. Main Loop
- 2.1 WD Heartbeat
- 2.2 Read Babysitter Func
- 2.3 Check Battery Status Func 
- 2.4 Set satellite mode Func (Based on battery status)
- 2.5 If state is higher than panic, then:
- 2.5.1 If 1 minute passed from last beacon, then:
- 2.5.1.1 Set Short/Long beacon (Based on battery status and at least 5 long every hour)
- 2.5.1.2 Prepare Beacon parameters Func
- 2.5.1.3 Send Beacon using LoRa 433
- 2.5.1.4 Wait one second
- 2.5.1.5 Send Beacon using LoRa 2.4 (If loRa 2.4 is On - Depends on Satellite health)
- 2.6 Smart wait for 20 seconds (Listen to commands)
- 2.7 Handle Tasks Func (Auto and based on commands received)
- 2.8 If not in mission
- 2.8.1 If GFSK is allowed and state is higher than panic, then:
- 2.8.1.1 Switch modem to GFSK 433
- 2.8.1.2 Send beacon using GFSK 433
- 2.8.1.3 Switch back to LoRa 433
- 2.8.2 If in panic mode, then
- 2.8.2.1 Sleep for 45 seconds
- 2.8.2.2 Else, Sleep for at most 15 seconds (Depends on last beacon sent time)

## 3. Smart Delay Func
- 3.1 Loop for period
- 3.1.1 If GPS is on, feed GPS
- 3.1.2 If received 433, break
- 3.1.3 If received 2.4, break
- 3.1.4 WD Heartbeat
- 3.2 If break, handle message (2.4 or 433)
- 3.3 If restart requested, wait 2 seconds and restart.

## 4. Sleep Delay Func
- 4.1 Loop for period
- 4.1.1 If IMU is On and Not Sleep, Sleep
- 4.1.2 If LoRa 433 is On and Not Sleep, Sleep
- 4.1.3 If Lora 2.4 is On and Not Sleep, Sleep
- 4.1.4 If GPS is On and Not Sleep, Sleep
- 4.1.5 WD Heartbeat
- 4.2.1 If IMU is On and Sleep, Resume
- 4.2.2 If LoRa 433 is On and Sleep, Resume
- 4.2.3 If Lora 2.4 is On and Sleep, Resume
- 4.2.3 If GPS is On and Sleep, Resume
- 4.1.6 WD Heartbeat

## 5. Satellite Health Func
- 5.1 Switch Satellite State
- 5.1.2 Case: Panic power
- 5.1.2.1 If GPS is On, then turn Off
- 5.1.2.2 If IMU is On, then turn Off
- 5.1.2.4 If LoRa 433 is On, then turn Off
- 5.1.2.5 If LoRa 24 is On, then turn Off
- 5.1.3 Case: Medium power
- 5.1.3.1 If GPS is On, then turn Off
- 5.1.3.2 If IMU is Off, then turn On
- 5.1.3.4 If LoRa 433 is Off, then turn On
- 5.1.3.5 If LoRa 24 is Off, then turn On
- 5.1.3 Case: High power
- 5.1.3.1 If GPS is Off, then turn On
- 5.1.3.2 If IMU is Off, then turn On
- 5.1.3.4 If LoRa 433 is Off, then turn On
- 5.1.3.5 If LoRa 24 is Off, then turn On

## 6. Switch Beacon Type Func
- 6.1 If Satellite state > Medium power, then
- 6.1.2 Use Long Beacon
- 6.1.3 Else, Use Short Beacon
- 6.1.4 In addition, If last sent long beacon > 1 hour, then 
- 6.1.4.1 Use Long Beacon for the next 5 messages

## 7. Handle Beacon Func:
- 7.1 Read Babysitter
- 7.2 Read IMU
- 7.3 Read NTC
- 7.4 Read GPS
- 7.5 If SD, then list files
- 7.6 If SD error or SD overwrite, then list files from FS
- 7.7 Add Infodata to beacon
- 7.8 Add Metadata to beacon 
- 7.9 Prepare Long/Short beacon

## 8. Send Beacon Func:
- 8.1 If Stop Flag raised, return
- 8.2 Send Short/Long beacon via LoRa 433/24
=======
1.1 RTC
1.2 SPI Pin: 13 or 14 (Based on Teensy)
1.3 Wire
1.4 Watchdog
1.5 SD Card
1.6 Flash FS
1.7 Mode Led
1.8 Main LED
1.9 Laser Setup
1.10 EEPROM
1.11 IMU (BNO or LSM)
1.12 LoRa 433
1.13 LoRa 2.4
1.14 GPS
1.15 NTC (1 and 2)
1.16 Babysitter (BQ27441)
1.17 Initial Boot (Antenna Deploy)
1.18 Increase Reset index

## 2. Main Loop
2.1 WD Heartbeat
2.2 Read Babysitter Func
2.3 Check Battery Status Func 
2.4 Set satellite mode Func (Based on battery status)
2.5 If state is higher than panic, then:
2.5.1 If 1 minute passed from last beacon, then:
2.5.1.1 Set Short/Long beacon (Based on battery status and at least 5 long every hour)
2.5.1.2 Prepare Beacon parameters Func
2.5.1.3 Send Beacon using LoRa 433
2.5.1.4 Wait one second
2.5.1.5 Send Beacon using LoRa 2.4 (If loRa 2.4 is On - Depends on Satellite health)
2.6 Smart wait for 20 seconds (Listen to commands)
2.7 Handle Tasks Func (Auto and based on commands received)
2.8 If not in mission
2.8.1 If GFSK is allowed and state is higher than panic, then:
2.8.1.1 Switch modem to GFSK 433
2.8.1.2 Send beacon using GFSK 433
2.8.1.3 Switch back to LoRa 433
2.8.2 If in panic mode, then
2.8.2.1 Sleep for 45 seconds
2.8.2.2 Else, Sleep for at most 15 seconds (Depends on last beacon sent time)

## 3. Smart Delay Func
3.1 Loop for period
3.1.1 If GPS is on, feed GPS
3.1.2 If received 433, break
3.1.3 If received 2.4, break
3.1.4 WD Heartbeat
3.2 If broken, handle message (2.4 or 433)
3.3 If restart requested, wait 2 seconds and restart.

## 4. Sleep Delay Func
4.1 Loop for period
4.1.1 If IMU is On, Sleep
4.1.2 If LoRa 433 is On, Sleep
4.1.3 If Lora 2.4 is On, Sleep
4.1.4 If GPS is On, Sleep
4.1.5 WD Heartbeat

