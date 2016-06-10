/**
 * This software is released into the public domain.
 *
 * Based on Graham Shaw's [microhowto.info](http://www.microhowto.info/howto/send_an_arbitrary_ethernet_frame_using_an_af_packet_socket_in_c.html)
 * and RFC's such as [rfc1180](https://tools.ietf.org/html/rfc1180)
 */
#define _DEFAULT_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>
#include <string.h>
#include <sys/ioctl.h>
//#include <net/if.h>
#include <linux/if.h>
#include <unistd.h>

#include <stdio.h>

#define ON_EXPR_TRUE(s, x, label) { \
  int result = (x); \
  if (result != 0) { \
    printf("%s: result '%s' == %d in %s:%d function %s\n", \
           s, #x, result, __FILE__, __LINE__, __func__); \
    error |= result; \
    goto label; \
  } \
}

#define ON_NZ(x, label) ON_EXPR_TRUE("ON_NZ", ((x) != 0), label)
#define ON_TRUE(x, label) ON_EXPR_TRUE("ON_TRUE", (x), label)

/**
 * Display memory
 */
void print_mem(char* leader, void *mem, int len, char* format, char sep, char* trailer) {
  if (leader != NULL) {
    printf(leader);
  }
  unsigned char* p = (unsigned char*)mem;

  for (int i = 0; i < len; i++) {
    if (i != 0) printf("%c", sep);
    printf(format, p[i]);
  }

  if (trailer != NULL) {
    printf(trailer);
  }
}

/**
 * Display hex memory
 */
void println_hex(char* leader, void *mem, int len, char sep) {
  print_mem(leader, mem, len, "%02x", sep, "\n");
}

/**
 * Display hex memory
 */
void println_dec(char* leader, void *mem, int len, char sep) {
  print_mem(leader, mem, len, "%d", sep, "\n");
}

/**
 * Set ifr_name
 *
 * @return 0 if OK
 */
int IfReq_set_ifr_name(struct ifreq* ifr, const char* ifname) {
  int error = 0;

  // Copy ifname to ifr.ifr_name if it fits
  ON_TRUE (strlen(ifname) > (sizeof(ifr->ifr_name) - 1), done);
  strcpy(&ifr->ifr_name[0], ifname);

done:
  return error;
}

/**
 * Get index for ifname
 *
 * @return 0 if OK
 */
int get_ifindex(int fd, const char* ifname, int* ifindex) {
  int error = 0;    // Assume no errors
  struct ifreq ifr;

  ON_NZ (IfReq_set_ifr_name(&ifr, ifname), done);

  // Issue iotcl to get the index
  ON_TRUE (ioctl(fd, SIOCGIFINDEX, &ifr) < 0, done);

  *ifindex = ifr.ifr_ifindex;

done:
  return error;
}


/**
 * Get MAC address of NIC in network order
 *
 * @return 0 if OK
 */
int get_ethernet_mac_addr(int fd, const char* ifname, unsigned char mac_addr[ETH_ALEN]) {
  int error = 0;    // Assume no errors
  struct ifreq ifr;

  ON_NZ (IfReq_set_ifr_name(&ifr, ifname), done);

  // Issue iotcl to get the hardware/mac address
  ON_TRUE (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0, done);

#if 0
  println_hex("ifr.ifr_hwaddr.sa_data=", &ifr.ifr_hwaddr.sa_data, sizeof(ifr.ifr_hwaddr.sa_data), ':');
#endif

  // Be sure address fits
  _Static_assert(ETH_ALEN <= sizeof(ifr.ifr_hwaddr.sa_data),
    "ETH_ALEN is larger than ifr.ifr_hwaddr.sa_data");

  memcpy(mac_addr, &ifr.ifr_hwaddr.sa_data, ETH_ALEN);

done:
  return error;
}

/**
 * Get IPV4 address of NIC in network order
 *
 * @return 0 if OK
 */
int get_ethernet_ipv4_addr(int fd, const char* ifname, struct sockaddr_in* ipv4_addr) {
  int error = 0;    // Assume no errors
  struct ifreq ifr;

  //error |= IfReq_set_ifr_name(&ifr, ifname);
  //if (error) goto done;
  ON_NZ (IfReq_set_ifr_name(&ifr, ifname), done);

  // Issue iotcl to get the address
  ON_TRUE (ioctl(fd, SIOCGIFADDR, &ifr) < 0, done);

  // Be sure address fits
  _Static_assert(sizeof(*ipv4_addr) <= sizeof(ifr.ifr_addr),
    "struct sockaddr_in larger than ifr.ifr_addr");

  // Get ipv4 address
  memcpy(ipv4_addr, &ifr.ifr_addr, sizeof(*ipv4_addr));

done:
  return error;
}


