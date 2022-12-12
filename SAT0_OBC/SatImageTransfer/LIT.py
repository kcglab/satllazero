'''
    @file LIT.py
    @brief Lora Image Transfer for SATLLA0 OBC.

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

import cv2
import numpy as np
import os
from PIL import Image

UP_SAMPLE_FACTOR = 4
KBYTE = 1024


def pyrGauss(img, depth, g_ker):
    pyr_lst = []

    pyr_lst.append(cv2.filter2D(img, -1, g_ker))
    min_size_depth = np.floor(np.log2(min(img.shape[:2]) / 2))
    max_depth = int(min(depth, min_size_depth))
    for i in range(max_depth):
        last_img = pyr_lst[-1]
        last_img = last_img[::2, ::2]
        pyr_lst.append(cv2.filter2D(last_img, -1, g_ker))

    return pyr_lst


def pyrLap(img, depth, g_ker):
    layer_lst = []

    gau_pyr = pyrGauss(img, depth, g_ker)
    layer_lst.append(gau_pyr[-1])
    last_img = gau_pyr[-1]
    for i in range(1, len(gau_pyr)):
        curr_img = gau_pyr[-(i + 1)]
        exp_img = np.zeros_like(curr_img)
        exp_img[::2, ::2] = last_img

        exp_img = cv2.filter2D(exp_img, -1, g_ker * UP_SAMPLE_FACTOR)

        lap_img = curr_img - exp_img

        layer_lst.append(lap_img)
        last_img = curr_img

    return layer_lst


def lapExp(pyr_lap, g_ker, depth=100):
    last_img = pyr_lap[0]

    for idx, lap in enumerate(pyr_lap[1:]):
        curr_img = pyr_lap[idx + 1]
        exp_img = np.zeros_like(curr_img)
        exp_img[::2, ::2] = last_img
        exp_img = cv2.filter2D(exp_img, -1, g_ker * 4)

        last_img = exp_img + lap
        if idx == depth:
            break

    return last_img


def saveLap(base_path, lap_pyr, max_size, verbos=False):
    save_paths = []
    for i, lap in enumerate(lap_pyr):
        lap = lap * 255
        offset = 128
        lap += offset
        lap = lap.astype(np.uint8)
        save_path = "{}_{}.png".format(base_path, i+1)
        save_path_jp2 = save_path[:-3] + 'jp2'

        if min(lap.shape[:2]) < 10:
            with open("{}_{}.bin".format(base_path, i+1, offset), 'wb') as fp:
                lap = lap.astype(np.uint8)
                h, w = lap.shape[:2]
                lap = lap.reshape(h, w, -1)
                lap_shape = np.array(lap.shape, dtype=np.uint8)

                fp.write(lap_shape)
                fp.write(lap)

                # save_paths.append(save_path)
                save_paths.append("{}_{}.bin".format(base_path, i+1, offset))
        elif min(lap.shape[:2]) < 32 or max_size < 3 * KBYTE:
            cv2.imwrite(save_path, lap, [int(cv2.IMWRITE_JPEG_QUALITY), 100])

            if os.path.getsize(save_path) > max_size:
                os.remove(save_path)
                break
            save_paths.append(save_path)
        else:
            quality = 0
            while True:
                Image.fromarray(lap).save(save_path_jp2, quality_layers=[quality])
                if os.path.getsize(save_path_jp2) < max_size:
                    break
                quality += 1
            if os.path.getsize(save_path_jp2) > (max_size - 1 * KBYTE):
                os.remove(save_path_jp2)
                break
            save_paths.append(save_path_jp2)
    return save_paths


def loadLap(base_path):
    tot_size = 0
    base_fld = os.path.dirname(base_path)
    base_file = os.path.basename(base_path)
    lap_paths = [x for x in os.listdir(base_fld) if x.startswith(base_file) and 'full' not in x]
    lap_paths.sort()
    offset = 128

    lap_pyr = []
    is_gray = True
    for i, fp in enumerate(lap_paths):
        save_path = os.path.join(os.path.dirname(base_path), fp)

        if fp.endswith('.jp2'):
            l_img = np.array(Image.open(save_path)).astype(float)
        elif fp.endswith('.png'):
            l_img = cv2.imread(save_path, 0 if is_gray else 1).astype(float)
        elif fp.endswith('.bin'):
            with open(save_path, 'rb') as binp:
                shape_bin = binp.read(3)
                h, w, c = np.frombuffer(shape_bin, dtype=np.uint8).astype(int)

                data = binp.read(h * w * c)
                l_img = np.frombuffer(data, dtype=np.uint8).reshape(h, w, c).astype(float)
                is_gray = l_img.shape[2] == 1

        lap_pyr.append((l_img - offset).squeeze() / 255)

        tot_size += os.path.getsize(os.path.join(os.path.dirname(base_path), fp))
    return lap_pyr


def getSizeRatio(img_path, lap_path, depth=100):
    org_size = os.path.getsize(img_path)
    lap_files = [os.path.getsize(os.path.join(lap_path, f)) for f in os.listdir(lap_path) if
                 os.path.isfile(os.path.join(lap_path, f))]
    load_size = sum(lap_files[:depth])

    print("Org Size:", org_size)
    print("Tot Size:", load_size)
    print("Size Ratio:", org_size / load_size)

    return org_size, load_size


def mse(a, b):
    return np.sqrt(np.square(a - b).mean())


def CompressImage(in_img: np.ndarray, path: str, max_size: int = 16 * KBYTE, comp_gray=True, save_full = False):
    if in_img is None:
        cap = cv2.VideoCapture(0)
        for i in range(12):  # ronyr: Let the camera focus
            res, in_img = cap.read()
        cap.release()
        print("Image Captured!")

    img = in_img
    if len(in_img.shape) > 2 and comp_gray:
        img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        # import matplotlib.pyplot as plt
        # plt.imshow(img)
        # plt.show()
    img = img / 255
    g_ker = cv2.getGaussianKernel(5, -1)
    g_ker = g_ker @ g_ker.T

    lap_pyr = pyrLap(img, 7, g_ker)
    jp2_img = img[:, :, [2, 1, 0]] if len(img.shape) > 2 else img
    Image.fromarray((jp2_img * 255).astype(np.uint8)).save('{}/full.jp2'.format(path), quality_layers=[40])
    max_size = min(max_size, os.path.getsize('{}/full.jp2'.format(path)))
    save_paths = saveLap('{}/lap_pyr'.format(path), lap_pyr, max_size)
    #ronyr: remove full
    if not save_full:
        try:
            os.remove('{}/full.jp2'.format(path))
        except:
            pass

    return save_paths, in_img


def LoadImage(path):
    g_ker = cv2.getGaussianKernel(5, -1)
    g_ker = g_ker @ g_ker.T

    lap_pyr_load = loadLap('{}/lap_pyr'.format(path))
    res_lap_load = lapExp(lap_pyr_load, g_ker)

    return res_lap_load
