from cv2 import cvtColor as cv2_cvtColor
from cv2 import morphologyEx as cv2_morphologyEx
from cv2 import GaussianBlur as cv2_GaussianBlur
from cv2 import threshold as cv2_threshold
from cv2 import adaptiveThreshold as cv2_adaptiveThreshold
from cv2 import bitwise_and as cv2_bitwise_and
from cv2 import bitwise_not as cv2_bitwise_not
from cv2 import Sobel as cv2_Sobel
from cv2 import circle as cv2_circle
from cv2 import findContours as cv2_findContours
from cv2 import dilate as cv2_dilate
from cv2 import minEnclosingCircle as cv2_minEnclosingCircle
from cv2 import THRESH_TOZERO, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, MORPH_CLOSE, MORPH_OPEN, RETR_EXTERNAL,\
    CHAIN_APPROX_SIMPLE, COLOR_BGR2GRAY

import numpy as np

from SatImageTaking import utils


class earth:
    """
    Class earth

    Args:
        path_or_image: The path of the image or the image itself.
        dim: The image dimension (optional).

    Attributes:
        mask (np.ndarray): The mask of Earth in image. ##
        image (np.ndarray): The image itself.
        gray_image (np.ndarray): The gray version of the image.
        clear_sky (np.ndarray): The sky portion of the image.
        clear_earth (np.ndarray): The Earth portion of the image
        earth_mask (np.ndarray): The mask of the Earth portion of the image
        is_earth (bool): Boolean indicating if the Earth is in the imag.
        earth_percent (int): The percent of the earth in the image #
        curve (np.ndarray): The curve of the image  #
        is_dark_earth (bool): Boolean indicating if the image is of the Earth at night.
        is_stars (bool): Boolean indicating if contain only stars.
    """

    def __init__(self, path_or_image: str or np.ndarray, dim=None) -> None:
        """Constructor for earth class. Initializes the attributes of the class"""
        self.mask = None
        self.image = utils.get_image(path_or_image, dim=dim)
        self.gray_image = cv2_cvtColor(self.image, COLOR_BGR2GRAY)
        self.clear_sky, self.clear_earth, self.earth_mask = self.get_mask()
        self.is_earth = self._is_earth()
        self.earth_percent = self._earth_percent()
        self.curve = self.get_curve()
        if self.is_earth:
            self.is_dark_earth = False
        else:
            self.is_dark_earth = self._is_earth_at_night()
        self.is_stars = False if self.is_dark_earth or self.is_earth else True

    def _is_earth_at_night(self):
        """Detects if the image taken is of the Earth at night.

        Returns:
                bool: True if the image contain Earth at night, False otherwise.
        """
        gray_image = cv2_GaussianBlur(
            self.gray_image, (3, 3), 0)
        _, gray_image = cv2_threshold(
            self.gray_image, 80, 255, THRESH_TOZERO)
        mask = cv2_adaptiveThreshold(
            gray_image, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, blockSize=3, C=5)
        kernel = np.ones((5, 5), np.uint8)
        mask = cv2_morphologyEx(mask, MORPH_CLOSE, kernel)

        contours, _ = cv2_findContours(
            mask, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE)[-2:]

        self.mask = mask
        for cnt in contours:
            (x, y), r = cv2_minEnclosingCircle(cnt)
            if r > 60:
                return True
        return False

    def get_curve(self):
        """find the curve of the Earth.

        Returns:
            np.ndarray: image that contains the curve o the Earth"""
        gauss = cv2_GaussianBlur(self.gray_image, (7, 7), 0)

        _, gauss = cv2_threshold(gauss, 80, 255, THRESH_TOZERO)

        adaptive_mean = cv2_adaptiveThreshold(gauss, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV,
                                              blockSize=201, C=3)
        kernel = np.ones((5, 5), np.uint8)
        adaptive_mean = cv2_morphologyEx(
            adaptive_mean, MORPH_CLOSE, kernel)
        adaptive_mean = cv2_morphologyEx(
            adaptive_mean, MORPH_OPEN, kernel)
        return adaptive_mean

    def _is_earth(self):
        """Detects if the Earth is in the image

        Returns:
                bool: True if the image contain Earth, False otherwise.
        """
        height, width = self.gray_image.shape
        image_size, white_pix = height * \
            width, np.count_nonzero(self.earth_mask == 255)
        white_pix_percent = 1 - (image_size - white_pix) / image_size
        if white_pix_percent > 0.1:
            return True
        else:
            False

    def _earth_percent(self):
        """Returns the percent of the earth in the image"""
        height, width = self.gray_image.shape
        image_size, white_pix = height * \
            width, np.count_nonzero(self.earth_mask == 255)
        white_pix_percent = 1 - (image_size - white_pix) / image_size
        return white_pix_percent

    def _get_earth_mask(self):
        """Returns the earth mask in the image"""
        gauss = cv2_GaussianBlur(self.gray_image, (5, 5), 0)

        _, gray_image = cv2_threshold(
            gauss, 30, 255, THRESH_TOZERO)

        thresh = cv2_Sobel(src=gray_image, ddepth=-1, dx=1, dy=1, ksize=11)

        kernel = np.ones((10, 10), np.uint8)
        thresh = cv2_morphologyEx(thresh, MORPH_CLOSE, kernel)
        kernel = np.ones((20, 20), np.uint8)
        thresh = cv2_morphologyEx(thresh, MORPH_OPEN, kernel)
        kernel = np.ones((40, 40), np.uint8)
        thresh = cv2_morphologyEx(thresh, MORPH_CLOSE, kernel)
        kernel = np.ones((70, 70), np.uint8)
        thresh = cv2_morphologyEx(thresh, MORPH_OPEN, kernel)

        return thresh

    def draw_black_cir(self, contours):
        """This function helps to eliminate all the stars in the sky or fill the holes in Earth mask"""
        for center, radius in contours:
            cv2_circle(self.mask, center, radius, (0, 0, 0), -1)

    def _get_cir_contours(self):
        """
        Finds all the the little circles for a given image.

         Returns:
            list: A list containing the contours of the circles in the image
        """
        height, width = self.gray_image.shape
        contours, _ = cv2_findContours(
            self.mask, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE)[-2:]
        all_cir = []
        for cnt in contours:
            (x, y), r = cv2_minEnclosingCircle(cnt)
            if r < int(max(height, width) * 0.3):
                x, y, r = int(x), int(y), int(r)
                center = x, y
                all_cir.append((center, r))
        return all_cir

    def fill_earth(self):
        """Fill all the little holes in Earth mask image."""
        self.mask = cv2_bitwise_not(self.mask)
        contours = self._get_cir_contours()
        self.draw_black_cir(contours)
        self.mask = cv2_bitwise_not(self.mask)

    def get_mask(self):
        """
        Returns the clear sky, clear earth and earth mask of the image.
        The clear sky contains just the sky portion of the image.
        The clear_earth contains just the portion with the earth.
        The mask contains the Earth mask.
        """
        self.mask = self._get_earth_mask()
        contours = self._get_cir_contours()
        self.draw_black_cir(contours)

        kernel = np.ones((5, 5), np.uint8)
        self.mask = cv2_dilate(self.mask, kernel, iterations=2)

        self.fill_earth()

        kernel = np.ones((15, 15), np.uint8)
        self.mask = cv2_dilate(self.mask, kernel, iterations=1)

        clear_earth = cv2_bitwise_and(
            self.image, self.image, mask=self.mask)

        mask = cv2_bitwise_not(self.mask)
        clear_sky = cv2_bitwise_and(
            self.image, self.image, mask=mask)

        return clear_sky, clear_earth, self.mask
