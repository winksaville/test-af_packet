Some docs for iproute2 see:
 - [biproute2 cheat sheet - Daniil Baturi](http://baturin.org/docs/iproute2/)
 - [LInux Advanced Routing & Traffic Control HOWTO](http://lartc.org/howto/index.html)

I had a heck of a time figuring out how to change the ethernet device
to use static routes and then switch back to dynamic routes. I've created
three scripts to capture something that works. One key was to use flush
see eno1-flush.sh to flush out existing configuration.

The eno1-flush.sh is called as the first thing in eno1-static.sh and
eno1-dynamic.sh.

In eno1-dynamic.sh I just bring eno1 interface down then up and it
dhclient seems to do its thing and gets a new address and updates
the routing table.

In eno1-static.sh I use ip addr and ip route commands note I do not
use `ip link set eno1 up` or `down` because if I do I end up with
both static and the dynamic addersses.

NOTE: All this is just something that 'works' right now, I suspect as I
learn more I find the 'right' way to do it.
