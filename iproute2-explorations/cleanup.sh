#!/bin/bash
# flush if1, delte if2 and br
#set -x

# Defaults
DFLT_IF1="eno1"
DFLT_IF2="tap0"
DFLT_BR="br0"

if [ "$1" == "help" ] || [ "$1" == "-h" ]; then
  echo "Usage: $0 [ if1=<name> ] [ if2=<name> ] [ br=<name> ] [ gw=<ip> ]"
  echo " Cleanup: Flush if1, delete if2 and br"
  echo ""
  echo "  if1 is the interface name to the interent, default ${DFLT_IF1}"
  echo "  if2 will be delete if it exists, default ${DFLT_IF2}"
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
  # Second interface
  [ "$if2" == "" ] && args[if2]=${DFLT_IF2} || args[if2]="$if2"
  # bridge
  [ "$br" == "" ] && args[br]=${DFLT_BR} || args[br]="$br"
}

parse_args $*

sudo ip address show ${args[if1]} 1> /dev/null 2>&1
if [ $? != 0 ]; then
  echo "no ${args[if1]}" ; exit 1
else
  ./flush.sh ${args[if1]}
fi

# Delete ${args[if2]} if it exits
sudo ip address show ${args[if2]} 1> /dev/null 2>&1
if [ $? == 0 ]; then
  ./flush.sh ${args[if2]}
  sudo ip link set ${args[if2]} down
  sudo ip tuntap del name ${args[if2]} mode tap
fi

# Delete ${args[br]} if it exits
sudo ip address show ${args[br]} 1> /dev/null 2>&1
if [ $? == 0 ]; then
  ./flush.sh ${args[br]}
  sudo ip link set ${args[br]} down
  sudo brctl delbr ${args[br]}
fi