/**
 * Init sockaddr_ll
 */
struct sockaddr_ll* sockaddr_ethernet_init(struct sockaddr_ll* sa,
    const unsigned char ethernet_addr[ETHER_ADDR_LEN],
    const int ifindex,
    const int protocol) {
  memset(sa, 0, sizeof(struct sockaddr_ll));

  sa->sll_family = AF_PACKET;
  sa->sll_ifindex = ifindex;
  sa->sll_halen = ETHER_ADDR_LEN;
  sa->sll_protocol = htons(protocol);
  memcpy(sa->sll_addr, ethernet_addr, ETHER_ADDR_LEN);

  return sa;
}

/**
 * send_arp
 *
 * @return 0 if no errors
 */
int send_ethernet_arp_ipv4(int fd, const char* ifname, const char* ipv4_addr_str) {
  int error = 0;
#if 0
  struct ether_arp req;
  int ifindex;

  // Convert ipv4_addr_str to target address
  struct in_addr target_ip_addr = {0};
  ON_NZ (inet_aton(ipv4_addr_str, &target_ip_addr) != 0, done);

  // Get Source hardware address and ipv4 address
  ON_NZ (get_ethernet_mac_addr(fd, ifname, req.arp_sha), done);

  struct sockaddr_in ipv4_addr;
  ON_NZ (get_ethernet_ipv4_addr(fd, ifname, &ipv4_addr), done);
  memcpy(req.arp_spa, ipv4_addr.sin_addr, sizeof(req.arp_spa));

  // Initialize addr to the Ethernet broadcast address and ARP protocol
  struct sockaddr_ll addr;
  unsigned char ethernet_broadcast_addr[ETHER_ADDR_LEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

  sockaddr_ethernet_init(&addr, ethernet_broadcast_addr, ifindex, ETH_P_ARP);
  ON_NZ (addr.sll_ifindex != ifindex), done);
  ON_NZ (addr.sll_protocol != htons(ETH_P_ARP)), done);
  ON_NZ (memcmp(addr.sll_addr, ethernet_broadcast_addr, ETHER_ADDR_LEN) != 0), done);
  printf("addr: family=%d ifindex=%d protocol=%d\n", addr.sll_family, addr.sll_ifindex, addr.sll_protocol);

  // Initialize ethernet arp request
  req.arp_hrd = htons(ARPHRD_ETHER);
  req.arp_pro = htons(ETH_P_IP);
  req.arp_hln = ETHER_ADDR_LEN;
  req.arp_pln = sizeof(in_addr_t);
  req.arp_op = htons(ARPOP_REQUEST);
  memset(&req.arp_tha, 0, sizeof(req.arp_tha));
  memcpy(&req.arp_tpa, &target_ip_addr, sizeof(req.arp_tpa));

done:
#endif
  return error;
}

int main(int argc, const char* argv[]) {
  int error = 0;

  printf("test-af_packet: argc=%d\n", argc);
  for (int i = 0; i < argc; i++) {
    printf("argv[%d]=%s\n", i, argv[i]);
  }

  if (argc != 2) { 
    printf("test-af_packet: Usage param $1 is interface name\n");
    error = 1;
    goto done;
  }

  // Open an AF_PACKET socket
  int fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ARP));
  printf("fd=%d\n", fd);
  ON_TRUE (fd < 0, done);

  // Get interface segment
  int ifindex;
  ON_NZ (get_ifindex(fd, argv[1], &ifindex), done);
  printf("ifindex=%d\n", ifindex);

  // Get interface mac address
  unsigned char mac_addr[6];
  ON_NZ (get_ethernet_mac_addr(fd, argv[1], mac_addr), done);
  println_hex("mac_addr=", mac_addr, sizeof(mac_addr), ':');

  // Get interface ip address
  struct sockaddr_in ipv4_addr;
  ON_NZ (get_ethernet_ipv4_addr(fd, argv[1], &ipv4_addr), done);
  println_dec("ipv4_addr=", &ipv4_addr.sin_addr, 4, '.');

done:
  if (fd >= 0) {
    close(fd);
  }

  printf("error=%d\n", error);
  return error;
}
