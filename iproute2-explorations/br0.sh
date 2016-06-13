#!/bin/bash
# Some docs for iproute2 see:
# [biproute2 cheat sheet - Daniil Baturi](http://baturin.org/docs/iproute2/)
# [LInux Advanced Routing & Traffic Control HOWTO](http://lartc.org/howto/index.html)
set -x

# Flush current settings for br0
#./flush.sh br0
#./flush.sh eno1
#./flush.sh tap0

sudo ip address show eno1 1> /dev/null 2>&1
if [ $? != 0 ]; then
  echo "no eno1" ; exit 1
else
  ./flush.sh eno1
fi

# create tap0 if it doesn't exist else flush it
sudo ip address show tap0 1> /dev/null 2>&1
if [ $? != 0 ]; then
  sudo ip tuntap add name tap0 mode tap
  if [ $? != 0 ]; then
    echo "could not create tap0"
    exit 1
  fi
else
  ./flush.sh tap0
fi

# create br0 if it doesn't exist else flush it
sudo ip address show br0 1> /dev/null 2>&1
if [ $? != 0 ]; then
  sudo ip link add name br0 type bridge
  if [ $? != 0 ]; then
    echo "could not create br0"
    exit 1
  fi
else
  ./flush.sh br0
fi

# Add the interfaces
sudo brctl addif br0 eno1
sudo brctl addif br0 tap0

# bring up the interfaces and bridge
sudo ip link set eno1 up
sudo ip link set tap0 up
sudo ip link set br0 up

# Add static address to the bridge
# From [cyberciti.biz ip examples](http://www.cyberciti.biz/faq/linux-ip-command-examples-usage-syntax/#4)
sudo ip addr add 192.168.0.90/24 brd + dev br0

# Add default route
# See [gentoo docs](https://wiki.gentoo.org/wiki/Iproute2)
sudo ip route add default via 192.168.0.1

# Add static route
sudo ip route add 192.168.0.0/24 metric 100 scope global dev eno1

# Display results
sudo ip address show ; ip route show
