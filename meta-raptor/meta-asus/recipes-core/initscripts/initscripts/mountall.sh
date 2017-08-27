#!/bin/sh
### BEGIN INIT INFO
# Provides:          mountall
# Required-Start:    mountvirtfs
# Required-Stop: 
# Default-Start:     S
# Default-Stop:
# Short-Description: Mount all filesystems.
# Description:
### END INIT INFO

. /etc/default/rcS

#
# Mount local filesystems in /etc/fstab. For some reason, people
# might want to mount "proc" several times, and mount -v complains
# about this. So we mount "proc" filesystems without -v.
#
test "$VERBOSE" != no && echo "Mounting local filesystems..."
mount -at nonfs,nosmbfs,noncpfs 2>/dev/null
mount /tmp

# Create unionfs

mkdir /tmp/unionfs
mkdir /tmp/unionfs/dev
mkdir /tmp/unionfs/etc
mkdir /tmp/unionfs/var
mkdir /tmp/unionfs/media

umount /dev/pts

mount -t unionfs -o dirs=/tmp/unionfs/dev=rw:/dev=ro none /dev
mount -t unionfs -o dirs=/tmp/unionfs/etc=rw:/etc=ro none /etc
mount -t unionfs -o dirs=/tmp/unionfs/var=rw:/var=ro none /var
mount -t unionfs -o dirs=/tmp/unionfs/media=rw:/media=ro none /media

mount -t devpts devpts /dev/pts

#
# We might have mounted something over /dev, see if /dev/initctl is there.
#
if test ! -p /dev/initctl
then
	rm -f /dev/initctl
	mknod -m 600 /dev/initctl p
fi
kill -USR1 1

#
# Execute swapon command again, in case we want to swap to
# a file on a now mounted filesystem.
#
swapon -a 2> /dev/null

: exit 0

