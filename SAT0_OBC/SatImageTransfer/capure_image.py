'''
    @file capure_image.py
    @brief Capture Image for SATLLA0 OBC.

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

import sys
import os

# from matplotlib import pyplot as plt
sys.path.append(os.path.dirname(os.path.realpath(__file__)))
import LIT
from SIClib import classifyImg, starFinder
import utils
import numpy as np

out_fld = '../output'

def CaptureImage(out_fld = out_fld):
    try:
        gray = False
        # cvt = lambda x: x[:, :, [2, 1, 0]]
        img = None
        # if len(sys.argv) > 1 and sys.argv[1] == "demo":
        #     print('Running Demo')
        #     import cv2
        #     np.random.seed()
        #     img_path = '../data/classes/stars/'
        #     # img_path += np.random.choice(os.listdir(img_path), 1)[0] + '/'
        #     img_path += np.random.choice(os.listdir(img_path), 1)[0]
        #     # img_path = '../data/sat_village.png'
        #     img = cv2.imread(img_path)

        # Create dirs
        # os.makedirs(out_fld + '/meta', exist_ok=True) #ronyr: no need

        save_paths, img = LIT.CompressImage(img, out_fld, max_size=16 * LIT.KBYTE, comp_gray=gray)
        # res_lap_load = LIT.LoadImage('../output')
        # plt.imshow(res_lap_load)

        print('Classifying Image')
        try:
            img_class, img_class_ser = classifyImg(img)
        except Exception as e:
            print(e)
            img_class = 'earth'
            img_class_ser = np.int64(2)

        utils.save_meta(img_class_ser, img, len(save_paths), out_fld, h_copy=False) #ronyr: no need txt file
        utils.read_meta('{}/_metafile.bin'.format(out_fld), verbos=True)

        total_size = sum([os.path.getsize(os.path.join(out_fld, x)) for x in os.listdir(out_fld) if x.startswith('lap')])
        print("Tot Size:", total_size)
        print(save_paths)
        print("Class: {}".format(img_class))

        if img_class == 'stars':
            stars_xy = starFinder(img, disp=True)
            np.save('{}/stars.npy'.format(out_fld), stars_xy.astype(np.uint16))
            print("Stars size:", os.path.getsize('{}/stars.npy'.format(out_fld)))
            print()
        # else:
        #     plt.imshow(cvt(img))
        #     plt.title(img_class)
        #     plt.show()
    except Exception as e:
        print(e)

if __name__ == '__main__':
    if len(sys.argv) > 1:
        missionCount = sys.argv[1]
    else:
        missionCount = 45

    out_fld = f"../outbox/{missionCount}"
    if not os.path.exists(out_fld):
        os.makedirs(out_fld, exist_ok=True)
    CaptureImage(out_fld)
