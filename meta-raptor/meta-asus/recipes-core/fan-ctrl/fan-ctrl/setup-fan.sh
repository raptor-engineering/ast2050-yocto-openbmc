#!/bin/sh
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

### BEGIN INIT INFO
# Provides:          setup-fan
# Required-Start:    board-id
# Required-Stop:
# Default-Start:     S
# Default-Stop:
# Short-Description: Set fan speed
### END INIT INFO

echo -n "Start AST2050 timer workaround..."
/usr/local/bin/ast2050-timer-workaround.sh > /dev/null
echo "done".

echo -n "Setup fan speed... "
rmmod w83795 || true
/usr/local/bin/platform_sensor_init
modprobe w83795 force_w83795=1,0x2f
/usr/local/bin/init_pwm.sh
/usr/local/bin/set_fan_speed.sh 50
/usr/local/bin/fand
echo "done."
