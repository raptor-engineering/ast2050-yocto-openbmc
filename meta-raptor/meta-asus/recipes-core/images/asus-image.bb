inherit asus_uboot_image

# /dev
require recipes-core/images/aspeed-dev.inc

# Base this image on core-image-minimal
include recipes-core/images/core-image-minimal.bb

# Changing the image compression from gz to lzma achieves 30% saving (~3M).
# However, the current u-boot does not have lzma enabled. Stick to gz
# until we generate a new u-boot image.
IMAGE_FSTYPES += "squashfs"

PYTHON_PKGS = " \
  python-core \
  python-io \
  python-json \
  python-shell \
  python-subprocess \
  python-argparse \
  python-ctypes \
  python-datetime \
  python-email \
  python-threading \
  python-mime \
  python-pickle \
  python-misc \
  python-netserver \
  "

NTP_PKGS = " \
  ntp \
  ntp-utils \
  sntp \
  ntpdate \
  "

# Include modules in rootfs
IMAGE_INSTALL += " \
  kernel-modules \
  u-boot \
  u-boot-fw-utils \
  openbmc-utils \
  openbmc-gpio \
  fan-ctrl \
  watchdog-ctrl \
  i2c-tools \
  sensor-setup \
  lldp-util \
  lmsensors-sensors \
  sms-kcsd \
  rest-api \
  bottle \
  ipmid \
  po-eeprom \
  bitbang \
  ${PYTHON_PKGS} \
  ${NTP_PKGS} \
  iproute2 \
  dhcp-client \
  jbi \
  flashrom \
  cherryPy \
  screen \
  iptables \
  "

IMAGE_FEATURES += " \
  ssh-server-openssh \
  "

DISTRO_FEATURES += " \
  ext2 \
  nfs \
  usbgadget \
  "

# IPv6 deactivated due to lack of memory.  If you need IPv6 please deactivate IPv4
# DISTRO_FEATURES += " ipv6"
disable_ipv6() {
	rm -f ${IMAGE_ROOTFS}/etc/network/if-up.d/dhcpv6_*
}
ROOTFS_POSTPROCESS_COMMAND += "disable_ipv6;"

# Flash space is limited and PIP will not be run
# However, it does not appear there is a way to prevent ensurepip from being installed,
# so simply delete it before the rootfs is packaged / compressed...
# Same goes for the various Python module test suites...
# ...and tutorial PDFs...
remove_spurious_files() {
	rm -rf ${IMAGE_ROOTFS}/usr/lib/python2.7/ensurepip
	rm -rf ${IMAGE_ROOTFS}/usr/lib/python2.7/ctypes/test
	rm -rf ${IMAGE_ROOTFS}/usr/lib/python2.7/email/test
	rm -rf ${IMAGE_ROOTFS}/usr/lib/python2.7/json/tests
	rm -rf ${IMAGE_ROOTFS}/usr/lib/python2.7/site-packages/cherrypy/test
	rm -rf ${IMAGE_ROOTFS}/usr/share/cherrypy/tutorial/
}

ROOTFS_POSTPROCESS_COMMAND += "remove_spurious_files;"

# Our root filesystem is read-only, so some directories need to be created before the
# rootfs is flashed onto the system
create_missing_directories() {
	mkdir ${IMAGE_ROOTFS}/mnt/data
}

ROOTFS_POSTPROCESS_COMMAND += "create_missing_directories;"
