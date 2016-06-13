#!/bin/bash
# Some docs for iproute2 see:
# [biproute2 cheat sheet - Daniil Baturi](http://baturin.org/docs/iproute2/)
# [LInux Advanced Routing & Traffic Control HOWTO](http://lartc.org/howto/index.html)
set -x

# flush current settings for eno1
./flush.sh eno1

sudo ip address show eno1 1> /dev/null 2>&1
if [ $? != 0 ]; then
  echo "no eno1" ; exit 1
else
  ./flush.sh eno1
fi

# Delete tap0 if it exits
sudo ip address show tap0 1> /dev/null 2>&1
if [ $? == 0 ]; then
  ./flush.sh tap0
  sudo ip link set tap0 down
  sudo ip tuntap del name tap0 mode tap
fi

# Delete br0 if it exits
sudo ip address show br0 1> /dev/null 2>&1
if [ $? == 0 ]; then
  ./flush.sh br0
  sudo ip link set br0 down
  sudo brctl delbr br0
fi

# Toggle eno1 down/up which causes dhcp to get new haddresses
sudo ip link set eno1 down
sudo ip link set eno1 up
sleep 3

# Display results
sudo ip addr show ; ip route show
