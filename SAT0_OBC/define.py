'''
    @file define.py.py
    @brief Defines for SATLLA0 OBC.

    This contains the commands file for the SAT.
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

from enum import Enum

# states
class StateTypes(Enum):
    STATE_BUSY = 0
    STATE_READY = 1


# Interface Types
class ApiTypes(Enum):
    API_NO_DATA = 0
    API_ACK = 3
    API_NONE = 9


# command vals
class CmdTypes(Enum):
    CMD_NONE = 0
    CMD_GET_STATE = 1
    CMD_GET_DATA = 2
    CMD_POWER_ON = 3

    CMD_TAKE_PHOTO = 4  # take photo (smart)
    CMD_ADSB = 14  # ADSB log
    CMD_NEW_TAKE_PHOTO = 15
    CMD_UPLOAD_FILE = 16

    # technical missions
    CMD_POWER_OFF = 8
    CMD_DROP_OUTBOX = 9


class DataTypes(Enum):
    # data types
    OTHER = 0  # 0x00
    PHOTO = 1  # 0x01
    FULL_PHOTO = 7  # 0x07
    ICON = 8  # 0x08
    META = 20  # 0x14
    STARS = 21  # 0x15
    TEXT = 22  # 0x16
    DATA = 23  # 0x17
    STD_OUT = 24  # 0x18
    STD_ERR = 25  # 0x19
    LASER_TEXT = 26  # 0x1A
    LASER_VIDEO = 27  # 0x1B
    IMG_JPG = 28 # 0x1C
