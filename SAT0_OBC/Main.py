'''
    @file Main.py
    @brief Main function for SATLLA0 OBC.

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

import controller
import sys

if __name__ == "__main__":
    print(f' Controller() Started')
    cont = controller.Controller()
    cmd = bytearray()
    if len(sys.argv) > 1:
        for index in range(1, len(sys.argv)):
            byte = sys.argv[index]
            print(f"Byte: {byte}, value: {[int(byte, 16)]}")
            cmd.append(int(byte, 16))

        cmd.append(10)
        print(f'cmd={cmd}')
        cont.msgArrived(cmd)
