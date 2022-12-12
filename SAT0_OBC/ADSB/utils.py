'''
    @file utils.py
    @brief Utils for SATLLA0 OBC.

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

import numpy as np

def save_meta(unique_icao, out_path, h_copy=False):
    if h_copy:
        with open('{}/_metafile.txt'.format(out_path), 'w') as f:
            f.write(str(unique_icao))

    with open('{}/_metafile.bin'.format(out_path), 'wb') as fbin:
        fbin.write(np.array(unique_icao).astype(np.uint16).tobytes())

'''
name = 'ADSB_VEHICLE'
        fieldnames = ['ICAO_address', 'lat', 'lon', 'altitude_type', 'altitude', 'heading', 'hor_velocity', 'ver_velocity', 'callsign', 'emitter_type', 'tslc', 'flags', 'squawk']
        fieldtypes = ['uint32_t', 'int32_t', 'int32_t', 'uint8_t', 'int32_t', 'uint16_t', 'uint16_t', 'int16_t', 'char', 'uint8_t', 'uint8_t', 'uint16_t', 'uint16_t']
        fieldunits_by_name = {"lat": "degE7", "lon": "degE7", "altitude": "mm", "heading": "cdeg", "hor_velocity": "cm/s", "ver_velocity": "cm/s", "tslc": "s"}
'''
fieldtypes = [np.uint32, np.int32, np.int32, np.uint8, np.int32, np.uint16, np.uint16, np.int16, np.char.chararray, np.uint8, np.uint8, np.uint16, np.uint16]

# bytearray.fromhex(str_h)
def save_data(adsb_icao_set, adsb_callsign_set, adsb_msg_set, out_path, h_copy=False):
    if h_copy:
        with open('{}/datafile.txt'.format(out_path), 'w') as f:
            for msg in adsb_msg_set:
                f.write(str(msg) + '\n')

    with open('{}/datafile.bin'.format(out_path), 'wb') as fbin:
        for msg in adsb_msg_set:
            fbin.write(bytearray.fromhex(msg))

    with open('{}/cs_file.bin'.format(out_path), 'wb') as fbin:
        for key in adsb_callsign_set:
            fbin.write(bytearray.fromhex(key))

    with open('{}/ca_file.bin'.format(out_path), 'wb') as fbin:
        for key in adsb_icao_set:
            fbin.write(np.array(key).astype(np.uint32).tobytes())

import os

def readtxt(path, mode = 'r'):
    if os.path.isfile(path):
        with open(path, mode) as f_r:
            lines = f_r.read().strip().splitlines()
    return lines