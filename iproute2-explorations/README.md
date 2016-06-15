Some docs for iproute2 see:
 - [biproute2 cheat sheet - Daniil Baturi](http://baturin.org/docs/iproute2/)
 - [LInux Advanced Routing & Traffic Control HOWTO](http://lartc.org/howto/index.html)

I had a heck of a time figuring out how to change the ethernet device
to use static routes and then switch back to dynamic routes. I've created
three scripts to capture something that works. One key was to use flush
see eno1-flush.sh to flush out existing configuration.

In dynamic.sh removes if2 and bridge and uses dhcpcd to bring up if1

In static.sh removes if2 and bridge and sets up static ip and routes for if1

In bridge.sh it creates a bridge with two interfaces, if1 and if2 and
assigns a static address to the bridge.

These are probably not the "right" way but they do work sometimes, althoug
I've also lost connectivity to the internet and had to reboot.

SO BE CAREFUL
