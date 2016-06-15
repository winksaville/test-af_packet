#!/bin/bash
# Switch to static address for if1 and delete if2 and br
#
# [biproute2 cheat sheet - Daniil Baturi](http://baturin.org/docs/iproute2/)
# [LInux Advanced Routing & Traffic Control HOWTO](http://lartc.org/howto/index.html)
#set -x

# Defaults
DFLT_IF1="eno1"
DFLT_IF1_IP="192.168.0.90/24"
DFLT_IF1_ROUTE="192.168.0.0/24"
DFLT_IF2="tap0"
DFLT_BR="br0"
DFLT_GW="192.168.0.2"

if [ "$1" == "help" ] || [ "$1" == "-h" ]; then
  echo "Usage: $0 [ if1=<name> ] [ if1_ip=<ip/bits> ] [ if1_route=<ip/bits> ] [ gw=<ip> ] [ if2=<name> ] [ br=<name> ]"
  echo " Change if1 to the static address in parameter if1_ip=x.x.x.x, delete if2 and br"
  echo ""
  echo "  if1 is the interface name to the interent, default ${DFLT_IF1}"
  echo "  if1_ip is static ip fir if1, default ${DFLT_IF1_IP}"
  echo "  if1_route is static ip for if1, default ${DFLT_IF1_ROUTE}"
  echo "  gw is ip address of gateway to internet, default ${DFLT_GW}"
  echo "  if2 will be deleted if it exists, default ${DFLT_IF2}"
  echo "  br will be deleted if it exists, default ${DFLT_BR}"
  exit 1
fi

# Associative array of 'named' argments
declare -A args

# Parse the arguments
function parse_args() {
  local $*
  # First interface name
  [ "$if1" == "" ] && args[if1]=${DFLT_IF1} || args[if1]="$if1"
  # ip address for if1
  [ "$if1_ip" == "" ] && args[if1_ip]=${DFLT_IF1_IP} || args[if1_ip]="$if_ip"
  # route address for if1
  [ "$if1_route" == "" ] && args[if1_route]=${DFLT_IF1_ROUTE} || args[if1_route]="$if1_route"
  # Second interface
  [ "$if2" == "" ] && args[if2]=${DFLT_IF2} || args[if2]="$if2"
  # bridge
  [ "$br" == "" ] && args[br]=${DFLT_BR} || args[br]="$br"
  # gw is ip address for gateway
  [ "$gw" == "" ] && args[gw]=${DFLT_GW} || args[gw]="$gw"
}

parse_args $*

echo args[if1]=${args[if1]}
echo args[if1_ip]=${args[if1_ip]}
echo args[if1_route]=${args[if1_route]}
echo args[if2]=${args[if2]}
echo args[br]=${args[br]}
echo args[gw]=${args[gw]}

./cleanup.sh if1=${args[if1]} if2=${args[if2]} br=${args[br]}

# Add static address
# From [cyberciti.biz ip examples](http://www.cyberciti.biz/faq/linux-ip-command-examples-usage-syntax/#4)
sudo ip address add ${args[if1_ip]} brd + dev ${args[if1]}

# Add default route
# See [gentoo docs](https://wiki.gentoo.org/wiki/Iproute2)
sudo ip route add default via ${args[gw]}

# Add static route
sudo ip route add ${args[if1_route]} metric 100 scope global dev ${args[if1]}

# Display results
sudo ip addr show ; ip route show
