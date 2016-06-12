#!/bin/bash
# Some docs for iproute2 see:
# [biproute2 cheat sheet - Daniil Baturi](http://baturin.org/docs/iproute2/)
# [LInux Advanced Routing & Traffic Control HOWTO](http://lartc.org/howto/index.html)
set -x

# Flush current settings for eno1
# I got this from [arch linux Network Config](https://wiki.archlinux.org/index.php/Network_configuration)
sudo ip addr flush dev eno1
sudo ip route flush dev eno1
