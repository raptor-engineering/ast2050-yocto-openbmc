require u-boot.inc

LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://COPYING;md5=1707d6db1d42237583f50183a5651ecb \
                    file://README;beginline=1;endline=22;md5=78b195c11cb6ef63e6985140db7d7bab"

# We use the revision in order to avoid having to fetch it from the repo during parse
SRCREV = "537b8ccdaff698607f6dd56b988df4439de6116f"

PV = "v2013.07"

SRC_URI = "git://github.com/raptor-engineering/ast2050-uboot.git;protocol=https;branch=master \
           file://fw_env.config \
          "

S = "${WORKDIR}/git"

PACKAGE_ARCH = "${MACHINE_ARCH}"
