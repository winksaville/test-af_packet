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

#define ON_TRUE(x, label) { \
  int result = (x); \
  if (result) { \
    printf("ON_TRUE: result '%s' == %d in %s:%d function %s\n", \
           #x, result, __FILE__, __LINE__, __func__); \
    error |= result; \
    goto label; \
  } \
}

#define ON_FALSE(x, label) { \
  int result = (x); \
  if (!result) { \
    printf("ON_FALSE: result '%s' == %d in %s:%d function %s\n", \
           #x, result, __FILE__, __LINE__, __func__); \
    error |= !result; \
    goto label; \
  } \
}

#define ON_Z(x, label) { \
  int result = (x); \
  if (result == 0) { \
    printf("ON_Z: result '%s' == %d in %s:%d function %s\n", \
           #x, result, __FILE__, __LINE__, __func__); \
    error |= result == 0; \
    goto label; \
  } \
}

#define ON_NZ(x, label) { \
  int result = (x); \
  if (result != 0) { \
    printf("ON_NZ: result '%s' == %d in %s:%d function %s\n", \
           #x, result, __FILE__, __LINE__, __func__); \
    error |= result; \
    goto label; \
  } \
}

#define ON_LZ(x, label) { \
  int result = (x); \
  if (result < 0) { \
    printf("ON_LZ: result '%s' == %d in %s:%d function %s\n", \
           #x, result, __FILE__, __LINE__, __func__); \
    error |= result < 0; \
    goto label; \
  } \
}

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
  ON_LZ (ioctl(fd, SIOCGIFINDEX, &ifr), done);

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
  ON_LZ (ioctl(fd, SIOCGIFHWADDR, &ifr), done);

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
  ON_LZ (ioctl(fd, SIOCGIFADDR, &ifr), done);

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

  printf("send_ethernet_arp_ipv4:+fd=%d ifname=%s ipv4=%s\n",
      fd, ifname, ipv4_addr_str);

  struct ether_arp req;

  // Get the interface index
  int ifindex;
  ON_NZ (get_ifindex(fd, ifname, &ifindex), done);
  printf("send_ethernet_arp_ipv4: ifname=%s findex=%d\n", ifname, ifindex);

  // Initialize addr to the Ethernet broadcast address and ARP protocol
  struct sockaddr_ll addr;
  unsigned char ethernet_broadcast_addr[ETHER_ADDR_LEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

  sockaddr_ethernet_init(&addr, ethernet_broadcast_addr, ifindex, ETH_P_ARP);
  ON_TRUE (addr.sll_ifindex != ifindex, done);
  ON_TRUE (addr.sll_protocol != htons(ETH_P_ARP), done);
  ON_NZ (memcmp(addr.sll_addr, ethernet_broadcast_addr, ETHER_ADDR_LEN), done);
  printf("send_ethernet_arp_ipv4: family=%d ifindex=%d protocol=%d\n", addr.sll_family, addr.sll_ifindex, addr.sll_protocol);

  // Initialize ethernet arp request
  req.arp_hrd = htons(ARPHRD_ETHER);
  req.arp_pro = htons(ETH_P_IP);
  req.arp_hln = ETHER_ADDR_LEN;
  req.arp_pln = sizeof(in_addr_t);
  req.arp_op = htons(ARPOP_REQUEST);

  // Convert ipv4_addr_str to target address and set arp target protocol address (arp_tpa)
  struct in_addr target_ip_addr = {0};
  ON_Z (inet_aton(ipv4_addr_str, &target_ip_addr), done);
  memcpy(&req.arp_tpa, &target_ip_addr, sizeof(req.arp_tpa));

  // Zero the arp target hardware address (arp_tha)
  memset(&req.arp_tha, 0, sizeof(req.arp_tha));

  // Get Source hardware address to arp source hardware address (arp_sha)
  ON_NZ (get_ethernet_mac_addr(fd, ifname, req.arp_sha), done);
  println_hex("send_ethernet_arp_ipv4: req.arp_sha=", req.arp_sha, sizeof(req.arp_sha), ':');

  // Get source ipv4 address to arp source protocol address (arp_spa)
  struct sockaddr_in ipv4_addr;
  ON_NZ (get_ethernet_ipv4_addr(fd, ifname, &ipv4_addr), done);
  memcpy(req.arp_spa, &ipv4_addr.sin_addr, sizeof(req.arp_spa));
  println_hex("send_ethernet_arp_ipv4: req.arp_spa=", req.arp_spa, sizeof(req.arp_spa), '.');

  // Broadcast the arp request
  int count = sendto(fd, &req, sizeof(req), 0, (struct sockaddr*)&addr, sizeof(addr));
  ON_LZ (count, done);
  printf("send_ethernet_arp_ipv4: sent count=%d\n", count);

done:
  printf("send_ethernet_arp_ipv4:-error=%d\n", error);
  return error;
}

int main(int argc, const char* argv[]) {
  int error = 0;

  printf("test-af_packet: argc=%d\n", argc);
  for (int i = 0; i < argc; i++) {
    printf("argv[%d]=%s\n", i, argv[i]);
  }

  if (argc != 3) {
    printf("Get ethernet MAC address of for target IPV4 address\n\n");
    printf("Usage:\n");
    printf("  %s <link name> <target IPV4 address>\n", argv[0]);
    printf("  link name: Name of NIC such as \"tap0\"\n");
    printf("  target IPV4 address: A dotted decimal IPV4 address\n");
    error = 1;
    goto done;
  }

  // Open an AF_PACKET socket
  int fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ARP));
  ON_LZ (fd, done);
  printf("fd=%d\n", fd);

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

  // Send arp ipv4
  ON_NZ (send_ethernet_arp_ipv4(fd, argv[1], argv[2]), done);

done:
  if (fd >= 0) {
    close(fd);
  }

  printf("error=%d\n", error);
  return error;
}
