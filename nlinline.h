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
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_link.h>
#include <linux/if_addr.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

static inline int nlinline_if_nametoindex(const char *ifname);
static inline int nlinline_linksetupdown(unsigned int ifindex, int updown);
static inline int nlinline_linksetaddr(unsigned int ifindex, void *macaddr);
static inline int nlinline_linkgetaddr(unsigned int ifindex, void *macaddr);
static inline int nlinline_linksetmtu(unsigned int ifindex, unsigned int mtu);

static inline int nlinline_ipaddr_add(int family, void *addr, int prefixlen, unsigned int ifindex);
static inline int nlinline_ipaddr_del(int family, void *addr, int prefixlen, unsigned int ifindex);

static inline int nlinline_iproute_add(int family, void *dst_addr, int dst_prefixlen, void *gw_addr, unsigned int ifindex);
static inline int nlinline_iproute_del(int family, void *dst_addr, int dst_prefixlen, void *gw_addr, unsigned int ifindex);

struct nl_iplink_data {
	int tag;
	int len;
	const void *data;
};

#define nl_iplink_strdata(t,s) &(struct nl_iplink_data) {(t), strlen(s) + 1, (s)}, 1

static inline int nlinline_iplink_add(const char *ifname, unsigned int ifindex, const char *type,
		struct nl_iplink_data *ifd, int nifd);
static inline int nlinline_iplink_del(const char *ifname, unsigned int ifindex);

static inline int nl_addrdata2prefix(unsigned char prefixlen, unsigned char flags, unsigned char scope);
static inline int nl_routedata2prefix(unsigned char prefixlen, unsigned char type, unsigned char scope);

#ifndef __NLINLINE_PLUSTYPE
#define __PLUSARG
#define __PLUSF
#define __PLUS
#define __nlinline_if_nametoindex nlinline_if_nametoindex
#define __nlinline_linksetupdown nlinline_linksetupdown
#define __nlinline_linksetaddr nlinline_linksetaddr
#define __nlinline_linkgetaddr nlinline_linkgetaddr
#define __nlinline_linksetmtu nlinline_linksetmtu
#define __nlinline_ipaddr_add nlinline_ipaddr_add
#define __nlinline_ipaddr_del nlinline_ipaddr_del
#define __nlinline_iproute_add nlinline_iproute_add
#define __nlinline_iproute_del nlinline_iproute_del
#define __nlinline_iplink_add nlinline_iplink_add
#define __nlinline_iplink_del nlinline_iplink_del
#define __nlinline_nldialog nlinline_nldialog
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

#define NLINLINE_ADDRDATA2PREFIX_MAGIC 0x70
#define NLINLINE_ROUTEDATA2PREFIX_MAGIC 0x60
static inline int nl_addrdata2prefix(unsigned char prefixlen, unsigned char flags, unsigned char scope) {
	return NLINLINE_ADDRDATA2PREFIX_MAGIC << 24 | flags << 16 | scope << 8 | prefixlen;
}

static inline int nl_routedata2prefix(unsigned char prefixlen, unsigned char type, unsigned char scope) {
	return NLINLINE_ROUTEDATA2PREFIX_MAGIC << 24 | type << 16 | scope << 8 | prefixlen;
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
		case NLMSG_ERROR: if (msg.e.error >= 0)
												return msg.e.error;
											else
												return errno = -msg.e.error, -1;
		case RTM_NEWLINK: return msg.i.ifi_index;
		default:          return errno = EFAULT, -1;
	}
}

