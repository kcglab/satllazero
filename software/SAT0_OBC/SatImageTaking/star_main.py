'''
    @file star_main.py
    @brief A file for testing SATLLA0 OBC picture taking mission runtime.

    Copyright (C) 2023 @authors Zachi Ben-Shtirit, Assaf Hiya

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
from cv2 import cvtColor as cv2_cvtColor
from cv2 import COLOR_BGR2GRAY
from time import time as time_time


from SatImageTaking import earth, star_finder, utils


PIC_DIR_PATH = "./photos/"

FINAL_DIRECTORY_PATH = "./photos/stars detected/"
path_list = utils.get_pic_path_list(PIC_DIR_PATH)

analsis_time_list = []
mask_time_list = []
run_time_list = []

i = 1
for pic_path in path_list[:]:
    print(
        f' - - - - - - - - - -  {i} from {len(path_list)} - - - - - - - - - - - - ')
    dim = (1280, 720)
    start_time = time_time()

    earth_analasis = earth.earth(pic_path, dim=dim)
    mask_time = (time_time() - start_time)

    mask_time_list.append(mask_time)

    gray_image = cv2_cvtColor(earth_analasis.clear_sky, COLOR_BGR2GRAY)
    star_analasis = star_finder.star_finder(
        pic_path, gray_image, sensitivity=100, dim=dim)

    star_analasis.draw()
    analsis_time = (time_time() - start_time) - mask_time
    analsis_time_list.append(analsis_time)

    utils.save_pic(FINAL_DIRECTORY_PATH,
                   star_analasis.draw_image, dim=(680, 480))

    run_time = time_time() - start_time
    run_time_list.append(run_time)
    i += 1

print('earth_mask time avg  ', sum(mask_time_list)/len(mask_time_list))
print('analsis time avg  ', sum(analsis_time_list)/len(analsis_time_list))
print('run time avg  ', sum(run_time_list)/len(run_time_list))
