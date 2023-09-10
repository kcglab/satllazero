'''
    @file controller.py
    @brief Main controller for SATLLA0 OBC.

    This contains the commands file for the SAT.
    Copyright (C) 2023 @author Aharon Gorodischer

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
 '''

import os
import sys
import serialCom
import subprocess
import configparser
import msgGenerator
from time import time as time_time

from TakePicSmart import take_pic_smart

from define import *

cfg_file = "config.conf"
config = configparser.ConfigParser()


class Controller:
    print("Controller Class")
    imuConnected = False
    generator = msgGenerator.MsgGenerator()
    config.read(cfg_file)
    state = StateTypes.STATE_BUSY.value

    def __init__(self, mode=0):
        print("Controller Class __ init__")
        print("Create Outbox folder")
        if not os.path.exists("./outbox/"):
            try:
                os.makedirs("./outbox", exist_ok=True)
            except Exception as e:
                print(e)

        print("Create Sent folder")
        if not os.path.exists("./sent/"):
            try:
                os.makedirs("./sent", exist_ok=True)
            except Exception as e:
                print(e)

        # init serial with teensy
        print("Init serial com")
        try:
            self.serial = serialCom.SerialCom(self)
            serial_path = config.get("global", "serialPath")
            # '/dev/cu.usbserial-0001'
            self.serial.trigger(serialPath=serial_path)
        except Exception as e:
            print(e)

        # update boot count
        print("Update boot count")
        try:
            bootCount = int(config.get("global", "bootCount")) + 1
            # resetLogFactor = int(config.get("global", "resetLogFactor"))
            config.set("global", "bootCount", str(bootCount))
        except Exception as e:
            print(e)
        self.saveConfigFile()

        # empty log each resetLogFactor times (default is 10)
        # try:
        #     if bootCount % resetLogFactor == 0:
        #         log = open(logFileName, "w")
        #         log.write("\n")
        #         log.flush()
        #         log.close()

        #     # update log with boot count
        #     self.log = open(logFileName, "a")
        #     self.writeLog("boot " + str(bootCount))
        # except Exception as e:
        #     print(e)

        self.state = StateTypes.STATE_READY.value
        print(f"Controller initiated. State: {StateTypes.STATE_READY.value}")

    def saveConfigFile(self):
        print("saveConfigFile()")
        try:
            with open(cfg_file, "w") as configfile:
                config.write(configfile)
        except Exception as e:
            print(e)

    def msgArrived(self, msgByte):
        print("msgArrived()")
        try:
            print(f"Message arrived: {msgByte}")
            command = CmdTypes.CMD_NONE.value
            paramsList = bytearray(msgByte)[:-1]  # remove '\n'
            paramsListLen = len(paramsList)
            print(f"paramsList: {paramsList}, Len: {paramsListLen}")

            if paramsListLen > 0:
                command = paramsList[0]
                print(f"command: {command}")

            if command == CmdTypes.CMD_GET_STATE.value:  # 1
                print("called CmdTypes.CMD_GET_STATE")
                self.serial.sendMsg(bytearray([self.state]))

            elif command == CmdTypes.CMD_GET_DATA.value:  # 2
                print("*** CmdTypes.CMD_GET_DATA ***")
                mission, file_type, data = self.generator.readMsg()
                # print (f'Send data: {data}')
                if len(data) > 0:
                    # UART buffer size is 2^14
                    if len(data) >= 16384:
                        return
                    # send the length
                    try:
                        header = mission.to_bytes(
                            2, 'little') + file_type.to_bytes(1, 'little')
                    except:
                        header = b"000"
                    print(f'Header: {header}')
                    data = header + data
                    self.serial.sendMsg(data)
                else:
                    self.serial.sendMsg(
                        bytearray([ApiTypes.API_NO_DATA.value]))

            elif command == CmdTypes.CMD_POWER_ON.value:  # 3
                print("called CmdTypes.CMD_POWER_ON")
                self.serial.sendMsg(bytearray([ApiTypes.API_ACK.value]))

            elif command == CmdTypes.CMD_TAKE_PHOTO.value:  # 4
                print("*** CmdTypes.CMD_TAKE_PHOTO ***")
                self.state = StateTypes.STATE_BUSY.value
                self.serial.sendMsg(bytearray([ApiTypes.API_ACK.value]))

                # update indexes
                picCount = int(config.get("mission", "picCount"))
                missionCount = int(config.get("mission", "missionCount"))
                config.set("mission", "missionCount", str(missionCount + 1))
                config.set("mission", "picCount", str(picCount + 1))
                self.saveConfigFile()

                # set parameters
                width = paramsList[1] if paramsListLen > 1 else 100
                # mode: 1 = Day, 2 = Night,
                mode = paramsList[2] if paramsListLen > 2 else 1

                print(
                    f'take_pic_smart(missionCount={missionCount}, picCount={picCount}, width={width}, mode={mode})')

                take_pic_smart(missionCount, picCount, width, mode)

                # set ready
                self.state = StateTypes.STATE_READY.value
                print("*** CmdTypes.CMD_TAKE_PHOTO Done ***")

            elif command == CmdTypes.CMD_ADSB.value:  # 14 ADSB log
                print("*** CmdTypes.CMD_ADSB ***")
                self.state = StateTypes.STATE_BUSY.value

                self.serial.sendMsg(bytearray([ApiTypes.API_ACK.value]))

                missionCount = int(config.get("mission", "missionCount"))
                config.set("mission", "missionCount", str(missionCount + 1))
                self.saveConfigFile()

                import RPi.GPIO as GPIO
                gpio_fet_pin = int(config.get("RWCS", "gpio_fet_pin"))
                GPIO.setmode(GPIO.BCM)
                GPIO.setup(gpio_fet_pin, GPIO.OUT)
                GPIO.output(gpio_fet_pin, GPIO.HIGH)

                # make mission folder
                out_fld = f"./outbox/{missionCount}"
                if not os.path.exists(out_fld):
                    os.makedirs(out_fld, exist_ok=True)

                # set parameters
                timeout = paramsList[1] if paramsListLen > 1 else 60

                print(f'adsb_listener(out_fld={out_fld}, timeout={timeout})')

                from ADSB import adsb_listener
                adsb_listener.run(out_fld, timeout)

                GPIO.output(gpio_fet_pin, GPIO.LOW)

                self.state = StateTypes.STATE_READY.value
                print("*** CmdTypes.CMD_RWCS Done ***")

            elif command == CmdTypes.CMD_POWER_OFF.value:  # 8
                print("*** CmdTypes.CMD_POWER_OFF ***")
                self.serial.sendMsg(bytearray([ApiTypes.API_ACK.value]))  # 8
                # here we should start power off
                self.serial.finish()
                # self.log.close()
                subprocess.call(["sudo", "shutdown", "now"])

                print("*** CmdTypes.CMD_POWER_OFF Done ***")

            elif command == CmdTypes.CMD_DROP_OUTBOX.value:  # 9
                print("*** CmdTypes.CMD_DROP_OUTBOX ***")
                self.serial.sendMsg(bytearray([ApiTypes.API_ACK.value]))

                self.generator.dropAllMessages(folderName='outbox')
                mode = paramsList[1] if paramsListLen > 1 else 0
                if mode == 1:
                    self.generator.dropAllMessages(folderName='sent')
                print("*** CmdTypes.CMD_DROP_OUTBOX Done ***")

            elif command == CmdTypes.CMD_NEW_TAKE_PHOTO.value:  # 15
                start_time = time_time()
                # set state to in mission and sendmsg
                print("*** CmdTypes.CMD_NEW_TAKE_PHOTO ***")
                self.state = StateTypes.STATE_BUSY.value
                self.serial.sendMsg(bytearray([ApiTypes.API_ACK.value]))

                # get mission id
                missionCount = int(config.get("mission", "missionCount"))
                config.set("mission", "missionCount", str(missionCount + 1))
                self.saveConfigFile()

                # check for mission folder and create if needed
                out_fld = f"./outbox/{missionCount}"
                if not os.path.exists(out_fld):
                    os.makedirs(out_fld, exist_ok=True)

                # execute mission with parameters:
                from SatImageTaking import start_service
                print("Started start_service main")
                start_service.main(out_fld, paramsList[1:])
                print("Finished start_service main")
                # finished executing mission:
                self.state = StateTypes.STATE_READY.value
                print("*** CmdTypes.CMD_NEW_TAKE_PHOTO Done ***")
                end_time = (time_time() - start_time)
                print(f"Time taken to execute mission: {end_time}")

            elif command == CmdTypes.CMD_UPLOAD_FILE.value:  # 16
                print("*** CmdTypes.CMD_UPLOAD_FILE ***")

                # set state to in mission and sendmsg
                self.state = StateTypes.STATE_BUSY.value
                self.serial.sendMsg(bytearray([ApiTypes.API_ACK.value]))

                # get mission id
                missionCount = int(config.get(
                    "mission", "missionCount"))  # TODO
                config.set("mission", "missionCount", str(missionCount + 1))
                self.saveConfigFile()

                # check for mission folder and create if needed
                out_fld = f"./outbox/{missionCount}"
                if not os.path.exists(out_fld):
                    os.makedirs(out_fld, exist_ok=True)

                # get parameters:
                scriptNum = paramsList[1]
                line_num = paramsList[2]
                num_chars = paramsList[3]

                def add(i): return chr(int(hex(i), 16))
                txt = ''.join([add(i) for i in paramsList[4: 4 + num_chars]])

                reset = 1 if len(paramsList[: 4 + num_chars]) < len(
                    paramsList) and paramsList[-1] == 1 else 0  # TODO how to reset
                # execute mission with parameters:
                print('parms: * > * > ',
                      f'missionCount: {missionCount}, scriptNum: {scriptNum}, ', end='')
                print(
                    f'line_num: {line_num}, num_chars: {num_chars}, txt: {repr(txt)}, reset: {reset}')

                import uploading
                uploading.main(missionCount, scriptNum, line_num, txt, reset)

                # finished executing mission:
                self.state = StateTypes.STATE_READY.value
                print("*** CmdTypes.CMD_UPLOAD_FILE Done ***")
            else:
                print(f'*** Command Unknown: {command} ***')
                self.serial.sendMsg(bytearray([ApiTypes.API_ACK.value]))

        except:  # msgArrived
            exception_type, exception_object, exception_traceback = sys.exc_info()
            from traceback import format_tb
            error = format_tb(exception_traceback)[-1]
            print()
            print(exception_type, exception_object, error)

            self.state = StateTypes.STATE_READY.value
            # self.serial.sendMsg(bytearray([self.MISSION_ERR]))


if __name__ == "__main__":
    print(f'*** Controller() Started ***')
    cont = Controller()
    if len(sys.argv) > 1:
        for i in range(1, len(sys.argv)):
            c = sys.argv[i]
            cmd = chr(int(c))
            cmd = str(cmd) + '\n'
            cont.msgArrived(cmd.encode())