static inline int __nlinline_open_send(__PLUSARG void *msg) {
	struct nlmsghdr *nlmsg = msg;
	struct sockaddr_nl sanl = {AF_NETLINK, 0, 0, 0};
	int fd;
#ifdef __NLINLINE_PLUSTYPE
  if (__PLUSF msocket)
    fd = __PLUSF msocket(__PLUSF mstack, AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
  else
#endif
    fd  = __PLUSF socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
  if (fd < 0)
    return fd;
  if (__PLUSF bind(fd, (struct sockaddr *) &sanl, sizeof(struct sockaddr_nl)) < 0)
    return __PLUSF close(fd), -1;
  if (__PLUSF send(fd, msg, nlmsg->nlmsg_len, 0) < 0)
    return __PLUSF close(fd), -1;
	return fd;
}

static inline int __nlinline_nldialog(__PLUSARG void *msg) {
  int ret_value;
  int fd = __nlinline_open_send(__PLUS msg);
	if (fd < 0)
		return fd;
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
	return __nlinline_nldialog(__PLUS &msg);
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
	return __nlinline_nldialog(__PLUS &msg);
}

struct __nlinline_macaddr {
  unsigned char byte[6];
};

struct __nlinline_macattr {
  struct nlattr h;
  struct __nlinline_macaddr addr;
};

static inline int __nlinline_linksetaddr(__PLUSARG unsigned int ifindex, void *macaddr) {
  struct {
    struct nlmsghdr h;
    struct ifinfomsg i;
    struct __nlinline_macattr mac;
  } msg = {
    .h.nlmsg_len = sizeof(msg),
    .h.nlmsg_type = RTM_NEWLINK,
    .h.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
    .h.nlmsg_seq = 1,
    .i.ifi_index = ifindex,
    .mac.h.nla_len = sizeof(struct __nlinline_macattr),
    .mac.h.nla_type = IFLA_ADDRESS,
    .mac.addr = *((struct __nlinline_macaddr *) macaddr)
  };
  return __nlinline_nldialog(__PLUS &msg);
}

static inline int __nlinline_linkgetaddr(__PLUSARG unsigned int ifindex, void *macaddr) {
  struct {
    struct nlmsghdr h;
    struct ifinfomsg i;
    unsigned char attrs[];
  } *reply, msg = {
    .h.nlmsg_len = sizeof(msg),
    .h.nlmsg_type = RTM_GETLINK,
    .h.nlmsg_flags = NLM_F_REQUEST,
    .h.nlmsg_seq = 1,
    .i.ifi_index = ifindex,
  };
  struct sockaddr_nl sanl = {AF_NETLINK, 0, 0, 0};
  int ret_value;
	int fd = __nlinline_open_send(__PLUS &msg);
  if (fd < 0)
    return fd;
  if ((ret_value = __PLUSF recv(fd, NULL, 0, MSG_PEEK|MSG_TRUNC)) < 0)
    return close(fd), -1;
  unsigned char buf[ret_value];
  if ((ret_value = __PLUSF recv(fd, buf, ret_value, 0)) < 0)
    return __PLUSF close(fd), -1;
  reply = (void *) buf;
  if (ret_value <= sizeof(msg.h) || ret_value < reply->h.nlmsg_len)
    return errno = EFAULT, __PLUSF close(fd), -1;
  if (reply->h.nlmsg_type != RTM_NEWLINK)
    return errno = ENODEV, __PLUSF close(fd), -1;
  unsigned char *limit = buf + reply->h.nlmsg_len;
  unsigned char *scan = reply->attrs;
  while (scan < limit) {
    struct nlattr *attr = (void *) scan;
    if (attr->nla_type == IFLA_ADDRESS && attr->nla_len >= 6) {
      memcpy(macaddr, attr + 1, 6);
      return __PLUSF close(fd), 0;
    }
    scan += NLMSG_ALIGN(attr->nla_len);
  }
  __PLUSF close(fd);
  return errno = ENOENT, -1;
}

struct __nlinline_u32 {
  struct nlattr h;
	__u32 value;
};

static inline int __nlinline_linksetmtu(__PLUSARG unsigned int ifindex, unsigned int mtu) {
	struct {
		struct nlmsghdr h;
		struct ifinfomsg i;
		struct __nlinline_u32 mtu;
	} msg = {
		.h.nlmsg_len = sizeof(msg),
		.h.nlmsg_type = RTM_NEWLINK,
		.h.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
		.h.nlmsg_seq = 1,
		.i.ifi_index = ifindex,
		.mtu.h.nla_len = sizeof(struct __nlinline_u32),
		.mtu.h.nla_type = IFLA_MTU,
		.mtu.value = mtu,
  };
	return __nlinline_nldialog(__PLUS &msg);
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
		int request, int xflags, int family, void *addr, int prefixlen, unsigned int ifindex) {
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
		if (prefixlen >> 24 == NLINLINE_ADDRDATA2PREFIX_MAGIC) {
			msg.i.ifa_scope = prefixlen >> 8;
			msg.i.ifa_flags = prefixlen >> 16;
		}
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
		return __nlinline_nldialog(__PLUS &msg);
	}
}

static inline int __nlinline_ipaddr_add(__PLUSARG
		int family, void *addr, int prefixlen, unsigned int ifindex) {
	return __nlinline_ipaddr(__PLUS
			RTM_NEWADDR, NLM_F_EXCL | NLM_F_CREATE, family, addr, prefixlen, ifindex);
}

