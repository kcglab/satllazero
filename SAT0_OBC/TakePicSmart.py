'''
    @file TakePicSmart.py
    @brief Capture Image for SATLLA0 OBC.

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

# import time
from define import *

# ctime = lambda: int(round(time.time() * 1000))
# print("time", ctime())
# startTime = ctime()
# globalStartTime = startTime

import subprocess
import sys
# import msgGenerator
import os
import numpy as np

# endTime = ctime()
# print("importTime", (endTime - startTime))

"""
take normal pic
take night pic
compress a pic to stars
compress a pic to small
"""

def take_pic_smart(missionName="0", picOriginName="0", compressedSizeParam="3", mode=1):
    # pic size
    pic_size = int(compressedSizeParam)
    if pic_size <= 1:
        compressedSize = 50
    elif pic_size == 2:
        compressedSize = 75
    elif pic_size == 3:
        compressedSize = 100
    elif pic_size == 4:
        compressedSize = 160
    elif pic_size == 5:
        compressedSize = 320
    elif pic_size == 6:
        compressedSize = 640
    elif pic_size == 7:
        compressedSize = 720
    else:
        compressedSize = pic_size

    # startTime = ctime()
    # make mission folder
    path = f'./outbox{os.path.sep}{missionName}'
    if not os.path.exists(path):
        os.makedirs(path)

    picName = os.path.join(path, f'pic_orig_{picOriginName}.jpg')
    smallPicName = os.path.join(path, f'pic_small_{picOriginName}.jpg')
    starsDataName = os.path.join(path, f'stars_{picOriginName}.bin')

    # prepare msgGenerator
    # generator = msgGenerator.MsgGenerator()
    # endTime = ctime()
    # print("init tools", (endTime - startTime))

    # get an idea of what mode we are:
    if mode == 1:  # we are on pic mode
        takeDayPhoto(picName)
        compressPhoto(picName, smallPicName, compressedSize)
        # generator.generateMessages(
        #     reply_size, missionName, DataTypes.PHOTO.value, smallPicName
        # )

    elif mode == 2:
        takeNightPhoto(picName)
        compressPhoto(picName, smallPicName, compressedSize)
        # generator.generateMessages(
        #     reply_size, missionName, DataTypes.PHOTO.value, smallPicName
        # )

        import starDetector_10
        star_detector_bytes = starDetector_10.exportStars(picName, 10)
        star_detector_bytes_file = open(starsDataName, "wb")
        star_detector_bytes_file.write(star_detector_bytes)
        star_detector_bytes_file.close()

        # generator.generateMessages(
        #     reply_size, missionName, DataTypes.STARS.value, starsDataName
        # )

    elif mode == 3:
        takeDayPhoto(picName)
        # isStarsImage = starDetector_10.isStarsImage(picName)
        isStarsImage = isStarsImageLite(picName)

        if isStarsImage:
            takeNightPhoto(picName)
            import starDetector_10
            star_detector_bytes = starDetector_10.exportStars(picName, 10)
            star_detector_bytes_file = open(starsDataName, "wb")
            star_detector_bytes_file.write(star_detector_bytes)
            star_detector_bytes_file.close()

            # generator.generateMessages(
            #     reply_size, missionName, DataTypes.STARS.value, starsDataName
            # )

        compressPhoto(picName, smallPicName, compressedSize)
        # generator.generateMessages(
        #     reply_size, missionName, DataTypes.PHOTO.value, smallPicName
        # )

    else: #mode == 4 or else
        import starDetector_10

        # gyro test mode. we need to take 3 pics, and record the stars data
        takeNightPhoto(picName)

        star_detector_bytes = starDetector_10.exportStars(picName, 10)
        star_detector_bytes_file = open(starsDataName, "wb")
        star_detector_bytes_file.write(star_detector_bytes)
        star_detector_bytes_file.close()
        takeNightPhoto(picName)

        star_detector_bytes = starDetector_10.exportStars(picName, 10)
        star_detector_bytes_file = open(starsDataName, "ab")
        star_detector_bytes_file.write(star_detector_bytes)
        star_detector_bytes_file.close()
        takeNightPhoto(picName)

        star_detector_bytes = starDetector_10.exportStars(picName, 10)
        star_detector_bytes_file = open(starsDataName, "ab")
        star_detector_bytes_file.write(star_detector_bytes)
        star_detector_bytes_file.close()

        # generator.generateMessages(
        #     reply_size, missionName, DataTypes.STARS.value, starsDataName
        # )

    if os.path.isfile(starsDataName):
        img_size = os.path.getsize(starsDataName)

        with open('{}/_metafile.bin'.format(path), 'wb') as fbin:
            fbin.write(np.array(mode).astype(np.uint8).tobytes())
            fbin.write(np.array(img_size).astype(np.uint32).tobytes())
    elif os.path.isfile(smallPicName):
        from PIL import Image
        img = Image.open(smallPicName)
        img_height, img_width, c = np.array(img).shape
        img.close()
        img_size = os.path.getsize(smallPicName)

        with open('{}/_metafile.bin'.format(path), 'wb') as fbin:
            fbin.write(np.array(mode).astype(np.uint8).tobytes())
            fbin.write(np.array(img_height).astype(np.uint16).tobytes())
            fbin.write(np.array(img_width).astype(np.uint16).tobytes())
            fbin.write(np.array(c).astype(np.uint8).tobytes())
            fbin.write(np.array(img_size).astype(np.uint32).tobytes())

def isStarsImageLite(picName):
    # startTime = ctime()
    # import pillow
    from PIL import Image, ImageStat

    # convert to grayscale
    img = Image.open(picName).convert("LA")
    # calc mean
    stat = ImageStat.Stat(img)
    rms = stat.rms[0]
    # print("isStarsImageTime", (ctime() - startTime))
    print("rms", rms)
    if rms < 50:
        # run file - stars
        return True
    else:
        # run file brightness
        return False


def takeDayPhoto(picName):
    # startTime = ctime()
    process = subprocess.Popen(
        ["raspistill", "-n", "-o", picName, "-w", "640", "-h", "480"]
    )
    out, err = process.communicate()
    # endTime = ctime()
    # print("takeDayPhotoTIme", (endTime - startTime))


def takeNightPhoto(picName):
    process = subprocess.Popen(
        ["raspistill", "-n", "-ex", "night", "-o", picName, "-w", "640", "-h", "480"]
    )
    out, err = process.communicate()
    # endTime = ctime()
    # print("takeNightPhoto", (endTime - startTime))


def compressPhoto(picName, smallPicName, compressedSize):
    # startTime = ctime()
    process = subprocess.Popen(
        [
            "convert",
            picName,
            "-filter",
            "Triangle",
            "-define",
            "filter:support=2",
            "-thumbnail",
            str(compressedSize),  # this defines the new size of the pic
            "-unsharp",
            "0.25x0.25+8+0.065",
            "-dither",
            "None",
            "-posterize",
            "136",
            "-quality",
            "82",
            "-define",
            "jpeg:fancy-upsampling=off",
            "-interlace",
            "none",
            "-colorspace",
            "sRGB",
            "-strip",
            smallPicName,
        ]
    )
    out, err = process.communicate()
    # endTime = ctime()
    # print("compressTime", (endTime - startTime))


if __name__ == "__main__":
    # verify input params
    # startTime = ctime()
    # mission number
    if len(sys.argv) > 1:
        missionName = sys.argv[1]
    else:
        missionName = "0"
    # pic number
    if len(sys.argv) > 2:
        picOriginName = sys.argv[2]
    else:
        picOriginName = "0"

    # pic size
    if len(sys.argv) > 3:
        compressedSizeParam = sys.argv[3]
    else:
        compressedSizeParam = 3

    # mode: 1 - pic, 2 - nigh, 3 - smart
    if len(sys.argv) > 4:
        mode = sys.argv[4]
    else:
        mode = 1
    mode = int(mode)

    # endTime = ctime()
    # print("init params", (endTime - startTime))
    take_pic_smart(missionName, picOriginName, compressedSizeParam, mode)
    # print("globalTime", (ctime() - globalStartTime))
