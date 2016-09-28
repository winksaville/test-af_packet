#!/usr/bin/env bash
# Some docs for iproute2 see:
# [biproute2 cheat sheet - Daniil Baturi](http://baturin.org/docs/iproute2/)
# [LInux Advanced Routing & Traffic Control HOWTO](http://lartc.org/howto/index.html)
set +x

# Defaults
DFLT_IF1="eno1"
DFLT_IF2="tap0"
DFLT_BR="br0"
DFLT_BR_IP="192.168.0.90/16"
DFLT_IF1_IP="192.168.0.99/16"
DFLT_IF2_IP="192.168.0.96/16"
DFLT_BR_ROUTE="192.168.0.0/16"
DFLT_GW="192.168.0.2"

if [ "$1" == "help" ] || [ "$1" == "-h" ]; then
  echo "Usage: $0 [ if1=<name> ] [ if2=<name> ] [ br=<name> ] [ br_ip=<ip/bits> ] [ br_route=<ip/bits> ] [ gw=<ip> ]"
  echo " Change Add a bride for if1 and if2, br_ip is the bridges ip address and br_route is its route" 
  echo ""
  echo "  if1 is the interface name to the interent, default ${DFLT_IF1}"
  echo "  if1_ip is the address for if1, default ${DFLT_IF1_IP}"
  echo "  if2 is a second interface name, default ${DFLT_IF2}"
  echo "  if2_ip is the address for if2, default ${DFLT_IF2_IP}"
  echo "  br will created or flushed if it exists, default ${DFLT_BR}"
  echo "  br_ip is static ip for br, default ${DFLT_BR_IP}"
  echo "  br_route is route for br, default ${DFLT_BR_ROUTE}"
  echo "  gw is ip address of gateway to internet, default ${DFLT_GW}"
  exit 1
fi

# Associative array of 'named' argments
declare -A args

# Parse the arguments
function parse_args() {
  local $*
  # First interface name
  [ "$if1" == "" ] && args[if1]=${DFLT_IF1} || args[if1]="$if1"
  # First interface ip address
  [ "$if1_ip" == "" ] && args[if1_ip]=${DFLT_IF1_IP} || args[if1_ip]="$if1_ip"
  # Second interface
  [ "$if2" == "" ] && args[if2]=${DFLT_IF2} || args[if2]="$if2"
  # Second interface ip address
  [ "$if2_ip" == "" ] && args[if2_ip]=${DFLT_IF2_IP} || args[if2_ip]="$if2_ip"
  # bridge
  [ "$br" == "" ] && args[br]=${DFLT_BR} || args[br]="$br"
  # ip address for if1
  [ "$br_ip" == "" ] && args[br_ip]=${DFLT_BR_IP} || args[br_ip]="$br_ip"
  # route address for if1
  [ "$br_route" == "" ] && args[br_route]=${DFLT_BR_ROUTE} || args[br_route]="$ip"
  # gw is ip address for gateway
  [ "$gw" == "" ] && args[gw]=${DFLT_GW} || args[gw]="$gw"
}

parse_args $*

echo args[if1]=${args[if1]}
echo args[if1_ip]=${args[if1_ip]}
echo args[if2]=${args[if2]}
echo args[if2_ip]=${args[if2_ip]}
echo args[br]=${args[br]}
echo args[br_ip]=${args[br_ip]}
echo args[br_route]=${args[br_route]}
echo args[gw]=${args[gw]}

printf "\nCurrent state\n"
sudo ip address

# flush $if1, if it doesn't exist exit
sudo ip address show ${args[if1]} 1> /dev/null 2>&1
if [ $? != 0 ]; then
  echo "no ${args[if1]}" ; exit 1
else
  ./flush.sh ${args[if1]}
fi
echo -e "\nAfter flush of ${args[if1]}"
sudo ip address


# create $if2 if it doesn't exist else flush it
sudo ip address show ${args[if2]} 1> /dev/null 2>&1
if [ $? != 0 ]; then
  sudo ip tuntap add dev ${args[if2]} mode tap
  if [ $? != 0 ]; then
    echo "could not create ${args[if2]}"
    exit 1
  fi
else
  ./flush.sh ${args[if2]}
fi
echo -e "\nAfter flush of ${args[if2]}"
sudo ip address

# create br0 if it doesn't exist else flush it
sudo ip address show ${args[br]} 1> /dev/null 2>&1
if [ $? != 0 ]; then
  sudo ip link add name ${args[br]} type bridge
  if [ $? != 0 ]; then
    echo "could not create ${args[br]}"
    exit 1
  fi
  echo -e "\nAfter create of ${args[br]}"
  sudo ip address
else
  ./flush.sh ${args[br]}
  echo -e "\nAfter flush of ${args[br]}"
  sudo ip address
fi

# bring up bridge
sudo ip link set ${args[br]} up
echo -e "\nAfter up of ${args[br]}"
sudo ip address

# Add static address to the bridge
# From [cyberciti.biz ip examples](http://www.cyberciti.biz/faq/linux-ip-command-examples-usage-syntax/#4)
sudo ip addr add ${args[br_ip]} brd + dev ${args[br]}
echo -e "\nAfter add of br_ip=${args[br_ip]} to ${args[br]}"
sudo ip address

## Bring up if1, add to br as its master, add address
sudo ip link set ${args[if1]} up
echo -e "\nAfter up of ${args[if1]}"
sudo ip address

sudo ip link set dev ${args[if1]} master ${args[br]}
echo -e "\nAfter add of ${args[if1]} to ${args[br]}"
sudo ip address

# Add static address to the if1
sudo ip address add ${args[if1_ip]} brd + dev ${args[if1]}
echo -e "\nAfter add address ${args[if1_ip]} to ${args[if1]}"
sudo ip address


# Bring up if2, add to br as its master, add address
sudo ip link set ${args[if2]} up
echo -e "\nAfter up of ${args[if2]}"
sudo ip address

sudo ip link set dev ${args[if2]} master ${args[br]}
echo -e "\nAfter add of ${args[if2]} to as a bridge_slave${args[br]}"
sudo ip address

# Add static address to the if2
sudo ip address add ${args[if2_ip]} brd + dev ${args[if2]}
echo -e "\nAfter add address ${args[if2_ip]} to ${args[if2]}"
sudo ip address

# Add default route
# See [gentoo docs](https://wiki.gentoo.org/wiki/Iproute2)
sudo ip route add default via ${args[gw]}
echo -e "\nAfter add of default via =${args[gw]}"
sudo ip address

# Add static route to br
sudo ip route add ${args[br_route]} dev ${args[br]}
#sudo ip route add ${args[br_route]} metric 100 scope global dev ${args[br]}
echo -e "\nAfter add of br_route to ${args[br]}"
sudo ip address

# Display results
echo -e "\ncomplete, show address and route:"
sudo ip address show ; ip route show
