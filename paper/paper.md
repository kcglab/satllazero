---
title: 'SATLLA0: A Flight Software Platform for Aerospace and STEM Education'
tags:
  - Python
  - Arduino
  - CubeSat
  - PicoSat
authors:
  - name: Rony Ronen
    orcid: 0000-0002-1300-5236
    affiliation: "1"
  - name: Boaz Ben Moshe
    orcid: 00000-0002-1580-5421
    affiliation: "1"
affiliations:
 - name: Computer Science Departmenet, Ariel University, 47100, Israel
   index: 1
date: 19 Decemner 2022
bibliography: paper.bib
---

# Summary
Nanosatellites are becoming a preferred platform for testing innovative technologies and conducting academic research in space. Flight Software (FSW) is software that runs on a processor embedded in the satellite body. The software is responsible for managing satellite activity, data processing, and satellite health and safety, thus enabling the satellite to perform all actions necessary to achieve its scientific objective. 

SATLLA-0 is an open source platform for building and experimenting with laboratory nanosatellites for academics and school students. The platform is based on the flight software of the SATLLA-2B satellite, developed and built by the Nanosatellite Research Laboratory of Ariel University. The SATLLA-2B satellite was successfully launched on January 13, 2022 aboard a Falcon 9 launch vehicle.

SATLLA-0 is divided into three main libraries, with the main library, SAT0_Master, responsible for the ongoing operation of the satellite, the SAT0_OBC library responsible for the operation of the satellite's research objective, and the SAT0_Ground library responsible for transmitting telecommand to the satellite and receiving satellite transmissions and routing the data for further analysis. 

# Statement of need

Building a nanosatellite and developing flight software is challenging for newcomers to the space field due to the lack of literature and high cost. There are open-source frameworks for space projects developed by space agencies, universities, or commercial companies to serve as reference designs and encourage code reuse in their future projects. These frameworks have not yet gained significant adoption outside of their host organizations.

At the same time, using an existing software framework leads to a shorter development program at lower costs and better quality. These effects are beneficial for organizations such as universities or schools that want to enter the space field. These reasons have led us to release the SATLLA-2B flight software as open-source code with additional documents to build a nanosatellite.

# Description

The SATLLA-0 core flight system is an open-source flight software used by the SATLLA-2B satellite. The library was designed as a starting point for academic institutions or schools that want to build or experiment with a laboratory or functional nanosatellite. As mentioned earlier, the system is divided into three parts. The main library contains the satellite's flight software, written in Arduino. Arduino is a C\C++-based programming language that is open source and easy to learn. The main library developed for the Teensy 3.x/4.x microcontroller family, which is the main microprocessor unit (MPU) of the SATLLA-2B nanosatellite. It is possible to configure the library to run on other microcontrollers using the settings available in the library.

![](https://github.com/kcglab/satllazero/blob/main/paper/figure1_1.pdf).
![](https://raw.githubusercontent.com/adrena-lab/Optimising-Light-Source-Positioning/Code/Figures/Figure1-1.png)
The FSW, as derived from the system requirements of a state machine, contains two main states:
Initialization (as shown in Fig. 1): the FSW performs the initial configuration process using the initialization parameters stored in the MCU's flash memory. At first startup or after a complete reset, the FSW uses previously defined default values. In these states, the various modules of the satellite are also initialized, e.g. IMU, GPS and communications. At the end of this state, the health of the satellite is checked to determine the mode of operation: Panic, Reduced Operation, or Normal Operation.

Operation: at the end of the initialization state and once the operating mode is established, the FSW enters successive loops that allow it to sample the various modules and sensors and respond accordingly to the values received. It also sends a heartbeat to the watchdog at regular intervals to prevent it from reaching zero

![](https://github.com/kcglab/satllazero/blob/main/paper/figure2_1.pdf)
An overview of the SATLLA-2 power, data, and radio frequency interfaces (RF) is shown in Fig. 2. In general, the interface to the various components is determined by the type of communications incorporated in each component. However, some components contain more than one communication interface, such as Integrated Communications (I$^2$C), Serial Peripheral Interface (SPI), or Universal Asynchronous Receiver Transmitter (UART). For these components, we preferred to use the type of communication as needed. For example, communication with the OBC is via a UART interface, as well as communication with the GPS. In this way, the MCU remains in operation and collects data as needed. The power interface provides regulated voltage levels (3.3 v) for the avionics and payloads of the satellite.


For a full documentation of `SATLLA-0`, the reader is referred to our [GitHub page](https://github.com/kcglab/satllazero).

# Acknowledgements

The authors acknowledge the great work and dedication of the entire SATLLA team and the support of the Department of Computer Science and Mathematics at Ariel University. Special thanks to the students who participated in the mission design for the nanosatellite: Michael Britvin, Zachi Ben-Shitrit, Assaf Chia, Shaya Sonnenberg, Shai Aharon, Michael Twito, Revital Marble, and Aharon Got. This work has been partially supported by the Ariel Cyber Innovation Center.

# References
