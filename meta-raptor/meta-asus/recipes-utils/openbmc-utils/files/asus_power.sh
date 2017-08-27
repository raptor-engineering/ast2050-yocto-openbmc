#!/bin/bash
#
# Copyright 2017 Raptor Engineering, LLC
# Copyright 2015-present Facebook. All Rights Reserved.
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

. /usr/local/bin/openbmc-utils.sh

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin

prog="$0"

PWR_BTN_GPIO="BMC_PWR_BTN_OUT_N"
PWR_SYSTEM_SYSFS="${SYSCPLD_SYSFS_DIR}/pwr_cyc_all_n"
PWR_USRV_RST_SYSFS="${SYSCPLD_SYSFS_DIR}/usrv_rst_n"

usage() {
    echo "Usage: $prog <command> [command options]"
    echo
    echo "Commands:"
    echo "  status: Get the current microserver power status"
    echo
    echo "  on: Power on microserver if not powered on already"
    echo "    options:"
    echo "      -f: Re-do power on sequence no matter if microserver has "
    echo "          been powered on or not."
    echo
    echo "  off: Power off microserver ungracefully"
    echo
    echo "  reset: Power reset microserver ungracefully"
    echo "    options:"
    echo "      -s: Power reset whole wedge system ungracefully"
    echo
}

do_status() {
    echo -n "Mainboard power is "
    if asus_is_us_on; then
        echo "on"
    else
        echo "off"
    fi
    return 0
}

do_gpio_power_on() {
    gpio_set CTL_REQ_POWERDOWN_N 1
    gpio_set CTL_REQ_RESET_N 0
    gpio_set CTL_REQ_POWERUP_N 0
    sleep 1
    gpio_set CTL_REQ_RESET_N 1
    gpio_set CTL_REQ_POWERUP_N 1
    return 0
}

do_on() {
    local force opt ret
    force=0
    while getopts "f" opt; do
        case $opt in
            f)
                force=1
                ;;
            *)
                usage
                exit -1
                ;;

        esac
    done
    echo -n "Power on mainboard ..."
    if [ $force -eq 0 ]; then
        # need to check if uS is on or not
        if asus_is_us_on 10 "."; then
            echo " Already on. Skip!"
            return 1
        fi
    fi

    # power on sequence
    do_gpio_power_on
    ret=$?
    if [ $ret -eq 0 ]; then
        echo " Done"
    else
        echo " Failed"
    fi
    return $ret
}

do_gpio_power_off() {
    gpio_set CTL_REQ_RESET_N 1
    gpio_set CTL_REQ_POWERUP_N 1
    gpio_set CTL_REQ_POWERDOWN_N 0
    sleep 1
    gpio_set CTL_REQ_POWERDOWN_N 1
    return 0
}

do_off() {
    local ret
    echo -n "Power off mainboard ..."
    do_gpio_power_off
    ret=$?
    if [ $ret -eq 0 ]; then
        echo " Done"
    else
        echo " Failed"
    fi
    return $ret
}

do_reset() {
    local system opt pulse_us
    system=0
    while getopts "s" opt; do
        case $opt in
            s)
                system=1
                ;;
            *)
                usage
                exit -1
                ;;
        esac
    done
    if [ $system -eq 1 ]; then
        echo -n "Power reset whole system ..."
        do_gpio_power_off
        sleep 1
        do_gpio_power_on
    else
        if ! asus_is_us_on; then
            echo "Power resetting mainboard that is powered off has no effect."
            echo "Use '$prog on' to power the mainboard on"
            return -1
        fi
        echo -n "Power reset microserver ..."
        gpio_set CTL_REQ_POWERDOWN_N 1
        gpio_set CTL_REQ_RESET_N 0
        gpio_set CTL_REQ_POWERUP_N 1
        sleep 1
        gpio_set CTL_REQ_RESET_N 1
    fi
    echo " Done"
    return 0
}

if [ $# -lt 1 ]; then
    usage
    exit -1
fi

command="$1"
shift

case "$command" in
    status)
        do_status $@
        ;;
    on)
        do_on $@
        ;;
    off)
        do_off $@
        ;;
    reset)
        do_reset $@
        ;;
    *)
        usage
        exit -1
        ;;
esac

exit $?
