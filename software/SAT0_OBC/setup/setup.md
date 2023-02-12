# OBC Setup

1. enable serial0
    * `sudo nano /boot/cmdline.txt`
    * remove the line: "console=serial0,115200"
    * `sudo reboot`
2. Update config.txt file with the contents of the config file in this directory
    * `sudo nano /boot/config.txt`
    * rewrite contents
3. Expand file system
    * `sudo raspi-config`
    * Advanced options -> Expand Filesystem 
4. Increase swap
    * `sudo dphys-swapfile swapoff`
    * `sudo nano /etc/dphys-swapfile`
    * modify the variable to be CONF_SWAPSIZE=1024
    * `sudo dphys-swapfile setup`
    * `sudo dphys-swapfile swapon`

5. Prepare git
    * `sudo apt update`
    * `sudo apt-get update`
    * `sudo apt upgrade`
    * `sudo apt install git`

6. Prepare python and pip
    * `sudo apt install python3-pip`
    * `python3 -m pip install --upgrade pip`
    * `sudo apt-get install libatlas-base-dev`
    * `sudo apt install python3-picamera`

7. Required for 'convert'
    * `sudo apt-get install imagemagick`

8. Required for 'pillow'
    * `sudo apt install libjpeg-dev`

9. Required for 'opencv'
    * `sudo apt install cmake build-essential pkg-config git`
    * `sudo apt install libjpeg-dev libtiff-dev libjasper-dev libpng-dev libwebp-dev libopenexr-dev`
    * `sudo apt install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev libxvidcore-dev libx264-dev libdc1394-22-dev libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev`
    * `sudo apt install libgtk-3-dev libqt5gui5 libqt5webkit5 libqt5test5 python3-pyqt5`
    * `sudo apt install libatlas-base-dev liblapacke-dev gfortran`
    * `sudo apt install libhdf5-dev libhdf5-103`
    * `sudo apt install python3-dev python3-pip python3-numpy`
    * `python3 -m pip install opencv-python==4.5.3.56`

10. Clone the SATLLAZero repository
    * `git clone https://github.com/kcglab/satllazero`

11. Install the requirements file
    * ` sudo pip install -r satllazero/SAT0_OBC/setup/requirements.txt`

12. More installs
    * `sudo apt-get install pigpio`
    * `sudo pip3 install pigpio`
    * `sudo apt-get install ninja`

13. crontab
    * `sudo crontab -e`
    * @reboot   /usr/bin/pigpiod

14. create sat service
    * `sudo cp satllazero/SAT0_OBC/setup/sat.service /lib/systemd/system/.`

15. load sat service
    * `sudo reboot`
    * `sudo systemctl daemon-reload`
    * `sudo systemctl enable sat.service`
    * `sudo systemctl start sat`
    * `sudo systemctl stop sat`
    * `sudo systemctl restart sat`