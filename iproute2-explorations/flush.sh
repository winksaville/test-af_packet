#!/bin/bash
# flush a interface/dev param1 is the interface
set -x

# Flush current settings for eno1
# I got this from [arch linux Network Config](https://wiki.archlinux.org/index.php/Network_configuration)
sudo ip addr flush dev $1
sudo ip route flush dev $1
