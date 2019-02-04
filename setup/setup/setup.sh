#!/bin/bash

set -e

scriptPath=$(readlink -f "$0")
scriptDir=$(readlink -f `dirname $0`)

if [ $(id -u) -ne 0 ]; then
	echo "$scriptPath must be run as root."
	exit 1
fi

# Turn on RPi led permanently so that we know when it shut down at the end
echo 0 | sudo tee /sys/class/leds/led0/brightness > /dev/null

echo "Running $scriptPath..."

# Error handling
error()
{
  local parent_lineno="$1"
  local message="$2"
  local code="${3:-1}"
  if [[ -n "$message" ]] ; then
    echo "$scriptPath error on or near line ${parent_lineno}: ${message}; exiting with status ${code}"
  else
    echo "$scriptPath error on or near line ${parent_lineno}; exiting with status ${code}"
  fi

  exit "${code}" # Exit without shutdown (error occured)
}
trap 'error ${LINENO}' ERR

# Set hostname to raspikey
echo "Configuring hostname..."
echo "raspikey" | cat > /etc/hostname
sed -Ei 's/^127\.0\.1\.1.*$/127.0.1.1\traspikey/' /etc/hosts

# Minimise the amount of RAM used by the GPU
echo "gpu_mem=32" | cat >> /boot/config.txt

# Disable ipv6
cat <<EOT >> /etc/sysctl.d/99-sysctl.conf

net.ipv6.conf.all.disable_ipv6 = 1
net.ipv6.conf.default.disable_ipv6 = 1
net.ipv6.conf.lo.disable_ipv6 = 1
EOT

# Configure raspikey_usb.service and libcomposite device to be available on first boot
echo "Configuring raspikey libcomposite device..."
cp $scriptDir/raspikey_usb /usr/sbin/raspikey_usb
chmod +x /usr/sbin/raspikey_usb
cp $scriptDir/raspikey_usb.service /etc/systemd/system/
systemctl enable raspikey_usb.service
echo "dtoverlay=dwc2" | cat >> /boot/config.txt
echo "dtoverlay=pi3-disable-wifi" | cat >> /boot/config.txt
echo "dwc2" | cat >> /etc/modules
echo "libcomposite" | cat >> /etc/modules

# Configure the raspikey.service
echo "Configuring raspikey.service..."
cp $scriptDir/raspikey.service /etc/systemd/system/
systemctl enable raspikey.service
mkdir /raspikey
cp -r $scriptDir/html /raspikey
cp $scriptDir/raspikey /raspikey
chmod ug+x /raspikey/raspikey

# Configure the raspikey_datafs.service
echo "Configuring raspikey_datafs.service..."
cp $scriptDir/raspikey_datafs.service /etc/systemd/system/
systemctl enable raspikey_datafs.service

# Setup tmpfs /data filesystem
mkdir /data
echo -e "\ntmpfs /data tmpfs nodev,nosuid 0 0" | cat >> /etc/fstab
mount /data

# Redirect the standard /var/lib/bluetooth data dir to /data
service bluetooth stop
[ ! -d /var/lib/bluetooth ] || rm -fr /var/lib/bluetooth
ln -s /data /var/lib/bluetooth

# Enable serial console login
#systemctl enable getty@ttyGS0.service

# Disable boot waiting for dhcpd (as it delays boot)
rm /etc/systemd/system/dhcpcd.service.d/wait.conf

# Delete uneeded files from boot partition
rm /boot/kernel7.img
rm /boot/bcm2708-rpi-b-plus.dtb
rm /boot/bcm2708-rpi-b.dtb
rm /boot/bcm2708-rpi-cm.dtb
rm /boot/bcm2709-rpi-2-b.dtb
rm /boot/bcm2710-rpi-3-b-plus.dtb
rm /boot/bcm2710-rpi-3-b.dtb
rm /boot/bcm2710-rpi-cm3.dtb

# Uninstall unneeded packages
echo "Uninstalling packages..."
apt-get remove --purge --yes build-essential gcc-6  g++-6  cpp-6 libc6-dbg libc6-dev  perl-modules-5.24 libpython2.7-stdlib  python3.5-minimal libperl5.24 libpython3.5-stdlib libpython3.5 libpython3.5-minimal python2.7-minimal libraspberrypi-dev libpython2.7-minimal manpages-dev gcc-4.8-base gcc-4.9-base gcc-4.6-base libraspberrypi-doc xkb-data libmnl-dev geoip-database samba-common man-db manpages wget nfs-common raspi-config
apt-get --yes autoremove 
apt-get --yes clean

# Disable unneeded services
systemctl disable wifi-country.service
systemctl disable apt-daily-upgrade.timer
systemctl disable apt-daily.timer
systemctl disable networking.service

# Convert system to readonly
echo "Converting to read-only..."
/bin/bash $scriptDir/readonly-convert.sh

# Delete various temporary files
echo "Deleting temporary and setup files..."
swapoff -a
rm /var/swap
rm -fr /opt

# Delete the setup files
rm -fr /boot/setup || true
rm -f /boot/start.sh || true

# Clear history
history -c

echo "Shutting down..."
shutdown -h now








