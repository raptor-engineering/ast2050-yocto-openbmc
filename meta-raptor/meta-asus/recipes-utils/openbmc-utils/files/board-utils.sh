# Copyright 2017 Raptor Engineering, LLC
# Copyright 2015-present Facebook. All Rights Reserved.

SLOT_ID=1

asus_is_us_on() {
    val=$(gpio_get STA_LINE_POWER)
    if [ "$val" == "1" ]; then
        return 0            # powered on
    else
        return 1
    fi
}

asus_board_type() {
    echo 'ASUS ASMB4 Platform'
}

asus_slot_id() {
    printf "%d\n" $SLOT_ID
}

asus_board_rev() {
    echo 0
}

# Should we enable OOB interface or not
asus_should_enable_oob() {
    # ASUS uses BMC MAC
    return -1
}

asus_power_on_board() {
    local val
    val=$(asus_is_us_on)
    if [[ "$val" == "1" ]]; then
        # power on
        gpio_set CTL_REQ_POWERDOWN_N 1
        gpio_set CTL_REQ_RESET_N 0
        gpio_set CTL_REQ_POWERUP_N 0
        sleep 1
        gpio_set CTL_REQ_RESET_N 1
        gpio_set CTL_REQ_POWERUP_N 1
    fi
}
