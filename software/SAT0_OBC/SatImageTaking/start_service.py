'''
    @file start_service.py
    @brief Main file for SATLLA0 OBC picture taking mission.

    Copyright (C) 2023 @authors Zachi Ben-Shtirit, Assaf Hiya.

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

# 17.1.2024 asaf h: changed cv2 import cv2_ >> cv2.
import cv2
import numpy as np
import os
from SatImageTaking import earth, star_finder, utils, LIT
from PIL import Image


def main(outputFolder: str, parameters_list: list) -> None:
    """Main function to start the photo analysis mission.
    The mission is run according to the given missionType and the other parameters,
    and the output image is saved to the outputFolder

    Args:
        outputFolder (str): Folder path to save the output image. 
        parameters_list (list): List of parameters to be passed to the mission, depending on the missionType.

    Returns:
        None

    Mission types and their respective parameters:

        0: TakePhotoAndDoStarAnalysis (none) will take a default picture and run star default star analysis on it.

        1: takePictureWithParameters (quality, width, height, shutter, ISO) will take a picture with custom parameters.

        2: crop_and_compress (mission_count, x, y, quality_factor, gray, qvga) will crop and compress an image from a specific mission,
         the result will be saved as qvga as default or as VGA otherwise.

        3: makeIcon (img_path, width, height, quality_factor, gray) will create an icon out of a given image path with
         custom parameters.

        4: StarAnalysis (mission_count, width, height, sensitive, n_stars, with_mask)  will run star analysis 
          on photos that have already been taken with given a custom parameters.

        5: testingOnPreSavedImages (missionID, x, y, quality_factor, gray, vga, img_n) will be able to run the above functions 
          on pre-saved images with a given parameters.
    """
    global output
    output = outputFolder

    global missionType

    print("\n")
    print(f"output: {output}")
    print("Parameters: ")
    for item in parameters_list:
        print(item)
    print("\n")
    # make arguements by missionType:
    missionType = parameters_list[0] if len(parameters_list) > 0 else 0

    # TODO: continue testing missions: 4,5

    if(missionType == 0 or missionType == 1):
        quality = parameters_list[1] if len(parameters_list) > 1 else 0
        width = (parameters_list[2] * parameters_list[3]
                 ) if len(parameters_list) > 3 else 0
        height = (parameters_list[4] * parameters_list[5]
                  ) if len(parameters_list) > 5 else 0
        Shutter = (parameters_list[6] * parameters_list[7]
                   ) if len(parameters_list) > 7 else 0
        ISO = (parameters_list[8] * parameters_list[9]
               ) if len(parameters_list) > 9 else 0

        mission = mission_Table[missionType]
        mission(quality, width, height, Shutter, ISO)

    elif(missionType == 2):
        mission_count = parameters_list[1] if len(parameters_list) > 1 else 0
        x = (parameters_list[2] * parameters_list[3]
             ) if len(parameters_list) > 3 else 0
        y = (parameters_list[4] * parameters_list[5]
             ) if len(parameters_list) > 5 else 0
        quality_factor = parameters_list[6] if len(parameters_list) > 6 else 0
        gray = parameters_list[7] if len(parameters_list) > 7 else 0
        qvga = parameters_list[8] if len(parameters_list) > 8 else 1

        mission = mission_Table[missionType]
        mission(mission_count, x, y, quality_factor, gray, qvga)

        crop_path = f"{output}/Img.jpeg"
        writeMetaDataSmallImg(crop_path, missionType)

    elif(missionType == 3):
        img_path = parameters_list[1] if len(parameters_list) > 1 else 0
        width = (parameters_list[2] * parameters_list[3]
                 ) if len(parameters_list) > 3 else 0
        height = (parameters_list[4] * parameters_list[5]
                  ) if len(parameters_list) > 5 else 0
        quality_factor = parameters_list[6] if len(parameters_list) > 6 else 0
        gray = parameters_list[7] if len(parameters_list) > 7 else 0

        mission = mission_Table[missionType]
        mission(img_path, width, height, quality_factor, gray)

        icon_path = f"{output}/icon.jpeg"
        writeMetaDataSmallImg(icon_path, missionType)

    elif(missionType == 4):
        mission_count = parameters_list[1] if len(parameters_list) > 1 else 0
        width = (parameters_list[2] * parameters_list[3]
                 ) if len(parameters_list) > 3 else 0
        height = (parameters_list[4] * parameters_list[5]
                  ) if len(parameters_list) > 5 else 0
        sensitive = parameters_list[6] if len(parameters_list) > 6 else 0
        n_stars = parameters_list[7] if len(parameters_list) > 7 else 0
        with_mask = parameters_list[8] if len(parameters_list) > 8 else 0

        mission = mission_Table[missionType]
        mission(mission_count, width, height, sensitive, n_stars, with_mask)

        detected_path = f"{output}/Detected.jpeg"
        writeMetaDataSmallImg(detected_path, missionType)

    elif(missionType == 5):
        missionID = parameters_list[1] if len(parameters_list) > 1 else 0
        x = (parameters_list[2] * parameters_list[3]
             ) if len(parameters_list) > 3 else 0
        y = (parameters_list[4] * parameters_list[5]
             ) if len(parameters_list) > 5 else 0
        quality_factor = parameters_list[6] if len(parameters_list) > 6 else 0
        gray = parameters_list[7] if len(parameters_list) > 7 else 0
        vga = parameters_list[8] if len(parameters_list) > 8 else 0
        img_n = parameters_list[9] if len(parameters_list) > 9 else 0

        mission = mission_Table[missionType]
        # 19.9.2023 asaf h: missionID was missing, it is like missionType, 7 arguments are needed for testing
        mission(missionID, x, y, quality_factor, gray, vga, img_n)


def TakePhotoStarAnalysis(quality: int, width: int, height: int, Shutter: int, ISO: int) -> None:
    """
    Function to take a picture with the given parameters and perform star analysis

    Args:
        quality (int): quality of the taken picture
        width (int): width of the taken picture
        height (int): height of the taken picture
        Shutter (int): the shutter speed of the taken picture
        ISO (int): the ISO of the taken picture

    Returns:
        None
    """
    print("Started TakePhotoStarAnalysis")
    # 19.9.2023 asaf h: there is no need to define the parameters, takePictureWithParameters does that.
    img = takePictureWithParameters(quality, width, height, Shutter, ISO)

    # define Dimensions
    dim = (1280, 720)
    # create img_object
    img_object = earth.earth(img, dim=dim)

    # checking whether the earth is seen
    is_Dark_Earth = img_object.is_dark_earth
    # star detection
    stars_list = []
    if(not is_Dark_Earth):
        detected_object = findStarsNoDarkEarth(img, img_object)
        stars_list = get_stars(detected_object)
    # get stars list:
    # create meta file for stars and mission summary
    print("Starting writeMetaDataStars")
    writeMetaDataStars(stars_list)

# 19.9.2023 asaf h: the default numbers needs to be removed cuz 0 is sent not None, they will be difined later
def StarAnalysis(mission_count: int, width: int, height: int, sensitive: int, n_stars: int, without_earth_mask=0):
    """Analyzes an image to identify stars

    Args:
        mission_count (int): The mission count of the image being analyzed
        width (int): The width of the image (default 1280)
        height (int): The height of the image (default 720)
        sensitive (int): The sensitivity of the star detection algorithm (default 100)
        n_stars (int): The maximum number of stars to detect (default 30)
        without_earth_mask (int): Whether or not to use a mask for the Earth (default 0)

    Returns:
            stars_list (list): A list of the detected stars in the image
    """
    print("Started StarAnalysis")
    # 19.9.2023 asaf h: fixed the default numbers
    if width == 0:
        width = 1280
    if height == 0:
        height = 720
    if sensitive == 0:
        sensitive = 50
    if n_stars == 0:
        n_stars = 30

    img = utils.get_image(mission_count)

    # define analysis Dimensions
    dim = (width, height)

    img_object = earth.earth(img, dim=dim)

    # star detection
    detected_object = findStarsNoDarkEarth(img, img_object, sensitive, n_stars)
    #  19.9.2023 asaf h:     detected_object.draw() and     saveFinalImg(detected_object.draw_image, "Detected")
    # will save the image with the green circels on the stars on the pi for future analysis. its unnecessary.

    # get stars list:
    stars_list = get_stars(detected_object)

    # create meta file for stars and mission summary
    print("Starting writeMetaDataStars")
    writeMetaDataStars(stars_list)
    print("Starting saveFinalImg")

# 19.9.2023 asaf h: removed takeStandardPicture function cuz there is no need to. check line 268 (else)

# 19.9.2023 asaf h: fixed the main function takePictureWithParameters. rony please double check it again. it should be fine
def takePictureWithParameters(quality: int, width: int, height: int, Shutter: int, ISO: int) -> np.ndarray:
    """Take a picture with specified parameters 

    Args:
        quality (int): The quality of the image (default 100)
        width (int): The width of the image (default 1280)
        height (int): The height of the image (default 720)
        Shutter (int): The shutter speed of the camera (default 0)
        ISO (int): The ISO of the camera (default 0)

    Returns:
            img (np.ndarray): A numpy array containing the image
    """

    print("Started takePictureWithParameters")
    if quality == 0:
        quality = 100
    if width == 0:
        width = 1280
    if height == 0:
        height = 720

    if (Shutter != 0 and ISO != 0):
        command_with_parameters = f"raspistill -o {output}/Img.jpeg --quality {quality} --width {width} --height {height} --shutter {Shutter} --ISO {ISO} -th none"
    elif (Shutter != 0):
        command_with_parameters = f"raspistill -o {output}/Img.jpeg --quality {quality} --width {width} --height {height} --shutter {Shutter} -th none"
    elif (ISO != 0):
        command_with_parameters = f"raspistill -o {output}/Img.jpeg --quality {quality} --width {width} --height {height}  --ISO {ISO} -th none"
    else:
        command_with_parameters = f"raspistill -o {output}/Img.jpeg --quality {quality}  --width {width} --height {height} -th none"

    # 19.9.2023 asaf h: removed the take standard picture. unnecessary function

    os.system(command_with_parameters)
    read = f"{output}/Img.jpeg"
    img = cv2.imread(read)

    print("Compressing")
    CompressImage(img, output, save_full=True)
    print("Making icon")
    makeIcon(img)
    print("checking if good image")
    isGoodImage = decideIfGoodImage(img)
    print("writing metadata")
    writeMetaDataSmallImg(read, isGoodImage)
    return img

# 19.9.2023 asaf h: lower the default sensitive for better result
def findStarsNoDarkEarth(img: np.ndarray, img_object: np.ndarray, sensitive=50, n_stars=30) -> np.ndarray:
    """Finds stars in an image while the Earth is lit and not dark

    Args:
            img (np.ndarray): An array containing the image
            img_object (np.ndarray): An array containing the object
            sensitive (int): The sensitivity of the star detection algorithm (default 100)
            n_stars (int): The maximum number of stars to detect (default 30)

    Returns:
            detected_object (np.ndarray): An array containing the detected objects
    """
    gray_image = cv2.cvtColor(img_object.clear_sky, cv2.COLOR_BGR2GRAY)
    star_detection = star_finder.star_finder(
        img, gray_image, sensitivity=sensitive, N_stars=n_stars)
    return star_detection


def get_stars(detected_object: list) -> list:
    """Gets a list of the detected stars in an image

    Args:
        detected_object (list): A list of the detected objects

    Returns:
            stars_list (list): A list of the detected stars in the image
    """
    stars_list = []
    for star, center, radius, b in detected_object.stars:
        stars_list.append(center)
    return stars_list


def writeMetaDataStars(stars_list: list) -> None:
    """Write meta data stars in a binary file

    Args:
        stars_list (list): List of stars. Each star is a couple (x,y) of coordinates. 

    Returns:
        None.
    """

    FINAL_PATH = f"{output}/metastars.bin"

    if(len(stars_list) == 0):
        pass
    else:
        with open(FINAL_PATH, 'wb+') as f:
            f.write(np.array(len(stars_list)).astype(np.uint8).tobytes())
            for x in stars_list:
                print(f"star: {x}")
                h, w = x
                f.write(np.array(h).astype(np.uint16).tobytes())
                f.write(np.array(w).astype(np.uint16).tobytes())


def writeAnalysisMetadata(stars_list: list, width: int, height: int) -> None:
    """Write metadata of the width and height of the image that has been taken in a binary file.

    Args:
        stars_list (list): List of stars. Each star is a couple (x,y) of coordinates. 
        width (int): Width of the image.
        height (int): Height of the image.

    Returns:
        None.
    """
    FINAL_PATH = f"{output}/metadataAnalysis.bin"

    with open(FINAL_PATH, 'wb+') as f:
        f.write(np.array(width).astype(np.uint16).tobytes())
        f.write(np.array(height).astype(np.uint16).tobytes())


def writeMetaDataSmallImg(path: str, isGoodImage: bool) -> None:
    """Write metadata of small image in a binary file.
    The metadata contains the weight, mission_count, and if Earth has been captured (isGoodImage)

    Args:
        path (str): Path to the image.
        isGoodImage (boolean): Indicates if Earth has been captured.

    Returns:
        None.
    """
    FINAL_PATH = f"{output}/_metafile.bin"
    i = output.rfind('outbox/') + 7
    mission_count = int(output[i:])
    with open(FINAL_PATH, 'wb+') as f:
        weight = os.path.getsize(path)
        weight = min(weight, 65535)
        f.write(np.array(weight).astype(np.uint16).tobytes())
        f.write(np.array(mission_count).astype(np.uint8).tobytes())
        f.write(np.array(isGoodImage).astype(np.uint8).tobytes())


def crop_and_compress(mission_count: int, x: int, y: int, quality_factor=95, gray=0, qvga=0) -> np.ndarray:
    """Crop and compress an exist image by mission_count parameter.

    Args:
        mission_count (int): Mission count of the image.
        x (int): X coordinate of the star.
        y (int): Y coordinate of the star.
        quality_factor (int, optional): Quality factor of the image. Defaults to 95.
        gray (boolean, optional): Indicates if the image is gray or not. Defaults to 0.
        qvga (boolean, optional): Indicates if the image is qvga or not. Defaults to 0.

    Returns:
        np.ndarray: The cropped image.
    """
    print("Starting crop_and_compress")
    if quality_factor == 0:
        quality_factor = 95
    img = utils.get_image(mission_count)
    crop_img = utils.crop(img, x, y, qvga)
    if(gray):
        crop_img = cv2.cvtColor(crop_img, cv2.COLOR_BGR2GRAY)
    saveFinalImg(crop_img, 'Img', quality_factor)
    read = f"{output}/Img.jpeg"
    cropped_img = cv2.imread(read)
    CompressImage(cropped_img, output, save_full=True)
    makeIcon(crop_img)


def decideIfGoodImage(img: np.ndarray) -> bool:
    """Decide if an image is good

    Args:
        img (str): Path to the image.

    Returns:
        int: 1 if the image is good, 0 otherwise.
    """
    img = utils.get_image(img)
    img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    w, h = img.shape
    black_percent = np.sum(img < 15)*100/(w*h)
    return 1 if black_percent > 95 else 0


def makeIcon(img_path: str, width=80, height=80, quality_factor=21, gray=0) -> None:
    """Make an icon from an image

    Args:
        img_path (str): Path to the image.
        width (int, optional): Width of the icon. Defaults to 80.
        height (int, optional): Height of the icon. Defaults to 80.
        quality_factor (int, optional): Quality factor of the icon. Defaults to 21.
        gray (boolean, optional): Indicates if the icon is gray or not. Defaults to 0.

    Returns:
        None.
    """
    print("Starting makeIcon")
    if width == 0:
        width = 80
    if height == 0:
        height = 80
    if quality_factor == 0:
        quality_factor = 21

    dim = (width, height)
    img = utils.get_image(img_path)
    resized_image = cv2.resize(img, dim, interpolation=cv2.INTER_AREA)
    if gray:
        resized_image = cv2.cvtColor(resized_image, cv2.COLOR_BGR2GRAY)
    saveFinalImg(resized_image, 'icon', quality_factor)


def saveFinalImg(star_detection: np.ndarray, name: str, compress_factor=95) -> None:
    """Save the final image

    Args:
        star_detection (np.ndarray): The star detection.
        name (str): Name of the file.
        compress_factor (int, optional): Compression factor of the image. Defaults to 95.

    Returns:
        None.
    """
    FINAL_PATH = f"{output}/{name}.jpeg"
    compress = [cv2.IMWRITE_JPEG_QUALITY, compress_factor]
    cv2.imwrite(FINAL_PATH, star_detection, compress)


def testing(missionType: int, x: int, y: int, quality_factor: int, gray: bool, qvga: bool, img_n=0) -> int:
    """Testing function

    Args:
        missionType (int): Mission type.
        x (int): X coordinate of the star.
        y (int): Y coordinate of the star.
        quality_factor (int): Quality factor of the image.
        gray (boolean): Indicates if the image is gray or not.
        qvga (boolean): Indicates if the image is vga or not.
        img_n (int, optional): Number of the image. Defaults to 0.

    Returns:
        None.
    """
    img_path = f"SatImageTaking/photos{img_n}.jpeg"
    mission = mission_Table[missionType]
    # 19.9.2023 asaf h: each function has its own order
    if mission == makeIcon:  #
        mission(img_path, x, y, quality_factor, gray)
    elif mission == crop_and_compress:
        mission(img_path, x, y, quality_factor, gray, qvga)
    else: #StarAnalysis
        sensitive = quality_factor
        n_stars = gray
        without_earth_mask = qvga
        mission(img_path, x, y, sensitive, n_stars, without_earth_mask)


def CompressImage(in_img: np.ndarray, path: str, max_size: int = 16 * LIT.KBYTE, comp_gray=True, save_full=False) -> str and np.ndarray:
    """compresses an image using a combination of Gaussian kernels and Laplacian pyramids.

    Args:
        in_img (np.ndarray): The image to compress.
        path (str): Path to the image.
        max_size (int, optional): Maximum size of the compressed image. Defaults to 16 * LIT.KBYTE.
        comp_gray (boolean, optional): Indicates if the image is gray or not. Defaults to True.
        save_full (boolean, optional): Indicates if the image is full or not. Defaults to False.

    Returns:
        tuple: Tuple containing the paths of the saved files and the shape of the original image.
    """
    img = in_img
    if len(in_img.shape) > 2 and comp_gray:
        img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    img = img / 255
    g_ker = cv2.getGaussianKernel(5, -1)
    g_ker = g_ker @ g_ker.T
    lap_pyr = LIT.pyrLap(img, 7, g_ker)
    jp2_img = img[:, :, [2, 1, 0]] if len(img.shape) > 2 else img
    Image.fromarray((jp2_img * 255).astype(np.uint8)
                    ).save('{}/full.jp2'.format(path), quality_layers=[40])
    max_size = min(max_size, os.path.getsize('{}/full.jp2'.format(path)))
    save_paths = LIT.saveLap('{}/lap_pyr'.format(path), lap_pyr, max_size)
    if not save_full:
        try:
            os.remove('{}/full.jp2'.format(path))
        except:
            pass

    return save_paths, in_img


mission_Table = {0: TakePhotoStarAnalysis, 1: takePictureWithParameters,
                 2: crop_and_compress, 3: makeIcon, 4: StarAnalysis, 5: testing}

