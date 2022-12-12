# RPI Commands Code and Examples
- Commands are sent in Hex
- Structure: <SATID><CMD><PARAMS>

## CMD_RPI_1_COMMAND 0x55       // RPI Command
## CMD_RPI_1_COMMAND_X_MNT 0x56 // RPI turn on in X minutes command
- Sending command to RPI is either 0x55 or 0x56 if delay is required.

## CMD_POWER_ON 0x03 (Power On)
- RPI power on.
- Params: N/A
- B45503
- 
## CMD_TAKE_PHOTO 0x04
- RPI Take photo - old method.
- Params: Width: (100), Mode: (1) = Day, 2 = Night,
- B45504

## CMD_SIT 0x05 (Satellite Image Transfer)
- RPI Take photo - new method.
- Params: N/A
- B45505

## CMD_SEEK_LASER 0x06 (Satellite Image Transfer)
- RPI Take photo - new method.
- Params: Freq: (2.0), Threshold: (1.0), Timeout: (45) secs
- B45506

## CMD_RWCS 0x07 (Reaction Wheel Control System)
- RPI Attitude Determination and Control System (ADCS)
- Params: desiredAngleX, Y, Z: (0), Timeout: (1) min
- B45507

## CMD_POWER_OFF 0x08 (Power Off)
- RPI power off.
- Params: N/A
- B45508

## CMD_DROP_OUTBOX 0x09 (Clear Outbox folder)
- RPI Clear Outbox folder.
- Params: N/A
- B45509

## CMD_ADSB 0x0E ()
- RPI ADS-B Exchange
- Params: N/A
- B4550E

## CMD_NEW_TAKE_PHOTO 0x0F ()
- RPI Take Photo - new experimental was
- Params: missionType: (0), quality: (0), iso: (0)(2b), shutter (0)(2b), width: (0)(2b), height: (0)(2b) 
- B4550F

## DataTypes:
- OTHER = 0  # 0x00
- PHOTO = 1  # 0x01
- META = 20  # 0x14
- STARS = 21  # 0x15
- TEXT = 22  # 0x16
- DATA = 23  # 0x17
- STD_OUT = 24 # 0x18
- STD_ERR = 25 # 0x19
- LASER_TEXT = 26 # 0x1A
- LASER_VIDEO = 27 # 0x1B