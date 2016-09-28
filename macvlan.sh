#!/usr/bin/env bash
# Some docs for iproute2 see:
# [biproute2 cheat sheet - Daniil Baturi](http://baturin.org/docs/iproute2/)
# [LInux Advanced Routing & Traffic Control HOWTO](http://lartc.org/howto/index.html)
#
# [Macvlan and IPvlan basics](https://sreeninet.wordpress.com/2016/05/29/macvlan-and-ipvlan/)
# [Bridge vs Macvlan](http://hicu.be/bridge-vs-macvlan)
set +x

# Defaults
DFLT_IF1="eno1"
DFLT_IF2="mvl1"

# Associative array of 'named' argments
declare -A args

# Diaplay usage
usage() {
  echo "Usage: $0 cmd [ if1=<name> ] [ if2=<name> ]"
  echo "  Add or delete if2 as a sub-interfaace of if1"
  echo ""
  echo "  First parameter must be 'add' or 'del'"
  echo "  optional param if1 is the interface name to the interent, default ${DFLT_IF1}"
  echo "  optional param if2 will be the name of the sub-interface to if1, default ${DFLT_IF2}"
}

# Display args
display_args() {
  echo args[if1]=${args[if1]}
  echo args[if2]=${args[if2]}
}

# Parse the arguments
function parse_args() {
  local $*
  #echo "parse_args: $*"
  # First interface name
  [ "$if1" == "" ] && args[if1]=${DFLT_IF1} || args[if1]="$if1"
  # Second interface
  [ "$if2" == "" ] && args[if2]=${DFLT_IF2} || args[if2]="$if2"
}

# Add If2 as a sub-interface of if1
macvlan_add() {
  # if if1 doesn't exist exit
  sudo ip address show ${args[if1]} 1> /dev/null 2>&1
  if [ $? != 0 ]; then
    echo "Error: no ${args[if1]}, exiting"
    exit 1
  fi

  # if if2 exists exit
  sudo ip address show ${args[if2]} 1> /dev/null 2>&1
  if [ $? != 0 ]; then
    sudo ip link add ${args[if2]} link ${args[if1]} type macvlan mode bridge
    if [ $? != 0 ]; then
      echo "Error: could not create ${args[if2]}"
      exit 1
    fi
  else
    echo "Error: ${args[if2]} already exists, exiting"
    exit 1
  fi
  echo -e "\nAfter creating ${args[if2]}"
  sudo ip address


  # Uset dhclient to get if2 an address
  sudo dhclient ${args[if2]}
  if [ $? != 0 ]; then
    echo "Error: $? from dhclient, exiting"
    exit 1
  fi
  echo -e "\nAfter getting address for ${args[if2]}"
  sudo ip address

  echo -e "\ncomplete, show route:"
  ip route show
}

# delete if2
macvlan_del() {
  # Exit if if2 doesn't exist
  output=$(sudo ip address show ${args[if2]} 2>&1)
  if [ $? != 0 ]; then
    echo "Error: ${args[if2]} doesn't exist, exiting"
    exit 1
  fi
  echo "output=$output"
  
  pattern="^.*: *${args[if2]}@${args[if1]}.*"
  echo "pattern=$pattern"
  if [[ $output =~ $pattern ]]; then
    sudo ip link set dev ${args[if2]} down
    sudo ip link del ${args[if2]}
    echo -e "\nAfter deleting ${args[if2]}"
    sudo ip address
  else
    echo "Error: ${args[if2]} is not a sub-interface of ${args[if1]}"
    exit 1
  fi
}

# process the command
case "$1" in
  "add")
    parse_args $*
    macvlan_add
    ;;
  "del")
    parse_args $*
    #display_args
    macvlan_del
    ;;
  "-h")
    ;&
  "help")
    usage $@
    ;;
  *)
    echo "Unknown command: '$1'\n"
    usage $*
    ;;
esac
