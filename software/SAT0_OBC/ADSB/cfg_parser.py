'''
    @file cfg_parser.py
    @brief Config Parser for SATLLA0 OBC.

    Copyright (C) 2023 @author Rony Ronen

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

from __future__ import print_function

import os
import sys

if sys.version_info[0] < 3:
    import ConfigParser as configparser
else:
    import configparser

config = configparser.SafeConfigParser()

get = config.get
config_dir = os.path.dirname(__file__)

def parser_config():
    config_file = os.path.join(config_dir, "config.conf")

    if not os.path.exists(config_file):
        sys.stderr.write("Error: Unable to find the config file!\n")
        sys.exit(1)

    # parse the configuration
    global config
    config.read_file(open(config_file))

parser_config()