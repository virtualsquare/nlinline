# nlinline
A quick and clean API for NetLink networking configuring.

NLINLINE (netlink inline) is a *library* of inline functions providing C programmers with very handy functions to configure network stacks. NLINLINE is entirely implemented in a header file, nlinline.h.

NLinline provides the following functions:

* `nlinline_if_nametoindex(const char *ifname);` return the index of the network interface whose name is `ifname`

* `int nlinline_iplink_add(const char *ifname, unsigned int ifindex, const char *type, const char *data);` add a (virtual) link

* `int nlinline_iplink_del(const char *ifname, unsigned int ifindex);` remove a link

* `int nlinline_linksetupdown(unsigned int ifindex, int updown);` turn the interface `ifindex` up (`updown == 1`) or down (`updown == 0`).

* `int nlinline_ipaddr_add(int family, void *addr, int prefixlen, int ifindex)` add an IP address to the interface `ifindex`. It supports IPv4 (`family == AF_INET`) and IPv6 `(family == AF_INET6)`.

* `int nlinline_ipaddr_del(int family, void *addr, int prefixlen, int ifindex)` remove the IP address from the interface `ifindex`. It supports IPv4 (`family == AF_INET`) and IPv6 `(family == AF_INET6)`.

* `int nlinline_iproute_add(int family, void *dst_addr, int dst_prefixlen, void *gw_addr, int ifindex);` add a static route to `dst_addr`/`dst_prefixlen` network through the gateway `gw_addr`. If `dst_addr == NULL` it adds a default route. `ifindex` must be specified when `gw_addr` is an IPv6 link local address.

* `int nlinline_iproute_del(int family, void *dst_addr, int dst_prefixlen, void *gw_addr, int ifindex);` remove the static route to `dst_addr`/`dst_prefixlen` network through the gateway `gw_addr`.

* `int nlinline_linksetaddr(unsigned int ifindex, void *macaddr);` set the MAC address of the interface `ifindex`.

* `int nlinline_linkgetaddr(unsigned int ifindex, void *macaddr);` get the MAC address of the interface `ifindex`.

* `int nlinline_linksetmtu(unsigned int ifindex, unsigned int mtu);` set the MTU of the interface `ifindex`.

IP addresses are `void *` arguments, any sequence of 4 or 16 bytes (in network byte order) is a legal IPv4 or IPv6 address respectively.

### Netlink + inline

* NLINLINE is a simple way to configure networking for network namespaces and *Internet of Threads* programs.
* NLINLINE does not add dependencies at run-time. It is useful for security critical applications (like PAM modules)
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
  if (nlinline_iproute_add(AF_INET, NULL, 0, ipv4gw, 0) < 0)
    perror("route ipv4");
  if (nlinline_ipaddr_add(AF_INET6, ipv6addr, 64, ifindex) < 0)
    perror("addr ipv6");
  if (nlinline_iproute_add(AF_INET6, NULL, 0, ipv6gw, 0) < 0)
    perror("route ipv6");
  return 0;
}
```

This program takes the name of an interface from the command line. It turns that interface up and
sets the interface IPv4 and IPv6 addresses and default routes.

### data2prefix: add more args to ipaddr and iproute

```
static inline int nl_addrdata2prefix(unsigned char prefixlen, unsigned char flags, unsigned char scope);
static inline int nl_routedata2prefix(unsigned char prefixlen, unsigned char type, unsigned char scope);
```

These functions permit to add more configuration flags and parameters to `nlinline_ipaddr*` and `nlinline_iproute*`.
The output of these functions should be used as the prefix arg in `nlinline_ipaddr*` and `nlinline_iproute*`.

For example the line to set the IPv6 address in the example above can be change as follows to disable the DAD protocol:
```
  if (nlinline_ipaddr_add(AF_INET6, ipv6addr, nl_addrdata2prefix(64, IFA_F_NODAD, RT_SCOPE_UNIVERSE), ifindex) < 0)
    perror("addr ipv6");