static inline int __nlinline_ipaddr_del(__PLUSARG
		int family, void *addr, int prefixlen, unsigned int ifindex) {
	return __nlinline_ipaddr(__PLUS
			RTM_DELADDR, 0, family, addr, prefixlen, ifindex);
}

static inline int __nlinline_iproute(__PLUSARG
		int request, int xflags, int family, void *dst_addr, int dst_prefixlen, void *gw_addr, unsigned int ifindex) {
	int addrlen = nlinline_family2addrlen(family);
  if (addrlen == 0)
    return errno = EINVAL, -1;
  else {
    struct {
      struct nlmsghdr h;
      struct rtmsg r;
      struct __nlinline_u32 oif;
      union {
        struct __nlinline_ipv4attr a4[2];
        struct __nlinline_ipv6attr a6[2];
      };
    } msg = {
      .h.nlmsg_len = sizeof(msg.h) + sizeof(msg.r) + sizeof(msg.oif),
      .h.nlmsg_type = request,
      .h.nlmsg_flags =  NLM_F_REQUEST | NLM_F_ACK | xflags,
      .h.nlmsg_seq = 1,
      .r.rtm_family = family,
      .r.rtm_dst_len = dst_prefixlen,
			.r.rtm_table = RT_TABLE_MAIN,
			.r.rtm_protocol = RTPROT_BOOT,
			.r.rtm_scope = RT_SCOPE_UNIVERSE,
			.r.rtm_type = RTN_UNICAST,
			.oif.h.nla_type = (ifindex == 0) ? RTA_UNSPEC : RTA_OIF,
			.oif.h.nla_len = sizeof(msg.oif),
			.oif.value = ifindex,
		};
		if (dst_prefixlen >> 24 == NLINLINE_ROUTEDATA2PREFIX_MAGIC) {
			msg.r.rtm_scope = dst_prefixlen >> 8;
			msg.r.rtm_type = dst_prefixlen >> 16;
		}
		int nattr = 0;
    if (addrlen == 4) {
			if (dst_prefixlen > 0) {
				msg.a4[nattr].h.nla_len = sizeof(msg.a4[0]);
				msg.a4[nattr].h.nla_type = RTA_DST;
				msg.a4[nattr].addr = *((struct __nlinline_ipv4addr *)dst_addr);
				nattr++;
			}
			if (gw_addr != NULL) {
				msg.a4[nattr].h.nla_len = sizeof(msg.a4[0]);
				msg.a4[nattr].h.nla_type = RTA_GATEWAY;
				msg.a4[nattr].addr = *((struct __nlinline_ipv4addr *)gw_addr);
				nattr++;
			}
			msg.h.nlmsg_len += nattr * sizeof(msg.a4[0]);
    } else {
			if (dst_prefixlen > 0) {
				msg.a6[nattr].h.nla_len = sizeof(msg.a6[0]);
				msg.a6[nattr].h.nla_type = RTA_DST;
				msg.a6[nattr].addr = *((struct __nlinline_ipv6addr *)dst_addr);
				nattr++;
			}
			if (gw_addr != NULL) {
				msg.a6[nattr].h.nla_len = sizeof(msg.a6[0]);
				msg.a6[nattr].h.nla_type = RTA_GATEWAY;
				msg.a6[nattr].addr = *((struct __nlinline_ipv6addr *)gw_addr);
				nattr++;
			}
			msg.h.nlmsg_len += nattr * sizeof(msg.a6[0]);
		}
		return __nlinline_nldialog(__PLUS &msg);
	}
}

static inline int __nlinline_iproute_add(__PLUSARG
		int family, void *dst_addr, int dst_prefixlen, void *gw_addr, unsigned int ifindex) {
	return __nlinline_iproute(__PLUS
			RTM_NEWROUTE, NLM_F_EXCL | NLM_F_CREATE, family, dst_addr, dst_prefixlen, gw_addr, ifindex);
}

static inline int __nlinline_iproute_del(__PLUSARG
		int family, void *dst_addr, int dst_prefixlen, void *gw_addr, unsigned int ifindex) {
	return __nlinline_iproute(__PLUS
			RTM_DELROUTE, 0, family, dst_addr, dst_prefixlen, gw_addr, ifindex);
}

