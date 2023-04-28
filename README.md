[![DOI](https://zenodo.org/badge/575807899.svg)](https://zenodo.org/badge/latestdoi/575807899)
[![status](https://joss.theoj.org/papers/af50648087de6186bd6eb99014658ada/status.svg)](https://joss.theoj.org/papers/af50648087de6186bd6eb99014658ada)

# Overview
**SATLLA-0** (also known as satllazero) is an open source project dedicated to the development of a complete pico-satellite.


# Our Vision
The overall vision is to enable both researchers and 12K to build a fully functional pico-satellite model based on a proven design that is fully functional in space (see [SATLLA-2B](https://tinygs.com/satellite/SATLLA-2B)).
The SATLLA-0 project includes both the software and hardware of the pico satellite (and ground station).
The vision of the project is to enable any science class (in high school or university) to experience the "new space" at a fraction of the price of existing solutions.


# Repository Structure
The repo is structured as follows:
+ software: The flight software is divided into three sub-folders:
  + SAT0_Ground: The ground station module written in Arduino [SAT0_Ground](/software/SAT0_Ground).
  + SAT0_Master: The satellite main computer module written in Arduino [SAT0_Master](/software/SAT0_Master)
  + SAT0_OBC: The on-board computer (OBC) module written in Python [SAT0_OBC](/software/SAT0_OBC).
+ hardware: Contains the hardware schematics of the satellite.
+ MD: Contains MD files describing the satellite commands, beacons and bill of materials (BOM).
+ stl: Contains 3D sketches.
+ paper: Contains the JOSS submission paper.


# Description
The SATLLA-0 core flight system is an open-source flight software used by the SATLLA-2B satellite. The library was designed as a starting point for academic institutions or schools that want to build or experiment with a laboratory or functional nanosatellite. As mentioned earlier, the system is divided into three parts. The main library contains the satellite's flight software, written in Arduino. Arduino is a C/C++ based programming language that is open source and easy to learn. The main library developed for the Teensy 3.x/4.x microcontroller family, which is the main microprocessor unit (MPU) of the SATLLA-2B nanosatellite. However, the library can be compiled for other microcontrollers using the definitions available in the library.


# Features
The SATLLA-0 flight software platform includes the following key features:
+ A functional nanosatellite based on a Teansy microcontroller which supports the following subsystems: 
  + 2P structure.
  + Electrical Power System (EPS).
  + Telecommand and communications based on LoRa UHF and S-band. 
  + GPS.
  + Inertial Measurement Unit (IMU).
  + Thermistors (Temperature).
  + High-power array LED.
+ An on-board computer (OBC) for research activities based on a RaspberryPi Zerro.
  + Linux-based 
  + Integrated camera.
  + Variety of payloads including Attitude Determination and Control System (ADCS) based on reaction wheels, an Automatic Dependent Surveillance-Broadcast (ADS-B) receiver for surveillance aircraft.
+ A functional ground station based on an ESP32 board.


## Supported Hardware
A list of all the hardware required to assemble a functional nanosatellite is available [here](https://github.com/kcglab/satllazero/blob/main/MD/bom.MD).
The following boards are being used in this repository:
+ SAT0_Ground: WiFi LoRa ESP32 from Heltec or TTGO.
+ SAT0_Master: Teensy 3.6/4.1 microcontroller.
+ SAT0_OBC: RPI-Zero or RPI-Zero-W.


# Installation
SATLLA-0 is written in Arduino, and can be installed via Arduino, Teensyduino or any other IDE supporting Arduino.
Please refer to [wiki](https://github.com/kcglab/satllazero/wiki) for details installation.


# For more info
For more into on SATLLA-0, its workings, inputs, outputs and more see the [wiki](https://github.com/kcglab/satllazero/wiki).


# Bug reporting and feature requests
Please submit bug reports and feature requests to the issue tracker on GitHub: [SATLLA-0 issue tracker](https://github.com/kcglab/satllazero/issues)


# Licence
This program is released as open source software under the terms of [GPL3 License](https://github.com/kcglab/satllazero/blob/main/LICENSE).


# Links
1. A great place to learn more on SATLLA project: [SATLLA Channel](https://www.youtube.com/watch?v=bJ7NgBDLjMQ)
2. SATLLA 2 DIY KIT Assembly [iFixIt Build Instructions](https://www.ifixit.com/Guide/SATLLA+2+DIY+KIT+Assembly/147004)
3. A good starting point is to use a Weather Balloons: [Extreme Long-RAnge Wi-Fi](https://www.youtube.com/watch?v=0xc7XjHUJkM&t=41s)

#Citation
If you'd like to cite us in a project or publication, please include a reference to the JOSS paper:
*** Pre Review ***


Ariel University. 2023.


# Troubleshoot
* Issue w/Actions