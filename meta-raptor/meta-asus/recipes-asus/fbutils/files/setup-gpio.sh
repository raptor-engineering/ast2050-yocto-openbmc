#!/bin/bash
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
# Provides:          gpio-setup
# Required-Start:
# Required-Stop:
# Default-Start:     S
# Default-Stop:
# Short-Description:  Set up GPIO pins as appropriate
### END INIT INFO

# This file contains definitions for the GPIO pins that were not otherwise
# defined in other files.  We should probably move some more of the
# definitions to this file at some point.

# The commented-out sections are generally already defined elsewhere,
# and defining them twice generates errors.

. /usr/local/bin/openbmc-utils.sh

# echo 4 > /sys/class/gpio/export
# echo 9 > /sys/class/gpio/export
# echo 14 > /sys/class/gpio/export
# echo 33 > /sys/class/gpio/export
# echo 34 > /sys/class/gpio/export
# echo 35 > /sys/class/gpio/export
# echo 40 > /sys/class/gpio/export
# echo 42 > /sys/class/gpio/export
# echo 44 > /sys/class/gpio/export
# echo 45 > /sys/class/gpio/export
# echo 48 > /sys/class/gpio/export
# echo 49 > /sys/class/gpio/export
# echo 58 > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio4/direction
echo "out" > /sys/class/gpio/gpio9/direction
echo "out" > /sys/class/gpio/gpio14/direction
echo "out" > /sys/class/gpio/gpio33/direction
echo "out" > /sys/class/gpio/gpio34/direction
echo "out" > /sys/class/gpio/gpio35/direction
echo "out" > /sys/class/gpio/gpio40/direction
echo "out" > /sys/class/gpio/gpio44/direction
echo "out" > /sys/class/gpio/gpio45/direction
echo "out" > /sys/class/gpio/gpio48/direction
echo "in"  > /sys/class/gpio/gpio49/direction
echo "in"  > /sys/class/gpio/gpio58/direction

# Once we set "out", output values will be random unless we set them
# to something

echo "1" > /sys/class/gpio/gpio4/value
echo "1" > /sys/class/gpio/gpio9/value
echo "1" > /sys/class/gpio/gpio14/value
echo "1" > /sys/class/gpio/gpio33/value
echo "1" > /sys/class/gpio/gpio34/value
echo "1" > /sys/class/gpio/gpio35/value
echo "1" > /sys/class/gpio/gpio40/value
echo "1" > /sys/class/gpio/gpio44/value
echo "1" > /sys/class/gpio/gpio45/value
echo "1" > /sys/class/gpio/gpio48/value
