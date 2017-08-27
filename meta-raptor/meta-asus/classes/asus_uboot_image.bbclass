inherit image_types_uboot

# 24M
IMAGE_ROOTFS_SIZE = "24576"
# and don't put overhead behind my back
IMAGE_OVERHEAD_FACTOR = "1"

IMAGE_PREPROCESS_COMMAND += " generate_data_mount_dir ; "
IMAGE_POSTPROCESS_COMMAND += " flash_image_generate ; "

FLASH_IMAGE_NAME ?= "flash-${MACHINE}-${DATETIME}"
FLASH_IMAGE_LINK ?= "flash-${MACHINE}"
# 16M
FLASH_SIZE ?= "16384"
FLASH_UBOOT_OFFSET ?= "0"
# 512k
FLASH_KERNEL_OFFSET ?= "512"
# 2.4M
FLASH_ROOTFS_OFFSET ?= "2304"

flash_image_generate() {
  kernelfile="${DEPLOY_DIR_IMAGE}/${KERNEL_IMAGETYPE}"
  ubootfile="${DEPLOY_DIR_IMAGE}/u-boot.${UBOOT_SUFFIX}"
  # rootfs has to match the type defined in IMAGE_FSTYPES"
  rootfs="${DEPLOY_DIR_IMAGE}/${IMAGE_LINK_NAME}.squashfs"
  if [ ! -f $kernelfile ]; then
    echo "Kernel file ${kernelfile} does not exist"
    return 1
  fi
  if [ ! -f $ubootfile ]; then
    echo "U-boot file ${ubootfile} does not exist"
    return 1
  fi
  if [ ! -f $rootfs ]; then
    echo "Rootfs file ${rootfs} does not exist"
    return 1
  fi
  dst="${DEPLOY_DIR_IMAGE}/${FLASH_IMAGE_NAME}"
  rm -rf $dst
  dd if=/dev/zero bs=1k count=${FLASH_SIZE} | tr "\000" "\377" > ${dst}
  dd if=${ubootfile} of=${dst} bs=1k seek=${FLASH_UBOOT_OFFSET} conv=notrunc
  dd if=${kernelfile} of=${dst} bs=1k seek=${FLASH_KERNEL_OFFSET} conv=notrunc
  dd if=${rootfs} of=${dst} bs=1k seek=${FLASH_ROOTFS_OFFSET} conv=notrunc
  dstlink="${DEPLOY_DIR_IMAGE}/${FLASH_IMAGE_LINK}"
  rm -rf $dstlink
  ln -sf ${FLASH_IMAGE_NAME} $dstlink
}

generate_data_mount_dir() {
  mkdir -p "${IMAGE_ROOTFS}/mnt/data"
}
