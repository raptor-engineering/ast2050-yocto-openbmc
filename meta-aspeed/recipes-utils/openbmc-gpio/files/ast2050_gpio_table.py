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

from openbmc_gpio_table import (
    BitsEqual, BitsNotEqual, And, Or, Function)


# The fallowing table is generated using:
# python ast_gpio_parser.py data/ast2050-gpio.csv
# DO NOT MODIFY THE TABLE!!!
# Manual modification will be overridden!!!

soc_gpio_table = {
    'A10': [
        Function('VBCK', BitsEqual(0x70, [5], 0x1)),
        Function('GPIOB5', None)
    ],
    'A11': [
        Function('FLBUSY#', BitsEqual(0x74, [2], 0x1)),
        Function('GPIOB1', None)
    ],
    'A12': [
        Function('SDA7', BitsEqual(0x74, [14], 0x1)),
        Function('GPIOH2', None)
    ],
    'A13': [
        Function('MII2DIO', BitsEqual(0x74, [20], 0x1)),
        Function('SDA5', BitsEqual(0x74, [12], 0x1)),
        Function('GPIOC6', None)
    ],
    'A8': [
        Function('PWM4', BitsEqual(0x74, [11], 0x1)),
        Function('GPIOC5', None)
    ],
    'A9': [
        Function('PECIO', BitsEqual(0x74, [7], 0x1)),
        Function('GPIOC1', None)
    ],
    'B1': [
        Function('OSCCLK', BitsEqual(0x2c, [1], 0x1)),
        Function('DDCACLK', BitsEqual(0x74, [18], 0x1)),
        Function('GPIOD7', None)
    ],
    'B10': [
        Function('VBCS', BitsEqual(0x70, [5], 0x1)),
        Function('LRST#', BitsEqual(0x70, [23], 0x1)),
        Function('GPIOB4', None)
    ],
    'B11': [
        Function('INTA#', BitsEqual(0x78, [4], 0x0)),
        Function('GPIOB0', None)
    ],
    'B12': [
        Function('SCL7', BitsEqual(0x74, [14], 0x1)),
        Function('GPIOH3', None)
    ],
    'B13': [
        Function('MII2DC', BitsEqual(0x74, [20], 0x1)),
        Function('SCL5', BitsEqual(0x74, [12], 0x1)),
        Function('GPIOC7', None)
    ],
    'B2': [
        Function('DDCADAT', BitsEqual(0x74, [18], 0x1)),
        Function('GPIOD6', None)
    ],
    'B8': [
        Function('PWM3', BitsEqual(0x74, [10], 0x1)),
        Function('GPIOC4', None)
    ],
    'B9': [
        Function('PECII', BitsEqual(0x74, [7], 0x1)),
        Function('GPIOC0', None)
    ],
    'C11': [
        Function('PHYPD#', BitsEqual(0x74, [25], 0x1)),
        Function('GPIOA5', None)
    ],
    'C12': [
        Function('SDA6', BitsEqual(0x74, [13], 0x1)),
        Function('GPIOH0', None)
    ],
    'C8': [
        Function('PWM2', BitsEqual(0x74, [9], 0x1)),
        Function('GPIOC3', None)
    ],
    'C9': [
        Function('VBDI', BitsEqual(0x70, [5], 0x1)),
        Function('GPIOB7', None)
    ],
    'D10': [
        Function('FLWP#', BitsEqual(0x74, [2], 0x1)),
        Function('GPIOB2', None)
    ],
    'D11': [
        Function('PHYLINK', BitsEqual(0x74, [25], 0x1)),
        Function('GPIOA4', None)
    ],
    'D12': [
        Function('SCL6', BitsEqual(0x74, [13], 0x1)),
        Function('GPIOH1', None)
    ],
    'D8': [
        Function('PWM1', BitsEqual(0x74, [8], 0x1)),
        Function('GPIOC2', None)
    ],
    'D9': [
        Function('VBDO', BitsEqual(0x70, [5], 0x1)),
        Function('WDTRST', BitsEqual(0x78, [3], 0x1)),
        Function('GPIOB6', None)
    ],
    'R1': [
        Function('VP2', BitsEqual(0x74, [22], 0x1)),
        Function('GPIOE2', None)
    ],
    'R2': [
        Function('VP1', BitsEqual(0x74, [22], 0x1)),
        Function('GPIOE1', None)
    ],
    'R3': [
        Function('VP0', BitsEqual(0x74, [22], 0x1)),
        Function('GPIOE0', None)
    ],
    'R4': [
        Function('VSYNC', BitsEqual(0x74, [15], 0x1)),
        Function('VPAVSYNC', BitsEqual(0x74, [16], 0x1)),
        Function('GPIOH5', None)
    ],
    'T1': [
        Function('VPACLK', BitsEqual(0x74, [16], 0x1)),
        Function('GPIOH7', None)
    ],
    'T2': [
        Function('VP5', BitsEqual(0x74, [22], 0x1)),
        Function('GPIOE5', None)
    ],
    'T3': [
        Function('VP4', BitsEqual(0x74, [22], 0x1)),
        Function('GPIOE4', None)
    ],
    'T4': [
        Function('VP3', BitsEqual(0x74, [22], 0x1)),
        Function('GPIOE3', None)
    ],
    'U1': [
        Function('VPADE', BitsEqual(0x74, [16], 0x1)),
        Function('GPIOH6', None)
    ],
    'U2': [
        Function('HSYNC', BitsEqual(0x74, [15], 0x1)),
        Function('VPAHSYNC', BitsEqual(0x74, [16], 0x1)),
        Function('GPIOH4', None)
    ],
    'U3': [
        Function('VP7', BitsEqual(0x74, [22], 0x1)),
        Function('GPIOE7', None)
    ],
    'U4': [
        Function('VP6', BitsEqual(0x74, [22], 0x1)),
        Function('GPIOE6', None)
    ],
    'V1': [
        Function('VP11', BitsEqual(0x74, [22], 0x1)),
        Function('GPIOF3', None)
    ],
    'V2': [
        Function('VP10', BitsEqual(0x74, [22], 0x1)),
        Function('GPIOF2', None)
    ],
    'V3': [
        Function('VP9', BitsEqual(0x74, [22], 0x1)),
        Function('GPIOF1', None)
    ],
    'V4': [
        Function('VP8', BitsEqual(0x74, [22], 0x1)),
        Function('GPIOF0', None)
    ],
    'W1': [
        Function('VP15', BitsEqual(0x74, [23], 0x1)),
        Function('GPIOF7', None)
    ],
    'W2': [
        Function('VP14', BitsEqual(0x74, [23], 0x1)),
        Function('GPIOF6', None)
    ],
    'W3': [
        Function('VP13', BitsEqual(0x74, [23], 0x1)),
        Function('GPIOF5', None)
    ],
    'W4': [
        Function('VP12', BitsEqual(0x74, [23], 0x1)),
        Function('GPIOF4', None)
    ],
    'Y3': [
        Function('VP17', BitsEqual(0x74, [23], 0x1)),
        Function('GPIOG1', None)
    ],
    'Y4': [
        Function('VP16', BitsEqual(0x74, [23], 0x1)),
        Function('GPIOG0', None)
    ],
}
