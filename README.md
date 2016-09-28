test AF_Packet [![CircleCI](https://circleci.com/gh/winksaville/test-af_packet.svg?style=svg)](https://circleci.com/gh/winksaville/test-af_packet)
===

Simple test of AF_PACKET and using macvlan interface. Before executing ./test-af_packet execute
./macvlan.sh to setup a macvlan sub-interface.

Since I'm using macvlan interfaces and they are [not supported](https://discuss.circleci.com/t/how-to-create-tap-network-interfaces/4483) by circleci this is
not continuously tested :(

Someday hopefully they will support it or I need to use another CI system, a
possibility might be [concourse.ci](https://concourse.ci/). [Here](https://opensource.com/business/15/7/six-continuous-integration-tools) is a list of some
open source CI's and [here](https://en.wikipedia.org/wiki/Comparison_of_continuous_integration_software) is what wikipedia has.
