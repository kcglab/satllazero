'''
    @file msgGenerator.py
    @brief Message Generator for SATLLA0 OBC.

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

# type - 1 byte
# length - 1 byte
# index - 1 byte
# sub index - 1 byte
# data - max 146 bytes

import os
import stat
from define import *

class MsgGenerator:
    def generateEmptySignal(self):
        result = bytearray([9])
        return result

    # read the parts
    def readMsg(self):
        print(f'readMsg()')
        folderName = 'outbox'
        ans = b''
        mission = 0
        file_type = DataTypes.OTHER.value

        # r=root, d=directories, f = files
        if len(os.listdir(folderName)) < 1:
            print("Directory is empty")
            return mission, file_type, ans
        print(f'Folders: {len(os.listdir(folderName))}')

        for r, d, f in os.walk(folderName):
            # if no files, continue
            if len(f) < 1:
                continue
            mission = self.extractMission(r)
            moveFolder = self.createMoveFolder(mission)

            # loop file by file and send it
            for i, name in enumerate(sorted(f)):
                print(f'File Name: {name}')
                if len(name) > 0 and not name.startswith('.'):
                    if 'metafile' in name:
                        file_type = DataTypes.META.value
                    elif 'stars' in name:
                        file_type = DataTypes.STARS.value
                    elif 'pic' in name:
                        file_type = DataTypes.PHOTO.value
                    elif "full" in name:
                        file_type = DataTypes.FULL_PHOTO.value
                    elif "icon" in name:
                        file_type = DataTypes.ICON.value
                    elif "Img.jpeg" in name:
                        file_type = DataTypes.IMG_JPG.value
                    elif 'lap_pyr' in name:
                        try:
                            filename, file_extension = os.path.splitext(name)
                            lap_pyr = filename.split('_')[-1]
                            file_type = int(lap_pyr)
                        except:
                            file_type = i
                    else:
                        file_type = DataTypes.OTHER.value
                    line = open(os.path.join(r, name), "rb").read()
                    print(f'File Size: {len(line)}')
                    # print(f'Line: {line}')
                    # os.remove(os.path.join(r, name))
                    os.rename(os.path.join(r, name),
                              os.path.join(moveFolder, name))
                    return mission, file_type, line

        # reach end. no more files.
        return mission, file_type, ans

    def extractMission(self, root):
        try:
            mission = root.split(os.path.sep)[-1]
            mission = int(mission)
        except:
            mission = 0
        print(f'Mission: {mission}')
        return mission

    def createMoveFolder(self, mission):
        sentFolder = './sent'
        moveFolder = f'{sentFolder}{os.path.sep}{mission}'
        if not os.path.exists(moveFolder):
            try:
                os.makedirs(moveFolder, exist_ok=True)
            except Exception as e:
                print(e)
        print(f'Move Folder: {moveFolder}')
        return moveFolder

    # --------------------------
    # drop all messages
    def dropAllMessages(self, folderName='./outbox'):
        # r=root, d=directories, f = files
        for root, dirs, files in os.walk(folderName, topdown=False):
            if len(root.split(os.path.sep)) > 1:
                for name in files:
                    filename = os.path.join(root, name)
                    print(f'removing: {filename}')
                    try:
                        os.chmod(filename, stat.S_IWUSR)
                        os.remove(filename)
                    except OSError as e:
                        print("Error: %s - %s." % (e.filename, e.strerror))
                for name in dirs:
                    print(f'removing: {name}')
                    try:
                        os.rmdir(os.path.join(root, name))
                    except OSError as e:
                        print("Error: %s - %s." % (e.filename, e.strerror))
                print(f'removing: {root}')
                try:
                    os.rmdir(root)
                except OSError as e:
                    print("Error: %s - %s." % (e.filename, e.strerror))
