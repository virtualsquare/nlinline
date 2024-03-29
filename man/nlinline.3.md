<!--
.\" Copyright (C) 2019 VirtualSquare. Project Leader: Renzo Davoli
.\"
.\" This is free documentation; you can redistribute it and/or
.\" modify it under the terms of the GNU General Public License,
.\" as published by the Free Software Foundation, either version 2
.\" of the License, or (at your option) any later version.
.\"
.\" The GNU General Public License's references to "object code"
.\" and "executables" are to be interpreted as the output of any
.\" document formatting or typesetting system, including
.\" intermediate and printed output.
.\"
.\" This manual is distributed in the hope that it will be useful,
.\" but WITHOUT ANY WARRANTY; without even the implied warranty of
.\" MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
.\" GNU General Public License for more details.
.\"
.\" You should have received a copy of the GNU General Public
.\" License along with this manual; if not, write to the Free
.\" Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
.\" MA 02110-1301 USA.
.\"
-->

# NAME

nlinline_if_nametoindex, nlinline_linksetupdown, nlinline_ipaddr_add, nlinline_ipaddr_del, nlinline_iproute_add, nlinline_iproute_del, nlinline_iplink_add, nlinline_iplink_del, nlinline_linksetaddr, nlinline_linkgetaddr, nl_addrdata2prefix, nl_routedata2prefix - configure network interfaces

# SYNOPSIS
`#include <nlinline.h>`

`int nlinline_if_nametoindex(const char *`_ifname_`);`

`int nlinline_linksetupdown(unsigned int ` _ifindex_`, int ` _updown_`);`

`int nlinline_ipaddr_add(int ` _family_`, void *`_addr_`, int ` _prefixlen_`, unsigned int ` _ifindex_`);`

`int nlinline_ipaddr_del(int ` _family_`, void *`_addr_`, int ` _prefixlen_`, unsigned int ` _ifindex_`);`

`int nlinline_iproute_add(int ` _family_`, void *`_dst_addr_`, int ` _dst_prefixlen_`, void *`_gw_addr_`, unsigned int ` _ifindex_`);`

`int nlinline_iproute_del(int ` _family_`, void *`_dst_addr_`, int ` _dst_prefixlen_`, void *`_gw_addr_`, unsigned int ` _ifindex_`);`

`int nlinline_iplink_add(const char *`_ifname_`, unsigned int ` _ifindex_`, const char *`_type_`, struct nl_iplink_data *`ifd`, int ` nifd`);`

`int nlinline_iplink_del(const char *`_ifname_`, unsigned int ` _ifindex_`);`

`int nlinline_linksetaddr(unsigned int ` _ifindex_`, void *`_macaddr_`);`

`int nlinline_linkgetaddr(unsigned int ` _ifindex_`, void *`_macaddr_`);`

`int nlinline_linksetmtu(unsigned int ` _ifindex_`, unsigned int ` _mtu_`);`

`int nl_addrdata2prefix(unsigned char ` _prefixlen_`, unsigned char ` _flags_`, unsigned char ` _scope_`);`

`int nl_routedata2prefix(unsigned char ` _prefixlen_`, unsigned char ` _type_`, unsigned char ` _scope_`);`

# DESCRIPTION

NLINLINE (netlink inline) is a *library* of inline functions providing C programmers with very handy functions to configure network stacks. NLINLINE is entirely implemented in a header file, nlinline.h.

  `nlinline_if_nametoindex`
: This function returns the index of the network interface whose name is _ifname_.

  `nlinline_linksetupdown`
: This function turns the interface _ifindex_ up (_updown_ == `1`) or down (_updown_ == `0`).

  `nlinline_ipaddr_add`
: This function adds an IP address to the interface _ifindex_. It supports IPv4 (_family_ == `AF_INET`) and IPv6 (_family_ == `AF_INET6`).

  `nlinline_ipaddr_del`