static inline int __nlinline_add_attr(void *buf, unsigned int type, const void *data, int datalen) {
	int attrlen = sizeof(struct nlattr) + datalen;
	if (buf) {
		struct nlattr *attr = buf;
		attr->nla_len = attrlen;
		attr->nla_type = type;
		memcpy(attr + 1, data, datalen);
	}
	return (attrlen + 3) & ~3;
}

static inline int __nlinline_add_strattr(void *buf, unsigned int type, const char *s) {
	if (s)
		return __nlinline_add_attr(buf, type, s, strlen(s) + 1);
	else
		return 0;
}

#define IFLA_VDE_VNL 1

/* [IFLA_IFNAME...] [IFLA_NEW_IFINDEX ""] [IFLA_LINKINFO [IFLA_INFO_KIND ...] [IFLA_INFO_DATA [..ifd.. */
static inline int __nlinline_iplink_add(__PLUSARG const char *ifname, unsigned int ifindex, const char *type, struct nl_iplink_data *ifd, int nifd) {
	int msglen = sizeof(struct nlmsghdr) + sizeof(struct ifinfomsg) + sizeof(struct nlattr) +
		(ifname ? __nlinline_add_strattr(NULL, IFLA_IFNAME, ifname) : 0) +
		__nlinline_add_strattr(NULL, IFLA_INFO_KIND, type) +
		((ifindex == -1) ? __nlinline_add_strattr(NULL, IFLA_NEW_IFINDEX, "") : 0) +
		(nifd > 0 ? sizeof(struct nlattr) : 0);
	for (int i = 0; i < nifd; i++)
		msglen += __nlinline_add_attr(NULL, ifd[i].tag, ifd[i].data, ifd[i].len);
	unsigned char msgbuf[msglen];
	unsigned char *rawmsg = msgbuf;
	struct {
		struct nlmsghdr h;
		struct ifinfomsg i;
	} *msg = (void *) rawmsg;
	struct nlattr *infohdr;
	memset(msgbuf, 0, msglen);
	msg->h.nlmsg_len = msglen;
	msg->h.nlmsg_type = RTM_NEWLINK;
	msg->h.nlmsg_flags = NLM_F_EXCL | NLM_F_CREATE | NLM_F_REQUEST | NLM_F_ACK;
	msg->h.nlmsg_seq = 1;
	msg->i.ifi_index = ifindex == -1 ? 0 : ifindex;
	rawmsg += sizeof(*msg);
	if (ifname)
		rawmsg += __nlinline_add_strattr(rawmsg, IFLA_IFNAME, ifname);
	if (ifindex == -1)
		rawmsg += __nlinline_add_strattr(rawmsg, IFLA_NEW_IFINDEX, "");
	infohdr = (void *) rawmsg;
	rawmsg += sizeof(*infohdr);
	infohdr->nla_type = IFLA_LINKINFO;
	rawmsg += __nlinline_add_strattr(rawmsg, IFLA_INFO_KIND, type);
	if (nifd > 0) {
		struct nlattr *datahdr = (void *) rawmsg;
		rawmsg += sizeof(*datahdr);
		datahdr->nla_type = IFLA_INFO_DATA;
		for (int i = 0; i < nifd; i++)
			rawmsg += __nlinline_add_attr(rawmsg, ifd[i].tag, ifd[i].data, ifd[i].len);
		datahdr->nla_len = rawmsg - (unsigned char *)datahdr;
	}
	infohdr->nla_len = rawmsg - (unsigned char *)infohdr;
	return __nlinline_nldialog(__PLUS &msgbuf);
}

static inline int __nlinline_iplink_del(__PLUSARG const char *ifname, unsigned int ifindex) {
	struct {
		struct nlmsghdr h;
		struct ifinfomsg i;
		struct nlattr a;
		char ifname[IFNAMSIZ];
	} msg = {
		.h.nlmsg_len = sizeof(msg.h) + sizeof(msg.i),
		.h.nlmsg_type = RTM_DELLINK,
		.h.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
		.h.nlmsg_seq = 1,
		.i.ifi_index = ifindex,
		.a.nla_type = IFLA_IFNAME,
	};
	if (ifname) {
		int namelen = snprintf(msg.ifname, IFNAMSIZ, "%s", ifname);
		msg.a.nla_len = sizeof(msg.a) + namelen + 1;
		msg.h.nlmsg_len += (msg.a.nla_len + 3) & ~3;
	}
	return __nlinline_nldialog(__PLUS &msg);
}

#endif
