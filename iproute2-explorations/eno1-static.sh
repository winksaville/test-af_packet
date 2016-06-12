#!/bin/bash
# Some docs for iproute2 see:
# [biproute2 cheat sheet - Daniil Baturi](http://baturin.org/docs/iproute2/)
# [LInux Advanced Routing & Traffic Control HOWTO](http://lartc.org/howto/index.html)
set -x

# Flush current settings for eno1
./eno1-flush.sh

# Add static address
# From [cyberciti.biz ip examples](http://www.cyberciti.biz/faq/linux-ip-command-examples-usage-syntax/#4)
sudo ip addr add 192.168.0.90/24 brd + dev eno1

# Add default routeh
# See [gentoo docs](https://wiki.gentoo.org/wiki/Iproute2)
sudo ip route add default via 192.168.0.1

# Add static route
sudo ip route add 192.168.0.0/24 metric 100 scope global dev eno1

# Display results
sudo ip addr show ; ip route show
