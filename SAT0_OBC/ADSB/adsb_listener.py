'''
    @file adsb_listener.py
    @brief ADSB listener for SATLLA0 OBC.

    This file contains ADS-B listener file for the SATLLA0 OBC.
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

import os
import sys
sys.path.append(os.path.dirname(os.path.realpath(__file__)))
from cfg_parser import config as cfg

import threading
import serial
import time

from serial import SerialException
from utils import save_meta, save_data
from pymavlink.dialects.v20 import ardupilotmega as mavlink

MAVLINK_MAGIC = 0xfe

# Class to store bytes as they arrive
class fifo(object):
    def __init__(self):
        self.buf = []

    def write(self, data):
        self.buf += data
        return len(data)

    def read(self):
        return self.buf.pop(0)


# ads-b data
class ADSBCom(object):
    ser = ''
    listener_run = True
    data = b''
    thread = None
    adsb_msg_set = set()
    adsb_icao_set = set()
    adsb_callsign_set = set()
    new_icao = False
    new_callsign = False

    def __init__(self, serial_port='', baudrate=57600, out_fld = '.'):
        self.serial_port = serial_port
        self.baudrate = baudrate
        self.out_fld = out_fld
        print(f'ADSBCom created: serial_port={self.serial_port}, br={self.baudrate}')

    def open(self):
        self.listener_run = True
        if 'PigpioCom' not in str(self.serial_port):
            self.ser = serial.Serial(self.serial_port, self.baudrate, timeout=0.75)
        else:
            self.ser = self.serial_port
        print(f'listener() starting')
        x = threading.Thread(target=self.listener)
        x.start()
        self.thread = x

    def stop(self):
        try:
            self.listener_run = False
            if hasattr(self.thread, 'join'):
                self.thread.join()
        except Exception as e:
            print(f'Error in stop(): {e}')

    def listener(self):
        while self.listener_run:
            try:
                self.data += self.ser.readline()
                while self.data.count(MAVLINK_MAGIC) > 2: # There is a frame ready to process
                    frame_start = self.data.find(MAVLINK_MAGIC) # should be 0
                    frame_end = self.data[1:].find(MAVLINK_MAGIC) # next frame start
                    frame = self.data[frame_start : frame_end + 1]
                    self.data = self.data[frame_end + 1:]
                    self.parse_messages(frame)
                    if self.new_icao or self.new_callsign:
                        save_meta(len(self.adsb_icao_set), self.out_fld)
                        save_data(self.adsb_icao_set, self.adsb_callsign_set, self.adsb_msg_set, self.out_fld)
                        self.new_icao = False
                        self.new_callsign = False
            except SerialException as e:
                print(f'Error in listener(): {e}')
                sys.exit(1)
            except Exception as e:
                print(f'Error in listener(): {e}')
        print("Serial Com completed")

    def show_header(self, data):
        # split up header information (using construct)
        print(f'Msg:{data[0]}')
        print(f'Payload Len:{data[1]}')
        print(f'Seq:{data[2]}')
        print(f'SYSId:{data[3]}')
        print(f'CompId:{data[4]}')
        print(f'MsgId:{data[5]}')

    # show incoming mavlink messages
    def parse_messages(self, frame):
        f = fifo()
        mav = mavlink.MAVLink(f)
        # assume, our 0xFE was the start of a packet
        if len(frame) > 6:
            # header = self.data[:6]
            # payload_len = header[1]
            msg = bytearray(frame)
            try:
                m2 = mav.decode(msg)
                # show what fields it has
                # print("Got a message with id %u and fields %s" % (m2.get_msgId(), m2.get_fieldnames()))
                # print out the fields
                # print(m2)
                if m2.name == 'ADSB_VEHICLE':
                    print(m2)
                    # msg_h='fe26e0019cf6a1157400c00c15134065c514942249007d6bb7426501bf01000000524a4133354b20200003011732'
                    msg_h = msg.hex()
                    icao = getattr(m2, 'ICAO_address')
                    if icao not in self.adsb_icao_set:
                        self.adsb_icao_set.add(icao)
                        self.adsb_msg_set.add(msg_h)
                        self.new_icao = True

                    callsign = getattr(m2, 'callsign')
                    if len(callsign) > 0:
                        callsign_h = callsign.encode('utf8').hex()
                        if callsign_h not in self.adsb_callsign_set:
                            self.adsb_callsign_set.add(callsign_h)
                            self.adsb_msg_set.add(msg_h)
                            self.new_callsign = True
            except:
                pass
        else:
            return


def run(out_fld = '.', timeout = 60):
    serial_stream = cfg.get('ENV', 'serial_stream')
    serial_baudrate = int(cfg.get('ENV', 'serial_baudrate'))
    if serial_stream == 'serial':
        serial_port = cfg.get('ENV', 'serial_port')
    else:
        sys.path.insert(0, '..')
        import pigpio_serial as ps
        serial_port = ps.PigpioCom(baudrate=serial_baudrate)
        serial_port.open()

    adsb = ADSBCom(serial_port=serial_port, baudrate=serial_baudrate, out_fld=out_fld)
    try:
        adsb.open()
        start = time.time()
        print(f'Srart: {time.ctime()}')
        while time.time() - start < timeout: # timeout in sec
            # print(int(time.time() - start))
            pass
        print(f'Stop: {time.ctime()}')
        adsb.stop()
    except KeyboardInterrupt:
        print(time.ctime())
        adsb.stop()
        sys.exit(0)
    except Exception as e:
        print(e)
        print(time.ctime())
        adsb.stop()
        sys.exit(1)


if __name__ == "__main__":
    out_fld = '.'
    timeout = 60
    if len(sys.argv) > 1:
        out_fld = sys.argv[1]
    if len(sys.argv) > 2:
        timeout = int(sys.argv[2])
    run(out_fld, timeout)

# Test
#ICAO_address=3896644, lat=319094624, lon=352276608, altitude_type=0,
# altitude=12192000, heading=28918, hor_velocity=21092, ver_velocity=0,
# callsign=CTM1015 , emitter_type=5, tslc=0, flags=415, squawk=0,
# frames = readtxt('/Users/ronyr/Documents/Arduino/SAT/Teensy/temp/datafile.txt')
# for frame in frames:
#     self.parse_messages(bytearray.fromhex(frame))