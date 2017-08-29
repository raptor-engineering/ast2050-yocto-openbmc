#/bin/bash
#
# Copyright 2017 Raptor Engineering, LLC
#
# Work around glitchy timer0 on the AST2050
# For an unknown reason, sometimes when timer0
# expires it does not fire an IRQ.  This in turn
# causes Linux to hang waiting for the timer to
# expire, causing all nanosleep calls to stall
# indefinitely.  On a stock OpenBMC system, this
# manifests as a spontanous reboot every few hours
# of uptime as the watchdog timer kicks in.
#
# This script works around the problem by watching
# timer0 and seeing if it is stuck at 0.  If so, it
# gives the afflicted timer a good kick in the rear
# to get it going again, which in turn un-stalls all
# of the queued nanosleep calls.
#
# Note that due to what this does and how it operates,
# using sleep in this script is a VERY BAD IDEA!

function daemon_process {
	while [[ 1 == 1 ]]; do
		TIMER_COUNT=$(devmem 0x1e782000 32)
		if [[ "$TIMER_COUNT" == "0x00000000" ]]; then
			TIMER_COUNT=$(devmem 0x1e782000 32)
			if [[ "$TIMER_COUNT" == "0x00000000" ]]; then
				echo "Kicking timer back into operation!"
				devmem 0x1e782004 32 0x00000001
			fi
		fi

		# Busy wait at highest niceness
		COUNTER=1000
		until [[ $COUNTER -lt 1 ]]; do
			let COUNTER-=1
		done
	done
}

renice -n 19 $$

daemon_process < /dev/null &
disown
