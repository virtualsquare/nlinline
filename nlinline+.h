/*
 * nlinline plus: compact library of inline function providing the most common
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

#ifndef NLINLINEPLUS_H
#define NLINLINEPLUS_H
#ifdef NLINLINE_H
#error nlinline+.h must be included instead of nlinline.h
#else

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

/**************************
 * Implementation
 **************************/

typedef int msocket_t (void *, int, int, int);
struct _stackinfo {
	msocket_t *msocket;
	void *mstack;
  typeof(socket) *socket;
  typeof(bind) *bind;
  typeof(send) *send;
  typeof(recv) *recv;
  typeof(close) *close;
};

#define __NLINLINE_PLUSTYPE struct _stackinfo
#include <nlinline.h>

#define __LIB_STACKINFO(X, Y) \
  struct _stackinfo stackinfo = {\
		NULL, NULL, \
    X ## socket, \
    Y ## bind, \
    Y ## send, \
    Y ## recv, \
    Y ## close \
  }

#define __LIB_NLINLINE(NAME, X, Y) \
	static inline int NAME ## if_nametoindex(const char *ifname) {\
		__LIB_STACKINFO(X, Y); \
		return __nlinline_if_nametoindex(&stackinfo, ifname); \
	} \
	static inline int NAME ## linksetupdown(unsigned int ifindex, int updown) {\
		__LIB_STACKINFO(X, Y); \
		return __nlinline_linksetupdown(&stackinfo, ifindex, updown); \
	} \
	static inline int NAME ## ipaddr_add(int family, void *addr, int prefixlen, unsigned int ifindex) {\
		__LIB_STACKINFO(X, Y); \
		return __nlinline_ipaddr_add(&stackinfo, family, addr, prefixlen, ifindex); \
	} \
	static inline int NAME ## ipaddr_del(int family, void *addr, int prefixlen, unsigned int ifindex) {\
		__LIB_STACKINFO(X, Y); \
		return __nlinline_ipaddr_del(&stackinfo, family, addr, prefixlen, ifindex); \
	} \
	static inline int NAME ## iproute_add(int family, void *dst_addr, int dst_prefixlen, void *gw_addr) {\
		__LIB_STACKINFO(X, Y); \
		return __nlinline_iproute_add(&stackinfo, family, dst_addr, dst_prefixlen, gw_addr); \
	} \
	static inline int NAME ## iproute_del(int family, void *dst_addr, int dst_prefixlen, void *gw_addr) {\
		__LIB_STACKINFO(X, Y); \
		return __nlinline_iproute_del(&stackinfo, family, dst_addr, dst_prefixlen, gw_addr); \
	} \
	static inline int NAME ## iplink_add(const char *ifname, unsigned int ifindex, const char *type, const char *data) {\
		__LIB_STACKINFO(X, Y); \
		return __nlinline_iplink_add(&stackinfo, ifname, ifindex, type, data); \
	} \
	static inline int NAME ## iplink_del(const char *ifname, unsigned int ifindex) {\
		__LIB_STACKINFO(X, Y); \
		return __nlinline_iplink_del(&stackinfo, ifname, ifindex); \
	}

#define NLINLINE_LIB(X) __LIB_NLINLINE(X, X, X)
#define NLINLINE_LIBCOMP(X) __LIB_NLINLINE(X, X, )

#define __LIBMULTI_STACKINFO(X, Y, MSTACK) \
  struct _stackinfo stackinfo = {\
		(msocket_t *) X ## msocket, \
		MSTACK, \
		NULL, \
    Y ## bind, \
    Y ## send, \
    Y ## recv, \
    Y ## close \
  }

#define __LIBMULTI_NLINLINE(X, Y) \
	static inline int X ## if_nametoindex(void *mstack, const char *ifname) {\
		__LIBMULTI_STACKINFO(X, Y, mstack); \
		return __nlinline_if_nametoindex(&stackinfo, ifname); \
	} \
	static inline int X ## linksetupdown(void *mstack, unsigned int ifindex, int updown) {\
		__LIBMULTI_STACKINFO(X, Y, mstack); \
		return __nlinline_linksetupdown(&stackinfo, ifindex, updown); \
	} \
	static inline int X ## ipaddr_add(void *mstack, int family, void *addr, int prefixlen, int ifindex) {\
		__LIBMULTI_STACKINFO(X, Y, mstack); \
		return __nlinline_ipaddr_add(&stackinfo, family, addr, prefixlen, ifindex); \
	} \
	static inline int X ## ipaddr_del(void *mstack, int family, void *addr, int prefixlen, int ifindex) {\
		__LIBMULTI_STACKINFO(X, Y, mstack); \
		return __nlinline_ipaddr_del(&stackinfo, family, addr, prefixlen, ifindex); \
	} \
	static inline int X ## iproute_add(void *mstack, int family, void *dst_addr, int dst_prefixlen, void *gw_addr) {\
		__LIBMULTI_STACKINFO(X, Y, mstack); \
		return __nlinline_iproute_add(&stackinfo, family, dst_addr, dst_prefixlen, gw_addr); \
	} \
	static inline int X ## iproute_del(void *mstack, int family, void *dst_addr, int dst_prefixlen, void *gw_addr) {\
		__LIBMULTI_STACKINFO(X, Y, mstack); \
		return __nlinline_iproute_del(&stackinfo, family, dst_addr, dst_prefixlen, gw_addr); \
	} \
	static inline int X ## iplink_add(void *mstack, const char *ifname, unsigned int ifindex, const char *type, const char *data) {\
		__LIBMULTI_STACKINFO(X, Y, mstack); \
		return __nlinline_iplink_add(&stackinfo, ifname, ifindex, type, data); \
	} \
	static inline int X ## iplink_del(void *mstack, const char *ifname, unsigned int ifindex) {\
		__LIBMULTI_STACKINFO(X, Y, mstack); \
		return __nlinline_iplink_del(&stackinfo, ifname, ifindex); \
	}

#define NLINLINE_LIBMULTI(X) __LIBMULTI_NLINLINE(X, X)
#define NLINLINE_LIBMULTICOMP(X) __LIBMULTI_NLINLINE(X, )

/* define the standard inline functions nlinline_...  */
__LIB_NLINLINE(nlinline_,,)

#endif
#endif
