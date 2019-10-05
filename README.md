# nlinline
A quick and clean API for NetLink networking configuring.

NLINLINE (netlink inline) is a *library* of inline functions providing C programmers with very handy functions to configure network stacks. NLINLINE is entirely implemented in a header file, nlinline.h.

NLinline provides the following functions:

*  `nlinline_if_nametoindex(const char *ifname);` return the index of the network interface whose name is `ifname`

*  `int nlinline_linksetupdown(unsigned int ifindex, int updown);` turn the interface `ifindex` up (`updown == 1`) or down (`updown == 0`).

*  `int nlinline_ipaddr_add(int family, void *addr, int prefixlen, int ifindex)` add an IP address to the interface `ifindex`. It supports IPv4 (`family == AF_INET`) and IPv6 `(family == AF_INET6)`.

*  `int nlinline_ipaddr_del(int family, void *addr, int prefixlen, int ifindex)` remove the IP address from the interface `ifindex`. It supports IPv4 (`family == AF_INET`) and IPv6 `(family == AF_INET6)`.

*  `int nlinline_iproute_add(int family, void *dst_addr, int dst_prefixlen, void *gw_addr);` add a static route to `dst_addr`/`dst_prefixlen` network through the gateway `gw_addr`. If `dst_addr == NULL` it adds a default route.

*  `int nlinline_iproute_del(int family, void *dst_addr, int dst_prefixlen, void *gw_addr);` remove the static route to `dst_addr`/`dst_prefixlen` network through the gateway `gw_addr`. 

IP addresses are `void *` arguments, any sequence of 4 or 16 bytes (in network byte order) is a legal IPv4 or IPv6 address respectively.

### Netlink + inline

* NLINLINE is a simple way to configure networking for network namespaces and *Internet of Threads* programs.
* NLINLINE do not add dependencies at run-time. It is useful for security critical applications (like PAM modules)
* NLINLINE uses netlink only, it does not depends on the obsolete netdevice (ioctl) API.
* Only the code of referenced inline functions enters in the object and executable code.

### Example

```C
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <nlinline.h>

int main(int argc, char *argv[]) {
  uint8_t ipv4addr[] = {192,168,2,2};
  uint8_t ipv4gw[] = {192,168,2,1};
  uint8_t ipv6addr[16] = {0x20, 0x01, 0x07, 0x60, [15] = 0x02};
  uint8_t ipv6gw[16] = {0x20, 0x01, 0x07, 0x60, [15] = 0x01};

  int ifindex = nlinline_if_nametoindex(argv[1]);
  if (ifindex > 0)
    printf("%d\n", ifindex);
  else {
    perror("nametoindex");
    return 1;
  }

  if (nlinline_linksetupdown(ifindex, 1) < 0)
    perror("link up");
  if (nlinline_ipaddr_add(AF_INET, ipv4addr, 24, ifindex) < 0)
    perror("addr ipv4");
  if (nlinline_iproute_add(AF_INET, NULL, 0, ipv4gw) < 0)
    perror("addr ipv6");
  if (nlinline_ipaddr_add(AF_INET6, ipv6addr, 64, ifindex) < 0)
    perror("route ipv4");
  if (nlinline_iproute_add(AF_INET6, NULL, 0, ipv6gw) < 0)
    perror("route ipv6");
  return 0;
}
```

This program takes the name of an interface from the command line. It turns that interface up and
sets the interface IPv4 and IPv6 addresses and default routes.

## how to install nlinline

Just put nlinline.h where you need it and that's all.
```
cp nlinline.h /usr/local/include
```
or
```
cp nlinline.h /usr/include
```

In case you are lazy and you like standard install methods and tools, there is a trivial CMake support. 
The standard cmake install sequence applies:
```
$ cmake .
$ sudo make install
```