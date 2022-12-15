'''
    @file pigpio_serial.py
    @brief pigpio wrapper for SATLLA0 OBC.

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
import pigpio

class PigpioCom(object):
    listener_run = True
    tx_pin = 20
    rx_pin = 21
    baudrate = 9600

    # thread = None

    def __init__(self, tx_pin = 20, rx_pin = 21, baudrate = 9600):
        self.tx_pin = tx_pin
        self.rx_pin = rx_pin
        self.baudrate = baudrate
        self.serialpi = pigpio.pi()
        self.serialpi.set_mode(self.rx_pin, pigpio.INPUT)
        self.serialpi.set_mode(self.tx_pin, pigpio.OUTPUT)
        print(f'PigpioCom created: rx={self.rx_pin}, tx={self.tx_pin}, br={self.baudrate}')

    def open(self):
        print(f'open()')
        pigpio.exceptions = False
        self.serialpi.bb_serial_read_close(self.rx_pin)
        pigpio.exceptions = True
        self.serialpi.bb_serial_read_open(self.rx_pin, self.baudrate, 8)

    def read(self, bytes = 1000):
        count, data = self.serialpi.bb_serial_read(self.rx_pin)
        if count > bytes:
            # print(f'read(): {count}, {data}')
            return data[:bytes]
        if count > 0:
            # print(f'read(): {count}, {data}')
            return data
        return b''

    def readline(self, bytes=1000):
        return self.read(bytes)

    def write(self, data):
        self.serialpi.wave_clear()
        self.serialpi.wave_add_serial(self.tx_pin, self.baudrate, data)
        wid=self.serialpi.wave_create()
        self.serialpi.wave_send_once(wid)
        while self.serialpi.wave_tx_busy():
            pass
        self.serialpi.wave_delete(wid)
        # print(f'write(): {len(data)}, {data}')
        return len(data)