```

## nlinline extended to user-mode stacks: nlinline+

The header file `nlinline+.h` implements an extended version of nlinline providing the support for user-mode networking stacks available as libraries.

`nlinline+.h` basically provides the same function as `nlinline.h` in a more flexible implementation. `nlinline+.h` includes a set of macro which define the same inline functions as nlinkine for library-based networking stacks.

```
NLINLINE_LIB(mystack)
``` 
defines the functions 
`mystack_if_nametoindex`, `mystack_linksetupdown`... using `mystack_socket` instead of `socket`, `mystack_bind` instead of `bind` etc.

```
NLINLINE_LIBCOMP(yourstack)
```
is similar to `NLINLINE_LIB`: it defines `yourstack_if_nametoindex`, `yourstack_linksetupdown`... 
but it uses only `yourstack_socket` instead of `socket`, while `bind`, `close`, `recv`, `send` are the standard system calls.

```
NLINLINE_LIBMULTI(hisstack)
```
is for librariesproviding multi-stack support. The functions `hisstack_if_nametoindex`,
`hisstack_linksetupdown` have one more (leading) argument: a generic pointer used as stack identifier. These functions use `hisstack_bind`, `hisstack_recv`...
Instead of `socket`, `hisstack_msocket` is used: it has one more (leading) argument which is the stack identifier.

```
NLINLINE_LIBMULTICOMP(herstack)
```
combines the characteristics of `NLINLINE_LIBCOMP` and those of `NLINLINE_LIBMULTI`, it is for libraries providing multi-stack support whose file descriptors are compatible with standard system calls. All the function generated by `NLINLINE_LIBMULTICOMP` have one more argument as in `NLINLINE_LIBMULTI`, `herstack_msocket` is used instead of `socket`,  while `bind`, `close`, `recv`, `send` are the standard system calls.

### Example

As an example of usage, programs using the [libvdestack](https://github.com/rd235/libvdestack) library can have nlinline support:

```
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <stdint.h>
#include <unistd.h>
#include <vdestack.h>
#include <nlinline+.h>

NLINLINE_LIBMULTICOMP(vde_)

int main(int argc, char *argv[]) {
  int rv;
    uint8_t ipv4addr[] = {192,168,2,2};
  uint8_t ipv4gw[] = {192,168,2,1};
  uint8_t ipv6addr[16] = {0x20, 0x01, 0x07, 0x60, [15] = 0x02};
  uint8_t ipv6gw[16] = {0x20, 0x01, 0x07, 0x60, [15] = 0x01};

  struct vdestack *stack = vde_addstack(argv[1], NULL);

  int ifindex = vde_if_nametoindex(stack, "vde0");
  if (ifindex > 0)
    printf("%d\n", ifindex);
  else {
    perror("nametoindex");
    return 1;
  }

  if (vde_linksetupdown(stack, ifindex, 1) < 0)
    perror("link up");
  if (vde_ipaddr_add(stack, AF_INET, ipv4addr, 24, ifindex) < 0)
    perror("addr ipv4");
  if (vde_iproute_add(stack, AF_INET, NULL, 0, ipv4gw) < 0)
    perror("route ipv4");
  if (vde_ipaddr_add(stack, AF_INET6, ipv6addr, 64, ifindex) < 0)
    perror("addr ipv6");
  if (vde_iproute_add(stack, AF_INET6, NULL, 0, ipv6gw, 0) < 0)
    perror("route ipv6");

  /* use the stack */

  vde_delstack(stack);
}

```

Functions like `vde_linksetupdown`, `vde_if_nametoindex`,... have been defined by
`NLINLINE_LIBMULTICOMP`.

## how to install nlinline

Just put `nlinline.h` (and `nlinline+.h`) where you need it and that's all.
```
cp nlinline.h nlinline+.h /usr/local/include
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
