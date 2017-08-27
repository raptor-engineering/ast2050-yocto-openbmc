# Copyright 2017 Raptor Engineering, LLC
# Copyright 2015-present Facebook. All rights reserved.
#
# This program file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program in a file named COPYING; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

from openbmc_gpio_table import BoardGPIO


# The fallowing table is generated using:
# python asus_gpio_parser.py data/asus-BMC-GPIO.csv
# DO NOT MODIFY THE TABLE!!!
# Manual modification will be overridden!!!

board_gpio_table_v1 = [
    BoardGPIO('GPIOA4', 'ASUS_BMC_CTL_LOCKOUT_N'),
    BoardGPIO('GPIOB1', 'CTL_REQ_POWERUP_N'),
    BoardGPIO('GPIOB6', 'CTL_REQ_RESET_N'),
    BoardGPIO('GPIOE1', 'LED_STA_BMC_N'),
    BoardGPIO('GPIOE2', 'LED_ERR_CPU2_N'),
    BoardGPIO('GPIOE3', 'LED_ERR_CPU1_N'),
    BoardGPIO('GPIOF0', 'CTL_REQ_POWERDOWN_N'),
    BoardGPIO('GPIOF4', 'CTL_REQ_SPD_MUX_S1'),
    BoardGPIO('GPIOF5', 'CTL_REQ_SPD_MUX_S0'),
    BoardGPIO('GPIOG0', 'LED_STA_LOC_N'),
    BoardGPIO('GPIOG1', 'USR_SW_LOC_N'),
    BoardGPIO('GPIOH2', 'STA_LINE_POWER'),
]
