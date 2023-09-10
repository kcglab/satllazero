'''
    @file utils.py
    @brief A file for the utility functions used in the mission.

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
from cv2 import imwrite as cv2_imwrite
from cv2 import imread as cv2_imread
from cv2 import resize as cv2_resize
from cv2 import INTER_AREA as cv2_INTER_AREA
from cv2 import IMWRITE_JPEG_QUALITY
from datetime import datetime
import os
import numpy as np


def get_image(path_or_image: str or np.ndarray, x=-1, dim=None) -> np.ndarray:
    """Get image from path or image.

    Args:
        path_or_image (str or np.ndarray): Path of the image or image.
        x (int, optional): Opencv flag. Defaults to -1 for colorful image.
        dim (tuple, optional): Resize dimension. Defaults to None.

    Returns:
        np.ndarray: Image.
    """
    if isinstance(path_or_image, str) and os.path.exists(path_or_image):
        path = path_or_image
        image = _read_image(path, x, dim)
    elif isinstance(path_or_image, np.ndarray):
        image = path_or_image
    elif isinstance(path_or_image, int):
        path = f"./sent/{path_or_image}/Img.jpeg"
        image = cv2_imread(path, -1)
    else:
        raise TypeError("pass path or image")
    return image


def _read_image(path: str, x=-1, dim=None) -> np.ndarray:
    """Read image from path.

    Args:
        path (str): Path of the image.
        x (int, optional): Opencv flag. Defaults to -1 for colorful image.
        dim (tuple, optional): Resize dimension. Defaults to None.

    Returns:
        np.ndarray: Image.
        """
    image = cv2_imread(path, x)
    if dim and len(dim):
        image = cv2_resize(image, dim)
    return image


def get_pic_path_list(path: str) -> list:
    """Get list of path of images in the directory.

    Args:
        path (str): Directory path.

    Returns:
        list: List of path of images in the directory.
    """
    file_list = os.listdir(path)
    pic_path_list = [path + '/' + i for i in file_list if i.endswith(('.png', '.jpg', '.jpeg', '.jfif')) and
                     not i.startswith('.')]
    return pic_path_list


def init_final_path(path: str) -> None:
    """Create directory if not exist.

    Args:
        path (str): Directory path.

    Returns:
        None
    """
    if isinstance(path, str) and not os.path.exists(path):
        os.makedirs(path)


def crop(img: np.ndarray, x: int, y: int, qvga=True) -> np.ndarray:
    """Crops an image to a given size.

    Parameters: 
        img (np.ndarray): A numpy ndarray representing an image.
        x (int): The x-coordinate of the point from which to start cropping the image.
        y (int): The y-coordinate of the point from which to start cropping the image.
        qvga (bool, optional): If True, the image will be cropped to QVGA size.
        otherwise, the image will be cropped to VGA size. Defaults to True(QVGA). 

    Returns:
        np.ndarray: The cropped image.
    """
    if qvga:
        WIDTH, HIGHT = 240, 320
    else:
        WIDTH, HIGHT = 480, 640
    if len(img.shape) > 2:
        h, w, c = img.shape
    else:
        h, w = img.shape
    x_init = x
    if x - WIDTH/2 < 0:
        x_init = 0
    elif x > w:
        x_init = w - WIDTH
    else:
        x_init = int(x - WIDTH/2)

    x_end = x_init + WIDTH
    if x_end > w:
        x_end = None

    y_init = 0
    if y - HIGHT/2 < 0:
        y_init = 0
    elif y - HIGHT/2 > h - HIGHT/2:
        y_init = h - HIGHT
    else:
        y_init = int(y - HIGHT/2)

    y_end = y_init + HIGHT
    if y_end > h:
        y_end = None

    crop_image = img[y_init: y_end, x_init: x_end]
    return crop_image


def if_stars(img: np.ndarray) -> bool:
    """Checks if an image is mostly black or not.

    Parameters:
        img (np.ndarray): A numpy ndarray representing an image.

    Returns:
        bool: True if the image is mostly black, False otherwise.
    """
    w, h = img.shape
    black_precent = np.sum(img < 15)*100/(w*h)
    return True if black_precent > 95 else False


def save_pic(pic_path: str, image: np.ndarray, compress_factor=95, dim=None) -> None:
    """Save image.

    Args:
        pic_path (str): Path of the image where it will be saved.
        image (np.ndarray): Image.
        dim (tuple, optional): Resize dimension. Defaults to None.
        compress_factor (int): Minimize image size. Defaults to 95.

    Returns:
        None
    """
    if dim and len(dim) == 2:
        image = cv2_resize(image, dim)
    image_name = datetime.now().strftime("%Y-%m-%d %H-%M-%S-%f") + '.jpeg'
    image_path = os.path.join(pic_path, image_name)
    compress = [IMWRITE_JPEG_QUALITY, compress_factor]
    cv2_imwrite(image_path, image, compress)


def resize(img: np.ndarray, dim=(60, 60)) -> np.ndarray:
    """Resizes an image to a given size.

    Parameters:
        img (np.ndarray): A numpy ndarray representing an image.
        dim (tuple, optional): The desired size in (width, height) format. Defaults to (60, 60).

    Returns:
        np.ndarray: The resized image.
    """
    new_img = cv2_resize(img, dim, Interpolation=cv2_INTER_AREA)
    return new_img
