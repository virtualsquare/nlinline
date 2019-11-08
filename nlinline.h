/*
 * nlinline: compact library of inline function providing the most common
 * network configuration operations.
 *
 * Copyright (C) 2019  Renzo Davoli <renzo@cs.unibo.it> VirtualSquare team.
 *
 * nlinline is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NLINLINE_H
#define NLINLINE_H

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_link.h>
#include <linux/if_addr.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

static inline int nlinline_if_nametoindex(const char *ifname);
static inline int nlinline_linksetupdown(unsigned int ifindex, int updown);

static inline int nlinline_ipaddr_add(int family, void *addr, int prefixlen, int ifindex);
static inline int nlinline_ipaddr_del(int family, void *addr, int prefixlen, int ifindex);

static inline int nlinline_iproute_add(int family, void *dst_addr, int dst_prefixlen, void *gw_addr);
static inline int nlinline_iproute_del(int family, void *dst_addr, int dst_prefixlen, void *gw_addr);

#ifndef __NLINLINE_PLUSTYPE
#define __PLUSARG
#define __PLUSF
#define __PLUS
#define __nlinline_if_nametoindex nlinline_if_nametoindex
#define __nlinline_linksetupdown nlinline_linksetupdown
#define __nlinline_ipaddr_add nlinline_ipaddr_add
#define __nlinline_ipaddr_del nlinline_ipaddr_del
#define __nlinline_iproute_add nlinline_iproute_add
#define __nlinline_iproute_del nlinline_iproute_del
#else
#define __PLUSARG __NLINLINE_PLUSTYPE *__stack,
#define __PLUSF __stack->
#define __PLUS __stack,
#endif

/**************************
 * Implementation
 **************************/

static inline int nlinline_family2addrlen(int family) {
	switch (family) {
		case AF_INET: return 4;
		case AF_INET6: return 16;
		default: return 0;
		__default: return 0;
	}
}

static inline int __nlinline_geterror(__PLUSARG int fd) {
	struct {
		struct nlmsghdr h;
		union {
			struct nlmsgerr e;
			struct ifinfomsg i;
		};
	} msg;
	int replylen = __PLUSF recv(fd, &msg, sizeof(msg), 0);
	if (replylen < 0)
		return -1;
	if (replylen <= sizeof(msg.h))
		return errno = EFAULT, -1;
	switch (msg.h.nlmsg_type) {
		case NLMSG_ERROR: return (msg.e.error == 0) ? 0 : (errno = -msg.e.error, -1);
		case RTM_NEWLINK: return msg.i.ifi_index;
		default:          return errno = EFAULT, -1;
	}
}

