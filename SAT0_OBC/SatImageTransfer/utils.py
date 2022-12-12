'''
    @file utils.py
    @brief Sat Image Transfer for SATLLA0 OBC.

    Copyright (C) 2023 @author Shai Aharon

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

import numpy as np

def save_meta(img_class, img, img_num, out_path, h_copy=False):
    if h_copy:
        with open('{}/_metafile.txt'.format(out_path), 'w') as f:
            f.write(str(img_class) + ',')
            f.write(','.join([str(x) for x in img.shape]))
            if len(img.shape) < 3:
                f.write(',1')
            f.write(',{}'.format(img_num))

    with open('{}/_metafile.bin'.format(out_path), 'wb') as fbin:
        fbin.write(img_class.astype(np.uint8).tobytes())
        for x in img.shape[:2]:
            fbin.write(np.array(x).astype(np.uint16).tobytes())
        c = 1 if len(img.shape) < 3 else img.shape[-1]
        fbin.write(np.array(c).astype(np.uint8).tobytes())

        fbin.write(np.array(img_num).astype(np.uint8).tobytes())


def read_meta(path, verbos=False):
    with open(path, 'rb') as f:
        class_num = int.from_bytes(f.read(1), 'little')
        h = int.from_bytes(f.read(2), 'little')
        w = int.from_bytes(f.read(2), 'little')
        c = int.from_bytes(f.read(1), 'little')
        img_num = int.from_bytes(f.read(1), 'little')

    if verbos:
        print("Class: {}".format(class_num))
        print("Lap depth: {}".format(img_num))
        print("Size: {},{},{}".format(h, w, c))
        print("File Size:", os.path.getsize(path))
        print()
