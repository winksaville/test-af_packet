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
#include <sys/socket.h>
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

void  println_sockaddr_ll(char* leader, struct sockaddr_ll* addr) {
  if (leader != NULL) {
    printf(leader);
  }
  printf("family=%0d protocol=0x%0x ifindex=%d hatype=%d pkttype=%d halen=%d addr=",
      addr->sll_family, ntohs(addr->sll_protocol), addr->sll_ifindex,
      addr->sll_hatype, addr->sll_pkttype, addr->sll_halen);
  println_hex(NULL, addr->sll_addr, addr->sll_halen, ':');
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
 * Initialize a struct ether_arp
 *
 * @param fd is the socket file descriptro
 * @param ifname is the name of the interface
 * @param ipv4_addr_str is a ipv4 dotted decimal address
 * @param pArpReq is the ether_arp to initialize
 *
 * @return 0 if OK
 */
int init_ether_arp(struct ether_arp* pArpReq,
    const int fd, const char* ifname, const char*ipv4_addr_str) {
  int error = 0; // Assume no errors

  // Initialize ethernet arp request
  pArpReq->arp_hrd = htons(ARPHRD_ETHER);
  pArpReq->arp_pro = htons(ETH_P_IP);
  pArpReq->arp_hln = ETHER_ADDR_LEN;
  pArpReq->arp_pln = sizeof(in_addr_t);
  pArpReq->arp_op = htons(ARPOP_REQUEST);

  // Convert ipv4_addr_str to target address and
  // set arp target protocol address (arp_tpa)
  struct in_addr target_ip_addr = {0};
  ON_Z (inet_aton(ipv4_addr_str, &target_ip_addr), done);
  memcpy(&pArpReq->arp_tpa, &target_ip_addr, sizeof(pArpReq->arp_tpa));

  // Zero the arp target hardware address (arp_tha)
  memset(&pArpReq->arp_tha, 0, sizeof(pArpReq->arp_tha));

  // Get Source hardware address to arp source hardware address (arp_sha)
  ON_NZ (get_ethernet_mac_addr(fd, ifname, pArpReq->arp_sha), done);
  println_hex("send_ethernet_arp_ipv4: pArpReq->arp_sha=",
      pArpReq->arp_sha, sizeof(pArpReq->arp_sha), ':');

  // Get source ipv4 address to arp source protocol address (arp_spa)
  struct sockaddr_in ipv4_addr;
  ON_NZ (get_ethernet_ipv4_addr(fd, ifname, &ipv4_addr), done);
  memcpy(pArpReq->arp_spa, &ipv4_addr.sin_addr, sizeof(pArpReq->arp_spa));
  println_hex("send_ethernet_arp_ipv4: req.arp_spa=",
      pArpReq->arp_spa, sizeof(pArpReq->arp_spa), '.');

done:
  return error;
}

/**
 * init a struct ethhdr
 *
 * @param dst_addr
 */
void init_ethhdr(struct ethhdr* pEthHdr, const void *dst_addr, const void* src_addr, int ll_protocol) {
  memcpy(pEthHdr->h_dest, dst_addr, ETH_ALEN);
  memcpy(pEthHdr->h_source, src_addr, ETH_ALEN);
  pEthHdr->h_proto = ll_protocol;
}

/**
 * Init sockaddr_ll
 */
void init_sockaddr_ll(struct sockaddr_ll* pSockAddrLl,
    const unsigned char ethernet_addr[ETHER_ADDR_LEN],
    const int ifindex,
    const int protocol) {
  memset(pSockAddrLl, 0, sizeof(struct sockaddr_ll));

  pSockAddrLl->sll_family = AF_PACKET;
  pSockAddrLl->sll_protocol = htons(protocol);
  pSockAddrLl->sll_ifindex = ifindex;
  pSockAddrLl->sll_halen = ETHER_ADDR_LEN;
  memcpy(pSockAddrLl->sll_addr, ethernet_addr, ETHER_ADDR_LEN);
}

/**
 * Init sockaddr_ll
 */
void init_broadcast_sockaddr_ll(struct sockaddr_ll* pSockAddrLl,
    const int ifindex,
    const int protocol) {
  unsigned char ethernet_broadcast_addr[ETHER_ADDR_LEN] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff
  };

  init_sockaddr_ll(pSockAddrLl, ethernet_broadcast_addr, ifindex, ETH_P_ARP);
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

#if 0 // Use sendto

  struct ethernet_raw {
    struct ethhdr ether_hdr;
    union {
      struct ether_arp arp_req;
      unsigned char payload[2048 - sizeof(struct ethhdr)];
    };
  } __attribute__((packed));

  struct ethernet_raw packet;
  memset(&packet, 0, sizeof(packet));

  // Get the interface index
  int ifindex;
  ON_NZ (get_ifindex(fd, ifname, &ifindex), done);
  printf("send_ethernet_arp_ipv4: ifname=%s findex=%d\n", ifname, ifindex);

  // Initialize addr to the Ethernet broadcast address and ARP protocol
  struct sockaddr_ll dst_addr;

  init_broadcast_sockaddr_ll(&dst_addr, ifindex, ETH_P_ARP);
  println_sockaddr_ll("send_ethernet_arp_ipv4: ", &dst_addr);

  // Initialize ethernet arp request
  ON_NZ (init_ether_arp(&packet.arp_req, fd, ifname, ipv4_addr_str), done);

  // Initialize ethernet header
  init_ethhdr(&packet.ether_hdr, &dst_addr, &packet.arp_req.arp_sha, dst_addr.sll_protocol);

  // Broadcast the arp request
  int len = sizeof(packet.ether_hdr) + sizeof(packet.arp_req);
  if (len < 60) {
    len = 60;
  }
  println_hex("send_ethernet_arp_ipv4: packet=", &packet, len, ' ');
  int count = sendto(fd, &packet, len, 0,
      (struct sockaddr*)&dst_addr, sizeof(dst_addr));
  ON_LZ (count, done);
  printf("send_ethernet_arp_ipv4: sent count=%d\n", count);

#else // Use sendmsg

  struct sockaddr_ll dst_addr;

  // Get the interface index
  int ifindex;
  ON_NZ (get_ifindex(fd, ifname, &ifindex), done);
  printf("send_ethernet_arp_ipv4: ifname=%s findex=%d\n", ifname, ifindex);

  init_broadcast_sockaddr_ll(&dst_addr, ifindex, ETH_P_ARP);
  println_sockaddr_ll("send_ethernet_arp_ipv4: ", &dst_addr);

  // Initialize ethernet arp request
  struct ether_arp arp_req;
  ON_NZ (init_ether_arp(&arp_req, fd, ifname, ipv4_addr_str), done);

  // Initialize ethernet header
  struct ethhdr ether_hdr;
  init_ethhdr(&ether_hdr, &dst_addr, &arp_req.arp_sha, dst_addr.sll_protocol);

  // Initialize iovec for msghdr
  uint8_t padding[60];
  memset(padding, 0, sizeof(padding));

  struct iovec iov[3];
  int iovlen = 2;
  iov[0].iov_base = &ether_hdr;
  iov[0].iov_len = sizeof(ether_hdr);
  iov[1].iov_base = &arp_req;
  iov[1].iov_len = sizeof(arp_req);

  int len = sizeof(ether_hdr) + sizeof(arp_req);
  if (len < 60) {
    // We need to padd out the frame to 60 bytes
    // so add an extr iov;
    iov[iovlen].iov_base = padding;
    iov[iovlen].iov_len = 60 - len;
    iovlen += 1;
  }

  // Initialize msghdr
  struct msghdr mh;
  mh.msg_name = &dst_addr;
  mh.msg_namelen = sizeof(dst_addr);
  mh.msg_iov = iov;
  mh.msg_iovlen = iovlen;
  printf("iovlen=%lu\n", mh.msg_iovlen);
  mh.msg_control = NULL;
  mh.msg_controllen = 0;
  mh.msg_flags = 0;

  //println_hex("send_ethernet_arp_ipv4: mh=", &packet, len, ' ');
  int count = sendmsg(fd, &mh, 0);
  ON_LZ (count, done);
  printf("send_ethernet_arp_ipv4: sent count=%d\n", count);

#endif

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
  int fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
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

  // Read response
  uint8_t resp[128];
  socklen_t addrlen = sizeof(struct sockaddr_ll);
  struct sockaddr_ll addr;

  // TODO: Add recvmsg
  int count = recvfrom(fd, &resp, sizeof(resp), 0,
      (struct sockaddr*)&addr, &addrlen);
      //(struct sockaddr*)&addr, &addrlen);
  ON_LZ (count, done);
  printf("recvfrom: count=%d addrlen=%d\n", count, addrlen);
  println_hex("recvfrom:  addr=", &addr, addrlen, ':');
  println_sockaddr_ll("recvfrom:  addr:", &addr);
  println_hex("recvfrom:  resp=", (unsigned char*)&resp, count, ' ');

  // Check that its the expected response
  if (addr.sll_family != AF_PACKET) {
    printf("recvfrom: error family=%d != AF_PACKET=%d\n",
        addr.sll_family, AF_PACKET);
    error = 1;
    goto done;
  }
  if (htons(addr.sll_protocol) != ETH_P_ARP) {
    printf("recvfrom: error addr.sll_protocol=0x%x != ETH_PARP=0x%x\n",
        htons(addr.sll_protocol), ETH_P_ARP);
    error = 1;
    goto done;
  }

  // TODO: Verify that the arp_resp exists
  struct ether_arp* arp_resp = (struct ether_arp*)&resp[14];
  println_hex("recvfrom: arp_resp=", arp_resp, sizeof(struct ether_arp), ' ');

done:
  if (fd >= 0) {
    close(fd);
  }

  printf("error=%d\n", error);
  return error;
}
