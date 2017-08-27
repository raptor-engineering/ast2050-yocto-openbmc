
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://asus.conf \
           "

do_install_board_config() {
    install -d ${D}${sysconfdir}/sensors.d
    install -m 644 ../asus.conf ${D}${sysconfdir}/sensors.d/asus.conf
}