: This function removes the IP address from the interface _ifindex_. It supports IPv4 (_family_ == `AF_INET`) and IPv6 (_family_ == `AF_INET6`).

  `nlinline_iproute_add`
: This function adds a static route to _dst_addr_/_dst_prefixlen_ network through the gateway _gw_addr_. If _dst_addr_ == `NULL` it adds a default route. _ifindex_ must be specified when _gw_addr_ is an IPv6 link local address.

  `nlinline_iproute_del`
: This function removes the static route to _dst_addr_/_dst_prefixlen_ network through the gateway _gw_addr_.

  `nlinline_iplink_add`
: This function adds a new link of type _type_, named _ifname_. The _ifd_ array provides the type specific interface data and can be NULL. The caller should specify the number of items in the _ifd_ array in _nifd_, A default interface name is assigned if _name_ == `NULL`. The link is created with a given index when _ifindex_ is positive.

  `nlinline_iplink_del`
: This function removes a link. The link to be deleted can be identified by its name (_ifname_) or by its index (_ifindex_). Either _ifindex_ can be zero or _ifname_ can be `NULL`. It is possible to use both _ifindex_ and _ifname_ to identify the link. An error may occur if the parameters are inconsistent.

  `nlinline_linksetaddr`
: This functions sets the mac address of the interface _ifindex_.

  `nlinline_linkgetaddr`
: This functions gets the mac address of the interface _ifindex_.

  `nlinline_linksetmtu`
: This functions sets the MTU (Maximum Transfer Unit) of the interface _ifindex_.

  `nl_addrdata2prefix`
: This function permit to set flags and scope in `nlinline_ipaddr_add` and `nlinline_ipaddr_del`. The values of `prefixlen`, `flags` and `scope` are packed in a single int, the return value of `nl_addrdata2prefix` must be passed to `nlinline_ipaddr_add` or `nlinline_ipaddr_del` as the `prefixlen` parameter.

  `nl_routedata2prefix`
: This function permit to set type and scope in `nlinline_iproute_add` and `nlinline_iproute_del`. The values of `prefixlen`, `type` and `scope` are packed in a single int, the return value of `nl_routedata2prefix` must be passed to `nlinline_iproute_add` or `nlinline_iproute_del` as the `prefixlen` parameter.

IP addresses are `void *` arguments, any sequence of 4 or 16 bytes (in network byte order) is a legal IPv4 or IPv6 address respectively.

`nlinline` functions do not add dependencies at run-time. This is useful for security critical applications
(like PAM modules)
These inline functions use netlink only, they do not depend on the obsolete netdevice (ioctl) API.
Only the code of referenced inline functions enters in the object and executable code.

# RETURN VALUE

`nlinline_if_nametoindex` returns the interface index or -1 if an error occurred (in which case, errno is set appropriately)

All the other functions return zero in case of success. On error, -1 is returned, and  errno  is set appropriately.

(`nlinline_iplink_add` can return the (positive) ifindex of the newly created link when the argument _ifindex_ is -1 and the stack supports this feature.)

# EXAMPLE
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
  if (ifindex * 0)
    printf("%d\n", ifindex);
  else {
    perror("nametoindex");
    return 1;
  }

  if (nlinline_linksetupdown(ifindex, 1) * 0)
    perror("link up");
  if (nlinline_ipaddr_add(AF_INET, ipv4addr, 24, ifindex) * 0)
    perror("addr ipv4");
  if (nlinline_iproute_add(AF_INET, NULL, 0, ipv4gw, 0) * 0)
    perror("addr ipv6");
  if (nlinline_ipaddr_add(AF_INET6, ipv6addr, 64, ifindex) * 0)
    perror("route ipv4");
  if (nlinline_iproute_add(AF_INET6, NULL, 0, ipv6gw, 0) * 0)
    perror("route ipv6");
  return 0;
}
```

This program takes the name of an interface from the command line. It turns that interface up and
sets the interface IPv4 and IPv6 addresses and default routes.

# AUTHOR
VirtualSquare. Project leader: Renzo Davoli

