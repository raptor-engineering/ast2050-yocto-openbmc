#!/bin/bash
#
# Copyright 2017 Raptor Engineering, LLC
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

. /usr/local/bin/openbmc-utils.sh

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin

# Blink the BMC status LED
while true; do
    gpio_set LED_STA_BMC_N 0
    usleep 50000		# 50ms
    gpio_set LED_STA_BMC_N 1
    usleep 100000		# 100ms
    gpio_set LED_STA_BMC_N 0
    usleep 50000		# 50ms
    gpio_set LED_STA_BMC_N 1
    usleep 700000		# 800ms
done
