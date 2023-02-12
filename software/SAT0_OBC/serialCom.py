'''
    @file serialCom.py
    @brief SerialCom wrapper for SATLLA0 OBC.

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
'''

import threading
import serial

class SerialCom(object):
    # def __init__(self):
    #     print ("Serial com created")
    # data
    __continueFlag = True

    # Initializer / Instance Attributes
    def __init__(self, controller):
        self.controller = controller

    # methods
    def commListener(self):
        print(f'commListener() thread started')
        while self.__continueFlag:
            try:
                # message = self.ser.readline()
                message = self.ser.read_until('\n')
                if len(message) > 1:
                    print(
                        f'Message arrived. Message={message}, Size: {len(message)}')
                    self.controller.msgArrived(message)
            except Exception as e:
                print(f'Error in commListener(): {e}')
        print("Serial Com completed")

    # def trigger(self, serialPath='/dev/ttyS0', serialSpeed=115200):
    # def trigger(self, serialPath='/dev/cu.usbserial-0001', serialSpeed=115200):
    def trigger(self, serialPath='/dev/serial0', serialSpeed=115200):
        print(f'trigger()')
        self.ser = serial.Serial(
            serialPath, serialSpeed, timeout=0.75)  # open serial port
        print(f'{self.ser.name} started')
        x = threading.Thread(target=self.commListener)
        x.start()

    def finish(self):
        print(f'finish()')
        self.__continueFlag = False

    def sendMsg(self, data):
        try:
            print(f'sendMsg(). Size: {len(data)}')
            # print(f'Send Message: {data}')
            self.ser.write(data)
        except Exception as e:
            print(e)
