#!/bin/sh
#
# Copyright 2017 Raptor Engineering, LLC
# Copyright 2004-present Facebook. All Rights Reserved.
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
PWM_DIR=/sys/class/i2c-adapter/i2c-1/1-002f

set -e

# On ASUS boards, there are up to 8 fans connected.
# Each fan has one PWM input and 1 tachometer output.
# For each fan, set the type, and set initial speed to 100%
for pwm in 1 2 3 4 5 6 7 8; do
    echo 1 > $PWM_DIR/pwm${pwm}_enable
    echo 255 > $PWM_DIR/pwm${pwm}
done
