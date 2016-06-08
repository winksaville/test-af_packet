/**
 * This software is released into the public domain.
 *
 * Based on Graham Shaw's [microhowto.info](http://www.microhowto.info/howto/send_an_arbitrary_ethernet_frame_using_an_af_packet_socket_in_c.html)
 * and RFC's such as [rfc1180](https://tools.ietf.org/html/rfc1180)
 */
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/if_ether.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdio.h>

int main(int argc, const char* argv[]) {
  printf("test-af_packet: argc=%d\n", argc);
  for (int i = 0; i < argc; i++) {
    printf("argv[%d]=%s\n", i, argv[i]);
  }

  int fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ARP));
  if (fd < 0) {
    printf("%s\n", strerror(errno));
  } else {
    printf("Success: fd=%d\n", fd);
  }

  if (fd >= 0) {
    close(fd);
  }
}
