#!/bin/bash

set -e # Stop further execution on error

scriptPath=$(readlink -f "$0")
scriptDir=$(readlink -f `dirname $0`)

if [ $(id -u) -ne 0 ]; then
	echo "$scriptPath must be run as root."
	exit 1
fi

echo "Running $scriptPath..."

if [ -z "$1" ]; then
    echo "Missing target block device argument. E.g. $0 /dev/sdb"
    exit 1
fi

devname=$1
bootpath=./mnt/boot

# Zero vfat partition's free space
#
echo "Zeroing ${devname}1..."
mkdir -p $bootpath
mount ${devname}1 $bootpath
{ # try
    echo "Filling ${devname}1 free space..."
    cat /dev/zero > $bootpath/big_zero
} || { # catch
    echo "Deleting ${devname}1 free space..."
    sync; rm $bootpath/big_zero
}
umount $bootpath
rmdir $bootpath

# Zero ext4 partitions' free space
#
echo "Zeroing ${devname}2..."
zerofree ${devname}2

# Create compressed image
#
echo "Extracting compressed image..."
# 3637248 is the last block of the last partition + 1
dd if=${devname} bs=512 count=3637248 | pv | xz -e -9 -c - > raspikey.xz

sync $devname

echo "Done!"







