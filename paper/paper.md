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
  equal-contrib: true
  affiliation: 1
- name: Boaz Ben Moshe
  orcid: 0000-0002-1580-5421
  equal-contrib: true 
  affiliation: 1
  affiliations:
- name: Ariel University, Israel
  index: 1
  date: 19 Dec 2022
  bibliography: paper.bib
___

# Summary
Nanosatellites are becoming a preferred platform for testing innovative technologies and conducting academic research in space. Flight Software (FSW) is software that runs on a processor embedded in the satellite body. The software is responsible for managing satellite activity, data processing, and satellite health and safety, thus enabling the satellite to perform all actions necessary to achieve its scientific objective. 

SATLLA-0 is an open source platform for building and experimenting with laboratory nanosatellites for academics and school students. The platform is based on the flight software of the SATLLA-2B satellite, developed and built by the Nanosatellite Research Laboratory of Ariel University. The SATLLA-2B satellite was successfully launched on January 13, 2022 aboard a Falcon 9 launch vehicle.

SATLLA-0 is divided into three main libraries, with the main library, Master, responsible for the ongoing operation of the satellite, the OBC library responsible for the operation of the satellite's research objective, and the Ground Station library responsible for transmitting telecommand to the satellite and receiving satellite transmissions and routing the data for further analysis. 

# Statement of need

Building a nanosatellite and developing flight software is challenging for newcomers to the space field due to the lack of literature and high cost. There are open-source frameworks for space projects developed by space agencies, universities, or commercial companies to serve as reference designs and encourage code reuse in their future projects. These frameworks have not yet gained significant adoption outside of their host organizations.

At the same time, using an existing software framework leads to a shorter development program with lower costs and better quality. These effects benefit space organizations, which led us to release the SATLLA-2B flight software as open source.

# Description


# Figures


# Acknowledgements

The authors acknowledge the great work and dedication of the entire SATLLA team and the support of the Department of Computer Science and Mathematics at Ariel University. Special thanks to the students who participated in the mission design for the nanosatellite: Michael Britvin, Zachi Ben-Shitrit, Assaf Chia, Shaya Sonnenberg, Shai Aharon, Michael Twito, Revital Marble, and Aharon Got. This work has been partially supported by the Ariel Cyber Innovation Center.

# References