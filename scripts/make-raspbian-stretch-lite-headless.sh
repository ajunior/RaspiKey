#!/bin/bash

# Creates Raspbian modified base image with headless setup script support in /boot partition
#

set -ev

script_path=$(readlink -f "$0")
script_dir=$(readlink -f `dirname $0`)

if [ $(id -u) -ne 0 ]; then
	echo "$script_path must be run as root."
	exit 1
fi

src_url=https://downloads.raspberrypi.org/raspbian_lite/images/raspbian_lite-2018-11-15/2018-11-13-raspbian-stretch-lite.zip
src_img_name=2018-11-13-raspbian-stretch-lite
rootpath=./root
bootpath=./boot

# Retrieve official source image
if [ ! -f "${src_img_name}.zip" ]; then 
	wget $src_url
fi
unzip -o "${src_img_name}.zip"

# Mount source image
mkdir $rootpath
mkdir $bootpath
losetup -P /dev/loop99 "${src_img_name}.img"
mount /dev/loop99p1 $bootpath
mount /dev/loop99p2 $rootpath

# Modify /etc/rc.local to run /boot/start.sh
sed -i 's/^.*exit 0.*$//g' $rootpath/etc/rc.local
cat <<EOT >> $rootpath/etc/rc.local
if [ -f /boot/start.sh ]; then
	/boot/start.sh
fi

exit 0
EOT

# Disable Raspbian sdcard autoresize on first boot
sed -i 's/init=[^ ]*//g' $bootpath/cmdline.txt
if [ -f $rootpath/etc/init.d/resize2fs_once ]; then
    rm $rootpath/etc/init.d/resize2fs_once
fi
if [ -f $rootpath/etc/rc3.d/S01resize2fs_once ]; then
    rm $rootpath/etc/rc3.d/S01resize2fs_once
fi

# Unmount source image
umount $bootpath
umount $rootpath
losetup -d /dev/loop99
rmdir $bootpath
rmdir $rootpath

# Compress to new image
xz -e -9 -c ${src_img_name}.img | pv > ${src_img_name}-headless.xz
rm ${src_img_name}.img



