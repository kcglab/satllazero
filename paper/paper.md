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
  - name: Michael Britvin
    affiliation: "2"
  - name: Boaz Ben Moshe
    orcid: 00000-0002-1580-5421
    affiliation: "1"
affiliations:
  - name: School of Computer Science, Ariel University, 47100, Israel
    index: 1
  - name: Faculty of Engineering, Ariel University, 47100, Israel
    index: 2
date: 19 December 2022
bibliography: paper.bib
---
# Summary
Nanosatellites are becoming a preferred platform for testing innovative technologies and conducting academic research in space. Flight Software (FSW) is software that runs on a processor embedded in the satellite body [@miranda2019comparative]. The software is responsible for managing satellite activity, data processing, and satellite health and safety, thus enabling the satellite to perform all actions necessary to achieve its scientific objective.

SATLLA-0 is an open source platform for building and experimenting with laboratory nanosatellites for academics and school students. The platform is based on the flight software of the SATLLA-2B satellite, developed and built by the Nanosatellite Research Laboratory of Ariel University. The SATLLA-2B satellite was successfully launched on January 13, 2022 aboard a Falcon 9 launch vehicle.

SATLLA-0 is divided into three main libraries, with the main library, SAT0_Master, responsible for the ongoing operation of the satellite, the SAT0_OBC library responsible for the operation of the satellite\'s research objective, and the SAT0_Ground library responsible for transmitting telecommand to the satellite and receiving satellite transmissions and routing the data for further analysis.

# Statement of need
Building a nanosatellite and developing flight software is challenging for newcomers to the space field due to the lack of literature and high cost. There are open-source frameworks for space projects developed by space agencies, universities, or commercial companies to serve as reference designs and encourage code reuse in their future projects [@jamie2017basic]. These frameworks have not yet gained significant adoption outside their host organizations.

At the same time, using an existing software framework leads to a shorter development program at lower costs and better quality. These effects are beneficial for organizations such as universities or schools that want to enter the space field. These reasons have led us to release the SATLLA-2B flight software as open-source code with additional documents to build a nanosatellite.

# Description
The SATLLA-0 core flight system is an open-source flight software used by the SATLLA-2B satellite. The library was designed as a starting point for academic institutions or schools that want to build or experiment with a laboratory or functional nanosatellite. As mentioned earlier, the system is divided into three parts. The main library contains the satellite\'s flight software, written in Arduino. Arduino is a C\C