/**
 * This software is released into the public domain.
 *
 * Based on Graham Shaw's [microhowto.info](http://www.microhowto.info/howto/send_an_arbitrary_ethernet_frame_using_an_af_packet_socket_in_c.html)
 * and RFC's such as [rfc1180](https://tools.ietf.org/html/rfc1180)
 */
#include <arpa/inet.h>
#include <errno.h>
#include <net/ethernet.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
//#include <net/if.h>
#include <linux/if.h>
#include <unistd.h>

#include <stdio.h>

/**
 * Get index for ifname
 *
 * @return < 0 if an error
 */
int get_ifindex(int fd, const char* ifname) {
  struct ifreq ifr;
  size_t max_len_ifr_name = sizeof(ifr.ifr_name) - 1;

  // Copy ifname to ifr.ifr_name if it fits
  size_t len = strlen(ifname);
  printf("sizeof(ifr.ifr_name)=%ld len=%ld\n", sizeof(ifr.ifr_name), len);
  if (len > max_len_ifr_name) {
    return -1;
  }
  strcpy(&ifr.ifr_name[0], ifname);

  printf("ifr.ifr_name=%s\n", ifr.ifr_name);

  // Issue iotcl to get the index
  if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1) {
    return -1;
  }
  return ifr.ifr_ifindex;
}

int main(int argc, const char* argv[]) {
  int fd = -1;
  int error;

  printf("test-af_packet: argc=%d\n", argc);
  for (int i = 0; i < argc; i++) {
    printf("argv[%d]=%s\n", i, argv[i]);
  }

  if (argc != 2) {
    printf("test-af_packet: Usage param $1 is interface name\n");
    error = 1;
    goto done;
  }

  fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ARP));
  if (fd < 0) {
    printf("%s\n", strerror(errno));
  } else {
    printf("Success: fd=%d\n", fd);
  }

  int ifindex = get_ifindex(fd, argv[1]);
  error = (ifindex < 0) ? 1 : 0;
  printf("ifindex=%d\n", ifindex);

done:
  if (fd >= 0) {
    close(fd);
  }

  printf("error=%d\n", error);
  return error;
}
