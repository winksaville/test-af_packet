#!/bin/bash
# Some docs for iproute2 see:
# [biproute2 cheat sheet - Daniil Baturi](http://baturin.org/docs/iproute2/)
# [LInux Advanced Routing & Traffic Control HOWTO](http://lartc.org/howto/index.html)
set -x

# flush current settings
./flush.sh eno1

# Toggle eno1 down/up which causes dhcp to get new haddresses
sudo ip link set eno1 down
sudo ip link set eno1 up
sleep 3

# Display results
sudo ip addr show ; ip route show
