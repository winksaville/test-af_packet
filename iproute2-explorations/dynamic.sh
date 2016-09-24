#!/usr/bin/env bash
# Switch to dynamic ip for if1 and delete if2 and br
#set -x

# Defaults
DFLT_IF1="eno1"
DFLT_IF2="tap0"
DFLT_BR="br0"

if [ "$1" == "help" ] || [ "$1" == "-h" ]; then
  echo "Usage: $0 [ if1=<name> ] [ if2=<name> ] [ br=<name> ]"
  echo " Change if1=eno1 to the dynamic address, delete if2 and br"
  echo ""
  echo "  if1 is the interface name to the interent, default ${DFLT_IF1}"
  echo "  if2 is a tap interface name, default ${DFLT_IF2}"
  echo "  br is a bridge for if1 and if2, default ${DFLT_BR}"
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

echo args[if1]=${args[if1]}
echo args[if2]=${args[if2]}
echo args[br]=${args[br]}

# Clean up
./cleanup.sh if1=${args[if1]} if2=${args[if2]} br=${args[br]}

# Down ${args[if1]} then release and rebind the interface
sudo ip link set ${args[if1]} down
sudo dhcpcd --release ${args[if1]}
sudo dhcpcd --rebind ${args[if1]}

# Display results
sudo ip addr show ; ip route show
