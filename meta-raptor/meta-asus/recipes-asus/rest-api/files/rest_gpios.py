#!/usr/bin/env python
#
# Copyright 2017 Raptor Engineering, LLC
# Copyright 2014-present Facebook. All Rights Reserved.
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
#

from rest_fruid import get_fruid

MAINBOARDS = ["KGPE-D16", "KCMA-D8"]


def read_gpio_sysfs(gpio):
    with open('/sys/class/gpio/gpio%d/value' % gpio, 'r') as f:
        val_string = f.read()
        if val_string == '1\n':
            return 1
        if val_string == '0\n':
            return 0
    return None


def read_asus_gpio():
    bhinfo = {}
    return bhinfo


def get_gpios():
    fruinfo = get_fruid()
    gpioinfo = {}
    if fruinfo["Information"]["Product Name"] in MAINBOARDS:
        gpioinfo["back_ports"] = read_asus_gpio()
    return gpioinfo
