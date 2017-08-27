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

### BEGIN INIT INFO
# Provides:          power-on
# Required-Start:
# Required-Stop:
# Default-Start:     S
# Default-Stop:
# Short-Description: Power on micro-server
### END INIT INFO
. /usr/local/bin/openbmc-utils.sh

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin

# set power and reset lines high to prepare for power on sequence
gpio_set CTL_REQ_POWERUP_N 1
gpio_set CTL_REQ_POWERDOWN_N 1
gpio_set CTL_REQ_RESET_N 1

# release control lockout
gpio_set ASUS_BMC_CTL_LOCKOUT_N 1

# power up
asus_power_on_board

echo -n "Checking mainboard power status ... "
if asus_is_us_on 10 "."; then
    echo "on"
    on=1
else
    echo "off"
    on=0
fi

if [ $on -eq 0 ]; then
    # Power on now
    asus_power.sh on -f
fi
