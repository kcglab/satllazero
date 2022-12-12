# SATLLA-4
K&CG Satellite Project 2022
This project is about K&CG SATTLA-4 pico satellite, version of 2022

## Arduino Libraries
- https://github.com/LoRaTracker/SX1280
- https://github.com/HelTecAutomation/Heltec_ESP32/tree/master/src/lora
- https://github.com/mikalhart/TinyGPSPlus/releases
- https://github.com/wizard97/ArduinoRingBuffer
- https://github.com/greiman/SdFat

## VS Project
The Arduino coding is using VS Code. It contains 3 main sub-folders.

## SAT4_Master
This contains the SAT master logic.

## Computer Mission
- https://github.com/ifryed/SatImageTransfer
- https://github.com/ifryed/SatAlign

**SAT4_Master.ino**

## RPI Autostart
sudo nano /lib/systemd/system/sat.service


    [Unit]
    Description=SAT
    After=multi-user.target

    [Service]
    Type=idle
    #ExecStart=/usr/bin/python3 -u /home/pi/git/satlla1/Pi/Main.py
    ExecStart=sudo /bin/bash -c '/usr/bin/python3 -u /home/pi/git/satlla1/Pi/Main.py > /home/pi/git/satlla1/Pi/output.log 2>&1'
    WorkingDirectory=/home/pi/git/satlla1/Pi/
    #StandardOutput=inherit
    #StandardError=inherit
    Restart=always
    User=pi

    [Install]
    WantedBy=multi-user.target


sudo systemctl daemon-reload

sudo systemctl enable sat.service
