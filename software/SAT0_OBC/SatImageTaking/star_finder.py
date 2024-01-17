"""
    @file star_finder.py
    @brief A file for the finding stars function.

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
"""

# 17.1.2024 asaf h: changed cv2 import cv2_ >> cv2.
import cv2
from copy import deepcopy
# import matplotlib.pyplot as plt
import numpy as np

from SatImageTaking import utils


class star_finder:
    """A class to detect stars in an image.
      Args:
        path_or_image (str or list): Path to the image or the image itself.
        gray_image (np.ndarray, optional): Grayscale image to be processed. Defaults to None.
        sensitivity (int, optional): Sensitivity of the threshold. Defaults to 100.
        N_stars (int, optional): Number of stars to be found. Defaults to None.
        dim (tuple, optional): Dimensions of the image. Defaults to None.
        draw (bool, optional): Whether to draw the stars on the image. Defaults to False.

    Attributes:
        draw_image (np.ndarray): The image drawn with detected stars.
        image (np.ndarray): The original image.
        gray_image (np.ndarray): The grayscale version of the original image.
        sensitivity (int):  Sensitivity of the threshold used when finding stars.
        mask (np.ndarray): The mask used to find stars.
        stars (list): A list of detected stars, including thier image,  center(x,y), radius, and brightness.
        N_stars (int): The number of stars to detect.

    Methods:
        find_stars(): Finds stars in the image and stores them in the 'stars' attribute.
        extract_star(): Extracts a single star from the image.
        get_threshold(): Returns the threshold used to find stars.
        draw (): draw the stars on the image.
    """

    # 19.9.2023 asaf h: lowered the default sensitivity for better result
    def __init__(self, path_or_image: str or list, gray_image=0, sensitivity=50, N_stars=None, dim=None, draw=False,) -> None:
        """
        Initializes the star_finder object.
        """
        self.draw_image = None
        self.image = utils.get_image(path_or_image, dim=dim)
        if isinstance(gray_image, np.ndarray):
            self.gray_image = gray_image
        else:
            self.gray_image = cv2.cvtColor(self.image, cv2.COLOR_BGR2GRAY)
        self.sensitivity = sensitivity
        self.mask = self.get_threshold()

        self.stars = self.find_stars()
        N_stars = N_stars if N_stars == None or N_stars < len(
            self.stars) and N_stars != 0 else len(self.stars)
        self.N_stars = N_stars
        self.stars = self.stars[:N_stars]
        if draw:
            self.draw()

    def find_stars(self):
        """
        Finds stars in the image.

        Returns:
            list: List of stars found in the image. 
            each elemant contain the the following data:
                star - the image of the star.
                center - x, y cordinate of the stars (tuple).
                r - the radius of the star.
                brightness- the brightness of the star.
        """
        contours, _ = cv2.findContours(
            self.mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)[-2:]
        stars_data = []
        for cnt in contours:
            (x, y), r = cv2.minEnclosingCircle(cnt)
            if r > 10:
                continue
            x, y, r = int(x), int(y), int(r)
            center = x, y
            if r == 0:
                r = 1
            r += 1
            star = self.extract_star(y, x, r)
            # 19.9.2023 asaf h: removed my get_brightness function for:  brightest_pixal = np.amax(star)
            brightest_pixal = np.amax(star)
            # 19.9.2023 asaf h: filter out all the stars that dont not meet the minimum thresholdץ
            if brightest_pixal < self.sensitivity:
                continue

            stars_data.append(
                (star, center, r, brightest_pixal))
        # 19.9.2023 asaf h: sort by brightest_pixal
        stars_data.sort(key=lambda cnt: cnt[3], reverse=True)
        return stars_data

    def extract_star(self, y, x, r):
        """
        Extracts a star from the image.

        Args:
            y (int): Y coordinate of the star.
            x (int): X coordinate of the star.
            r (int): Radius of the star.

        Returns:
            np.ndarray: Cropped star.
        """
        star = self.gray_image[max(y - r, 0): y + r, max(x - r, 0): x + r]
        mask = np.zeros_like(self.gray_image)
        mask = cv2.circle(mask, (x, y), r, (255, 255, 255), -1)
        mask = mask[max(y - r, 0): y + r, max(x - r, 0): x + r]
        cropped_star = cv2.bitwise_and(star, star, mask=mask)
        return cropped_star

    def get_threshold(self):
        """
        Applies a threshold to the image

        Returns:
            np.ndarray: Thresholded image.
        """
        gray_image = cv2.GaussianBlur(
            self.gray_image, (5, 5), 0)
        _, gray_image = cv2.threshold(
            self.gray_image, self.sensitivity, 255, cv2.THRESH_TOZERO)
        adaptive_mean = cv2.adaptiveThreshold(
            gray_image, 255, cv2.ADAPTIVE_THRESH_MEAN_C, cv2.THRESH_BINARY_INV, blockSize=5, C=1)
        return adaptive_mean

    def draw(self):
        """
        Draws the stars on the image.
        """
        self.draw_image = deepcopy(self.image)
        for star, center, radius, b in self.stars[:self.N_stars]:
            cv2.circle(self.draw_image, center, radius + 4, (0, 255, 0), 2)

    def get_data(self):
        """
        Prints the data of the found stars.
        """
        for i in range(len(self.stars[:self.N_stars])):
            star, center, r, b = self.stars[i]
            # plt.imshow(star, cmap="gray")
            # plt.show()
            print(star)