static inline int __nlinline_conversation(__PLUSARG void *msg) {
	struct nlmsghdr *nlmsg = msg;
	struct sockaddr_nl sanl = {AF_NETLINK, 0, 0, 0};
  int ret_value;
  int fd;
#ifdef __NLINLINE_PLUSTYPE
	if (__PLUSF msocket)
		fd = __PLUSF msocket(__PLUSF mstack, AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
	else
#endif
		fd	= __PLUSF socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
  if (fd < 0)
    return fd;
  if (__PLUSF bind(fd, (struct sockaddr *) &sanl, sizeof(struct sockaddr_nl)) < 0)
    return __PLUSF close(fd), -1;
  if (__PLUSF send(fd, msg, nlmsg->nlmsg_len, 0) < 0)
    return __PLUSF close(fd), -1;
  ret_value = __nlinline_geterror(__PLUS fd);
  __PLUSF close(fd);
  return ret_value;
}

static inline int __nlinline_if_nametoindex(__PLUSARG const char *ifname) {
	struct {
    struct nlmsghdr h;
    struct ifinfomsg i;
		struct nlattr a;
		char ifname[IFNAMSIZ];
  } msg = {
    .h.nlmsg_len = sizeof(msg.h) + sizeof(msg.i),
    .h.nlmsg_type = RTM_GETLINK,
    .h.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
    .h.nlmsg_seq = 1,
		.a.nla_type = IFLA_IFNAME,
	};
	int namelen = snprintf(msg.ifname, IFNAMSIZ, "%s", ifname);
	msg.a.nla_len = sizeof(msg.a) + namelen + 1;
	msg.h.nlmsg_len += (msg.a.nla_len + 3) & ~3;
	return __nlinline_conversation(__PLUS &msg);
}

static inline int __nlinline_linksetupdown(__PLUSARG unsigned int ifindex, int updown) {
	struct {
		struct nlmsghdr h;
		struct ifinfomsg i;
	} msg = {
		.h.nlmsg_len = sizeof(msg),
		.h.nlmsg_type = RTM_SETLINK,
		.h.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
		.h.nlmsg_seq = 1,
		.i.ifi_index = ifindex,
		.i.ifi_flags = (updown) ? IFF_UP : 0,
		.i.ifi_change=IFF_UP };
	return __nlinline_conversation(__PLUS &msg);
}

struct __nlinline_ipv4addr {
	unsigned char byte[4];
};

struct __nlinline_ipv6addr {
	unsigned char byte[16];
};

struct __nlinline_ipv4attr {
	struct nlattr h;
	struct __nlinline_ipv4addr addr;
};

struct __nlinline_ipv6attr {
	struct nlattr h;
	struct __nlinline_ipv6addr addr;
};

static inline int __nlinline_ipaddr(__PLUSARG
		int request, int xflags, int family, void *addr, int prefixlen, int ifindex) {
	int addrlen = nlinline_family2addrlen(family);
	if (addrlen == 0)
		return errno = EINVAL, -1;
	else {
		struct {
			struct nlmsghdr h;
			struct ifaddrmsg i;
			union {
				struct __nlinline_ipv4attr a4[2];
				struct __nlinline_ipv6attr a6[2];
			};
		}	msg = {
			.h.nlmsg_len = sizeof(msg.h) + sizeof(msg.i),
			.h.nlmsg_type = request,
			.h.nlmsg_flags =  NLM_F_REQUEST | NLM_F_ACK | xflags,
			.h.nlmsg_seq = 1,
			.i.ifa_family = family,
			.i.ifa_prefixlen = prefixlen,
			.i.ifa_scope = RT_SCOPE_UNIVERSE,
			.i.ifa_index = ifindex};
		if (addrlen == 4) {
			msg.a4[0].h.nla_len = msg.a4[1].h.nla_len = sizeof(struct nlattr) + addrlen;
			msg.a4[0].h.nla_type = IFA_LOCAL;
			msg.a4[1].h.nla_type = IFA_ADDRESS;
			msg.a4[0].addr = msg.a4[1].addr = *((struct __nlinline_ipv4addr *) addr);
			msg.h.nlmsg_len += 2 * sizeof(msg.a4[0]);
		} else {
			msg.a6[0].h.nla_len = msg.a6[1].h.nla_len = sizeof(struct nlattr) + addrlen;
			msg.a6[0].h.nla_type = IFA_LOCAL;
			msg.a6[1].h.nla_type = IFA_ADDRESS;
			msg.a6[0].addr = msg.a6[1].addr = *((struct __nlinline_ipv6addr *) addr);
			msg.h.nlmsg_len += 2 * sizeof(msg.a6[0]);
		}
		return __nlinline_conversation(__PLUS &msg);
	}
}

static inline int __nlinline_ipaddr_add(__PLUSARG
		int family, void *addr, int prefixlen, int ifindex) {
	return __nlinline_ipaddr(__PLUS
			RTM_NEWADDR, NLM_F_EXCL | NLM_F_CREATE, family, addr, prefixlen, ifindex);
}

static inline int __nlinline_ipaddr_del(__PLUSARG
		int family, void *addr, int prefixlen, int ifindex) {
	return __nlinline_ipaddr(__PLUS
			RTM_DELADDR, 0, family, addr, prefixlen, ifindex);
}

static inline int __nlinline_iproute(__PLUSARG
		int request, int xflags, int family, void *dst_addr, int dst_prefixlen, void *gw_addr) {
	int addrlen = nlinline_family2addrlen(family);
  if (addrlen == 0)
    return errno = EINVAL, -1;
  else {
    struct {
      struct nlmsghdr h;
      struct rtmsg r;
      union {
        struct __nlinline_ipv4attr a4[2];
        struct __nlinline_ipv6attr a6[2];
      };
    } msg = {
      .h.nlmsg_len = sizeof(msg.h) + sizeof(msg.r) + 2 * sizeof(struct nlattr) + 2 * addrlen,
      .h.nlmsg_type = request,
      .h.nlmsg_flags =  NLM_F_REQUEST | NLM_F_ACK | xflags,
      .h.nlmsg_seq = 1,
      .r.rtm_family = family,
      .r.rtm_dst_len = dst_prefixlen,
			.r.rtm_table = RT_TABLE_MAIN,
			.r.rtm_protocol = RTPROT_BOOT,
			.r.rtm_scope = RT_SCOPE_UNIVERSE,
			.r.rtm_type = RTN_UNICAST};
		int nattr = 0;
    if (addrlen == 4) {
			if (dst_prefixlen > 0) {
				msg.a4[nattr].h.nla_len = sizeof(struct nlattr) + addrlen;
				msg.a4[nattr].h.nla_type = RTA_DST;
				msg.a4[nattr].addr = *((struct __nlinline_ipv4addr *)dst_addr);
				nattr++;
			}
			msg.a4[nattr].h.nla_len = sizeof(struct nlattr) + addrlen;
			msg.a4[nattr].h.nla_type = RTA_GATEWAY;
			msg.a4[nattr].addr = *((struct __nlinline_ipv4addr *)gw_addr);
			nattr++;
			msg.h.nlmsg_len += nattr * sizeof(msg.a4[0]);
    } else {
			if (dst_prefixlen > 0) {
				msg.a6[nattr].h.nla_len = sizeof(struct nlattr) + addrlen;
				msg.a6[nattr].h.nla_type = RTA_DST;
				msg.a6[nattr].addr = *((struct __nlinline_ipv6addr *)dst_addr);
				nattr++;
			}
			msg.a6[nattr].h.nla_len = sizeof(struct nlattr) + addrlen;
			msg.a6[nattr].h.nla_type = RTA_GATEWAY;
			msg.a6[nattr].addr = *((struct __nlinline_ipv6addr *)gw_addr);
			nattr++;
			msg.h.nlmsg_len += nattr * sizeof(msg.a6[0]);
		}
		return __nlinline_conversation(__PLUS &msg);
	}
}

static inline int __nlinline_iproute_add(__PLUSARG
		int family, void *dst_addr, int dst_prefixlen, void *gw_addr) {
	return __nlinline_iproute(__PLUS
			RTM_NEWROUTE, NLM_F_EXCL | NLM_F_CREATE, family, dst_addr, dst_prefixlen, gw_addr);
}

static inline int __nlinline_iproute_del(__PLUSARG
		int family, void *dst_addr, int dst_prefixlen, void *gw_addr) {
	return __nlinline_iproute(__PLUS
			RTM_DELROUTE, 0, family, dst_addr, dst_prefixlen, gw_addr);
}
#endif
