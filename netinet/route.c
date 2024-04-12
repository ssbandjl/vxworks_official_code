/* route.c - packet routing routines */

/* Copyright 1984 - 1999 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/*
 * Copyright (c) 1980, 1986, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)route.c	8.3 (Berkeley) 1/9/95
 */

/*
modification history
--------------------
01p,04apr00,pul  removing fastpath IP references
01o,27mar00,pul  remove fastpath references
01n,04jan00,pul  Fast Path IP modifications: call to fpDelRouteEntry
01m,19oct99,pul  extending struct rtentry
01l,04mar99,spm  fixed errors in SPR #23301 fix causing build failures
01k,02mar99,spm  eliminated EEXIST error caused by ARP entries (SPR #23301)
01j,26aug98,n_s  added return val check for mBufClGet in rtinit. spr #22238.
01i,07jul97,rjc  newer version of rtentry incorporated.
01h,01jul97,vin  added route socket message hooks for scalability, fixed
		 warnings.
01g,30jun97,rjc  restored old rtentry size.
01f,03jun97,rjc  netmask with RTF_HOST set to 0.
01e,13feb97,rjc  more changes for tos routing,
01e,05feb97,rjc  changes for tos routing,
01d,05dec96,vin  moved ifafree() to if.c
01c,22nov96,vin  added cluster support replaced m_get(..) with mBufClGet(..).
01b,24sep96,vin  rtioctl() fixed for multicast addresses.
01a,03mar96,vin  created from BSD4.4 stuff. added rtioctl for backward
		 compatibility. moved ifa_ifwithroute() to if.c.
*/

#include "vxWorks.h"
#include "logLib.h"
#include "net/mbuf.h"
#include "net/domain.h"
#include "net/protosw.h"
#include "sys/socket.h"
#include "sys/ioctl.h"
#include "net/socketvar.h"
#include "errno.h"

#include "net/if.h"
#include "net/route.h"
#include "net/systm.h"
#include "net/raw_cb.h"

#include "routeEnhLib.h"

#include "netinet/in.h"
#include "netinet/in_var.h"



#ifdef NS
#include <netns/ns.h>
#endif

/* externs */
int	tickGet();

#define	SA(p) ((struct sockaddr *)(p))
#define SOCKADDR_IN(s) (((struct sockaddr_in*)(s))->sin_addr.s_addr)

struct	route_cb route_cb;
struct	rtstat	rtstat;
struct	radix_node_head *rt_tables[AF_MAX+1];

int	rttrash;		/* routes not in table but not freed */
int     rtmodified = 0;		/* route table modified */
struct	sockaddr wildcard;	/* zero valued cookie for wildcard searches */

struct radix_node * routeSwap (struct radix_node_head *, struct rtentry *,
                               struct sockaddr *, struct sockaddr *);

void
rtable_init(table)
	void **table;
{
	struct domain *dom;
	for (dom = domains; dom; dom = dom->dom_next)
		if (dom->dom_rtattach)
			dom->dom_rtattach(&table[dom->dom_family],
			    dom->dom_rtoffset);
}

int
route_init()
{
	rn_init();	/* initialize all zeroes, all ones, mask table */
	rtable_init((void **)rt_tables);
	return (OK);
}

/*
 * Packet routing routines.
 */
void
rtalloc(ro)
	register struct route *ro;
{
	if (ro->ro_rt && ro->ro_rt->rt_ifp && (ro->ro_rt->rt_flags & RTF_UP))
		return;				 /* XXX */
	ro->ro_rt = rtalloc1(&ro->ro_dst, 1);
}

struct rtentry* rtalloc1
    (
    struct sockaddr* dst, 
    int report
    )
    {
        register struct radix_node_head *rnh = rt_tables[dst->sa_family];
        register struct rtentry *rt;
        register struct radix_node *rn;
        struct rtentry *newrt = 0;
        struct rt_addrinfo info;
        int  s = splnet(), err = 0, msgtype = RTM_MISS;
        struct rtentry *   tosRt = NULL;
        uint16_t     savedTos;

      /*
       * save original tos since we overwrite it temporarily in the
       * dst socketaddr
       */

       savedTos = TOS_GET (dst);

match:

        if (rnh && (rn = rnh->rnh_matchaddr((caddr_t)dst, rnh)) &&
            ((rn->rn_flags & RNF_ROOT) == 0)) {
                newrt = rt = (struct rtentry *)rn;
                if (report && (rt->rt_flags & RTF_CLONING)) {

                         err = rtrequest(RTM_RESOLVE, dst, SA(0),
                                              SA(0), 0, &newrt);
                        if (err) {
                                newrt = rt;
                                rt->rt_refcnt++;
                                goto miss;
                        }
                        if ((rt = newrt) && (rt->rt_flags & RTF_XRESOLVE)) {
                                msgtype = RTM_RESOLVE;
                                goto miss;
                        }
                } else
                        rt->rt_refcnt++;
        } else {
                rtstat.rts_unreach++;
        miss:   if (report) {
                        bzero((caddr_t)&info, sizeof(info));
                        info.rti_info[RTAX_DST] = dst;
                        if (_rtMissMsgHook)
                            (*_rtMissMsgHook) (msgtype, &info, 0, err);
                }
        }


       /* check to see if we retrieved a non default tos route
        * and if yes then we also get the default tos route
        * and then select the one with the longer netmask
        * as required by the rfc
        */


       if (TOS_GET (dst) != 0)
           {
           TOS_SET (dst, 0);
           tosRt = newrt;
           newrt = NULL;
           goto match;
           }

       if (tosRt != NULL)
           {
           /* select the route with longest netmask. we assume
            * contiguous masks starting at the same bit position
            */

           if ((newrt != NULL) &&
               ((((struct sockaddr_in *)rt_mask(tosRt))->sin_addr.s_addr -
                ((struct sockaddr_in *)rt_mask(newrt))->sin_addr.s_addr) > 0))
               {
               RTFREE (newrt);
               newrt = tosRt;
               }
           else
               {
               RTFREE (tosRt);
               }
           }

       /*
        * restore the saved tos. is redundant but harmless in case tos
        * was default to  start with.
        */

       TOS_SET (dst, savedTos);

       splx(s);

       return (newrt);

    }

void
rtfree(rt)
	register struct rtentry *rt;
{
	register struct ifaddr *ifa;

	if (rt == 0)
		panic("rtfree");
	rt->rt_refcnt--;
	if (rt->rt_refcnt <= 0 && (rt->rt_flags & RTF_UP) == 0) {
		if (rt->rt_nodes->rn_flags & (RNF_ACTIVE | RNF_ROOT))
			panic ("rtfree 2");
		rttrash--;
		if (rt->rt_refcnt < 0) {
			logMsg("rtfree: %x not freed (neg refs)\n", (int)rt,
			       0, 0, 0, 0, 0);
			return;
		}
		ifa = rt->rt_ifa;
		IFAFREE(ifa);
		Free(rt_key(rt));
		if(((ROUTE_ENTRY*)rt)->repNode==FALSE && 
		   (!rt->rt_flags & RTF_LLINFO))
		    routeEntryFree((ROUTE_ENTRY*)rt);
                else
		    Free((ROUTE_ENTRY*)rt);
	}
}

/*
 * Force a routing table entry to the specified
 * destination to go through the given gateway.
 * Normally called as a result of a routing redirect
 * message from the network layer.
 *
 * N.B.: must be called at splnet
 *
 */
int
rtredirect(dst, gateway, netmask, flags, src, rtp)
	struct sockaddr *dst, *gateway, *netmask, *src;
	int flags;
	struct rtentry **rtp;
{
	register struct rtentry *rt;
	int error = 0;
	short *stat = 0;
	struct rt_addrinfo info;
	struct ifaddr *ifa;

	/* verify the gateway is directly reachable */
	if ((ifa = ifa_ifwithnet(gateway)) == 0) {
		error = ENETUNREACH;
		goto out;
	}
	rt = rtalloc1(dst, 0);
	/*
	 * If the redirect isn't from our current router for this dst,
	 * it's either old or wrong.  If it redirects us to ourselves,
	 * we have a routing loop, perhaps as a result of an interface
	 * going down recently.
	 */
#define	equal(a1, a2) (bcmp((caddr_t)(a1), (caddr_t)(a2), (a1)->sa_len) == 0)
	if (!(flags & RTF_DONE) && rt &&
	     (!equal(src, rt->rt_gateway) || rt->rt_ifa != ifa))
		error = EINVAL;
	else if (ifa_ifwithaddr(gateway))
		error = EHOSTUNREACH;
	if (error)
		goto done;
	/*
	 * Create a new entry if we just got back a wildcard entry
	 * or the the lookup failed.  This is necessary for hosts
	 * which use routing redirects generated by smart gateways
	 * to dynamically build the routing tables.
	 */
	if ((rt == 0) || (rt_mask(rt) && rt_mask(rt)->sa_len < 2))
		goto create;
	/*
	 * Don't listen to the redirect if it's
	 * for a route to an interface. 
	 */
	if (rt->rt_flags & RTF_GATEWAY) {
		if (((rt->rt_flags & RTF_HOST) == 0) && (flags & RTF_HOST)) {
			/*
			 * Changing from route to net => route to host.
			 * Create new route, rather than smashing route to net.
			 */
		create:
			flags |=  RTF_GATEWAY | RTF_DYNAMIC;

                         error = rtrequestAddEqui(dst, netmask, gateway,
                                    flags, M2_ipRouteProto_local,
                                    NULL);

			/* error = rtrequest((int)RTM_ADD, dst, gateway,
				    netmask, flags,
				    (struct rtentry **)0);
                        */
			stat = &rtstat.rts_dynamic;
		} else {
			/*
			 * Smash the current notion of the gateway to
			 * this destination.  Should check about netmask!!!
			 */
			rt->rt_flags |= RTF_MODIFIED;
			flags |= RTF_MODIFIED;
			stat = &rtstat.rts_newgateway;
			rt_setgate(rt, rt_key(rt), gateway);
			rt->rt_mod = tickGet(); 	/* last modified */
			rtmodified++; 
		}
	} else
		error = EHOSTUNREACH;
done:
	if (rt) {
		if (rtp && !error)
			*rtp = rt;
		else
			rtfree(rt);
	}
out:
	if (error)
		rtstat.rts_badredirect++;
	else if (stat != NULL)
		(*stat)++;
	bzero((caddr_t)&info, sizeof(info));
	info.rti_info[RTAX_DST] = dst;
	info.rti_info[RTAX_GATEWAY] = gateway;
	info.rti_info[RTAX_NETMASK] = netmask;
	info.rti_info[RTAX_AUTHOR] = src;
        if (_rtMissMsgHook)
            (*_rtMissMsgHook) (RTM_REDIRECT, &info, flags, error);
	return (error); 
}

/*
* Routing table ioctl interface.
*
* WRS MODS support for this function is being for backward compatibility.
* This function has be moved else where because the routing code has 
* nothing to do with internet specific addresses since routing is independent
* of the sockaddress family.
*/
int
rtioctl(req, data)
	int req;
	caddr_t data;
{
	struct ortentry * pORE = NULL;
	struct sockaddr netMask;
	struct sockaddr * pNetMask = &netMask; 
	register u_long i; 
	register u_long net;
	register struct in_ifaddr *ia;

        if (req != SIOCADDRT && req != SIOCDELRT)
                return (EINVAL);

	pORE = (struct ortentry *)data;

	/* BSD4.3 to BSD4.4 conversion */

	if (req == SIOCADDRT)
	    req = RTM_ADD;
	else
	    req = RTM_DELETE;

	/* 
	 * Set the netmask to the netmask of the interface address.
	 * This has to be removed if provision is made with routeAdd
	 * to add the network mask. 
	 */
	bzero ((caddr_t)&netMask, sizeof (struct sockaddr));

	/* check for default gateway address */

	if (((struct sockaddr_in *)(&pORE->rt_dst))->sin_addr.s_addr 
	    != INADDR_ANY )
	    {
	    i = ntohl(((struct sockaddr_in*)&pORE->rt_dst)->sin_addr.s_addr);
		
	    pNetMask->sa_family = AF_INET; 
	    pNetMask->sa_len    = 8;

	    if (IN_CLASSA(i))
		{
		((struct sockaddr_in *)pNetMask)->sin_addr.s_addr = 
		    htonl(IN_CLASSA_NET);
		net = i & IN_CLASSA_NET;
		}
	    else if (IN_CLASSB(i))
		{
		((struct sockaddr_in *)pNetMask)->sin_addr.s_addr = 
		    htonl(IN_CLASSB_NET);
		net = i & IN_CLASSB_NET;
		}
	    else if (IN_CLASSC(i))
		{
		((struct sockaddr_in *)pNetMask)->sin_addr.s_addr = 
		    htonl(IN_CLASSC_NET);
		net = i & IN_CLASSC_NET;
		}
	    else
		{
		((struct sockaddr_in *)pNetMask)->sin_addr.s_addr = 
		    htonl(IN_CLASSD_NET);
		net = i & IN_CLASSD_NET;
		}
	    /*
	     * Check whether network is a subnet;
	     * if so, return subnet number.
	     */
	    for (ia = in_ifaddr; ia; ia = ia->ia_next)
		if (net == ia->ia_net)
		    ((struct sockaddr_in *)pNetMask)->sin_addr.s_addr =
			htonl(ia->ia_subnetmask); 
	    in_socktrim ((struct sockaddr_in *)pNetMask); 
	    }
       pORE->rt_flags |= RTF_GATEWAY;

       if(pORE->rt_flags & RTF_HOST)
	   pNetMask=0;
 
#ifdef DEBUG
       logMsg("rtIoctl:before rtrequestAddEqui/DelEqui\n",0,0,0,0,0,0);
#endif
       if(req==RTM_ADD)
	   return (rtrequestAddEqui(&pORE->rt_dst, pNetMask, &pORE->rt_gateway, 
			            pORE->rt_flags, M2_ipRouteProto_other, NULL));
       else
	   return (rtrequestDelEqui(&pORE->rt_dst, pNetMask, &pORE->rt_gateway,
			            pORE->rt_flags, M2_ipRouteProto_other, NULL));
}

#define ROUNDUP(a) (a>0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))

int
rtrequest(req, dst, gateway, netmask, flags, ret_nrt)
	int req, flags;
	struct sockaddr *dst, *gateway, *netmask;
	struct rtentry **ret_nrt;
{
	int s = splnet(); int error = 0;
	register struct rtentry *rt;
	register struct radix_node *rn;
	register struct radix_node_head *rnh;
	struct ifaddr *ifa;
	struct sockaddr *ndst;
#define senderr(x) { error = x ; goto bad; }

	if ((rnh = rt_tables[dst->sa_family]) == 0)
		senderr(ESRCH);

	if (flags & RTF_HOST)
            {
            netmask = 0;
	    }

        if (netmask)
	    {
	    TOS_SET (netmask, 0x1f);  /* set the 5 bits in the mask corresponding
					 to the TOS bits */
	    }


	switch (req) {
	case RTM_DELETE:
		if ((rn = rnh->rnh_deladdr(dst, netmask, rnh)) == 0)
			senderr(ESRCH);
		if (rn->rn_flags & (RNF_ACTIVE | RNF_ROOT))
			panic ("rtrequest delete");
		rt = (struct rtentry *)rn;

		rt->rt_flags &= ~RTF_UP;

		if (rt->rt_gwroute) {
			rt = rt->rt_gwroute; RTFREE(rt);
			(rt = (struct rtentry *)rn)->rt_gwroute = 0;
		}
		if ((ifa = rt->rt_ifa) && ifa->ifa_rtrequest)
			ifa->ifa_rtrequest(RTM_DELETE, rt, SA(0));
		rttrash++;
		if (ret_nrt)
			*ret_nrt = rt;
		else if (rt->rt_refcnt <= 0) {
			rt->rt_refcnt++;
			rtfree(rt);
		}
		rtmodified++;
		break;

	case RTM_RESOLVE:
		if (ret_nrt == 0 || (rt = *ret_nrt) == 0)
			{
#ifdef DEBUG
			logMsg("rtrequest: RTM_RESOLVE, EINVAL error\n",0,0,0,0,0,0);
#endif
			senderr(EINVAL);
			}
		ifa = rt->rt_ifa;
		flags = rt->rt_flags & ~RTF_CLONING;
		gateway = rt->rt_gateway;
		if ((netmask = rt->rt_genmask) == 0)
			{
			flags |= RTF_HOST;
			}

		goto makeroute;

	case RTM_ADD:
		if ((ifa = ifa_ifwithroute(flags, dst, gateway)) == 0)
			senderr(ENETUNREACH);
	makeroute:
		R_Malloc(rt, struct rtentry *, sizeof(ROUTE_ENTRY));
		if (rt == 0)
			senderr(ENOBUFS);
		Bzero(rt, sizeof(*rt));
		rt->rt_flags = RTF_UP | flags;
		if (rt_setgate(rt, dst, gateway)) {
			Free(rt);
			senderr(ENOBUFS);
		}
		ndst = rt_key(rt);
		if (netmask) {
			rt_maskedcopy(dst, ndst, netmask);
		} else
			Bcopy(dst, ndst, dst->sa_len);
		rn = rnh->rnh_addaddr((caddr_t)ndst, (caddr_t)netmask,
					rnh, rt->rt_nodes);
		/* update proto field of rt entry, in the gateway sockaddr */
		if ((req == RTM_ADD) && (gateway->sa_family == AF_INET))
		    RT_PROTO_SET (ndst, RT_PROTO_GET (dst));

                if (rn == 0 && (rt->rt_flags & RTF_HOST))
                    {
                    /* Replace matching (cloned) ARP entry if possible. */

                    rn = routeSwap (rnh, rt, ndst, netmask);
                    }

		if (rn == 0) {
			if (rt->rt_gwroute)
				rtfree(rt->rt_gwroute);
			Free(rt_key(rt));
			Free(rt);
			senderr(EEXIST);
		}
		ifa->ifa_refcnt++;
		rt->rt_ifa = ifa;
		rt->rt_ifp = ifa->ifa_ifp;
		if (req == RTM_RESOLVE)
			rt->rt_rmx = (*ret_nrt)->rt_rmx; /* copy metrics */
		if (ifa->ifa_rtrequest)
			ifa->ifa_rtrequest(req, rt, SA(ret_nrt ? *ret_nrt : 0));
		if (ret_nrt) {
			*ret_nrt = rt;
			rt->rt_refcnt++;
		}
		rt->rt_mod = tickGet(); 	/* last modified */
		rtmodified++;
		break;
	}
bad:
	splx(s);
	return (error);
}

 /* 
  * This routine is called when adding a host-specific route to
  * the routing tree fails. That failure can occur because an
  * ARP entry for the destination already exists in the tree.
  * If so, the host-specific entry is added after the ARP entry
  * is removed.
  */

struct radix_node * routeSwap
    (
    struct radix_node_head * 	pNodeHead,
    struct rtentry * 		pRoute,
    struct sockaddr * 		pDest,
    struct sockaddr * 		pNetmask
    )
    {
    struct radix_node * 	pNode;
    struct rtentry * 		pTwinRt;
    struct ifaddr * 		pIfAddr;
    struct rtentry * 		pArpRt;
    struct radix_node_head * 	pTwinHead;
    struct sockaddr * 		pTwinKey;

    /*
     * Existing ARP entries will prevent the addition of
     * matching host routes. Delete the ARP entry and try
     * to add the host route again. The search is based
     * on the rtalloc1() routine.
     */

    pNode = rn_search ((caddr_t)pDest, pNodeHead->rnh_treetop);
    if ( (pNode->rn_flags & RNF_ROOT) == 0)
        {
        /* Check leaf node for ARP information. */

        pTwinRt = (struct rtentry *)pNode;
        if (pTwinRt->rt_flags & RTF_LLINFO &&
            pTwinRt->rt_flags & RTF_HOST &&
            pTwinRt->rt_gateway &&
            pTwinRt->rt_gateway->sa_family == AF_LINK)
            {
            /* Mimic RTM_DELETE case from rtrequest() routine. */

            pTwinKey = (struct sockaddr *)rt_key (pTwinRt);
            pTwinHead = rt_tables [pTwinKey->sa_family];
            if (pTwinHead)
                {
                pNode = pTwinHead->rnh_deladdr (pTwinKey, 0, pTwinHead);
                if (pNode)
                    {
                    pArpRt = (struct rtentry *)pNode;
                    pArpRt->rt_flags &= ~RTF_UP;
                    if (pArpRt->rt_gwroute)
                        {
                        pArpRt = pArpRt->rt_gwroute;
                        RTFREE (pArpRt);
                        pArpRt = (struct rtentry *)pNode;
                        pArpRt->rt_gwroute = 0;
                        }
                    pIfAddr = pArpRt->rt_ifa;
                    if (pIfAddr && pIfAddr->ifa_rtrequest)
                        pIfAddr->ifa_rtrequest (RTM_DELETE, pArpRt, SA(0));
                    rttrash++;
                    if (pArpRt->rt_refcnt <= 0)
                        {
                        pArpRt->rt_refcnt++;
                        rtfree (pArpRt);
                        }
                    rtmodified++;
                    }
                }    /* RTM_DELETE completed. */
            pNode = pNodeHead->rnh_addaddr ( (caddr_t)pDest, (caddr_t)pNetmask, 
                                            pNodeHead, pRoute->rt_nodes);
            }  /* End ARP entry replacement. */
        else
            {
            /*
             * Matching node was not an ARP entry. Reset node reference.
             */

            pNode = 0;
            }
        }    /* End handling for RNF_ROOT != 0 */
    else
        {
        /* 
         * No valid entry found: remove reference to leftmost/rightmost node.
         */

        pNode = 0;
        }
    return (pNode);
    }

int
rt_setgate(rt0, dst, gate)
	struct rtentry *rt0;
	struct sockaddr *dst, *gate;
{
	caddr_t new, old;
	int dlen = ROUNDUP(dst->sa_len), glen = ROUNDUP(gate->sa_len);
	register struct rtentry *rt = rt0;

	if (rt->rt_gateway == 0 || glen > ROUNDUP(rt->rt_gateway->sa_len)) {
		old = (caddr_t)rt_key(rt);
		R_Malloc(new, caddr_t, dlen + glen);
		if (new == 0)
			return 1;
		rt->rt_nodes->rn_key = new;
	} else {
		new = rt->rt_nodes->rn_key;
		old = 0;
	}
	Bcopy(gate, (rt->rt_gateway = (struct sockaddr *)(new + dlen)), glen);
	if (old) {
		Bcopy(dst, new, dlen);
		Free(old);
	}
	if (rt->rt_gwroute) {
		rt = rt->rt_gwroute; RTFREE(rt);
		rt = rt0; rt->rt_gwroute = 0;
	}
	if (rt->rt_flags & RTF_GATEWAY) {
		rt->rt_gwroute = rtalloc1(gate, 1);
	}
	return 0;
}

void
rt_maskedcopy(src, dst, netmask)
	struct sockaddr *src, *dst, *netmask;
{
	register u_char *cp1 = (u_char *)src;
	register u_char *cp2 = (u_char *)dst;
	register u_char *cp3 = (u_char *)netmask;
	u_char *cplim = cp2 + *cp3;
	u_char *cplim2 = cp2 + *cp1;

	*cp2++ = *cp1++; *cp2++ = *cp1++; /* copies sa_len & sa_family */
	cp3 += 2;
	if (cplim > cplim2)
		cplim = cplim2;
	while (cp2 < cplim)
		*cp2++ = *cp1++ & *cp3++;
	if (cp2 < cplim2)
		bzero((caddr_t)cp2, (unsigned)(cplim2 - cp2));
}

/*
 * Set up a routing table entry, normally
 * for an interface.
 */
int
rtinit(ifa, cmd, flags)
	register struct ifaddr *ifa;
	int cmd, flags;
{
	register struct rtentry *rt;
	register struct sockaddr *dst;
	register struct sockaddr *deldst;
	struct mbuf *m = 0;
	struct rtentry *nrt = 0;
	int error=0;

	dst = flags & RTF_HOST ? ifa->ifa_dstaddr : ifa->ifa_addr;
	if (cmd == RTM_DELETE) {
		if ((flags & RTF_HOST) == 0 && ifa->ifa_netmask) {
			m = mBufClGet(M_WAIT, MT_SONAME, CL_SIZE_128, TRUE);
			if (m == (struct mbuf *) NULL)
			    {
			    return (ENOBUFS);
			    }
			deldst = mtod(m, struct sockaddr *);
			rt_maskedcopy(dst, deldst, ifa->ifa_netmask);
			dst = deldst;
		}
		if ((rt = rtalloc1(dst, 0))) {
			rt->rt_refcnt--;
			if (rt->rt_ifa != ifa) {
				if (m)
					(void) m_free(m);
				return (flags & RTF_HOST ? EHOSTUNREACH
							: ENETUNREACH);
			}
		}
	}

       if(cmd==RTM_ADD)
	   error=rtrequestAddEqui(dst, ifa->ifa_netmask, ifa->ifa_addr, 
	   		          flags | ifa->ifa_flags, M2_ipRouteProto_local, 
				  (ROUTE_ENTRY**)&nrt);
       if(cmd==RTM_DELETE)
	   error=rtrequestDelEqui(dst, ifa->ifa_netmask, ifa->ifa_addr,
			          flags | ifa->ifa_flags, M2_ipRouteProto_local, 
				  (ROUTE_ENTRY**)&nrt);

	if (m)
		(void) m_free(m);
	if (cmd == RTM_DELETE && error == 0 && (rt = nrt)) {
        	if (_rtNewAddrMsgHook) 
                    (*_rtNewAddrMsgHook) (cmd, ifa, error, nrt);
		if (rt->rt_refcnt <= 0) {
			rt->rt_refcnt++;
			rtfree(rt);
		}
	}
	if (cmd == RTM_ADD && error == 0 && (rt = nrt)) {
		rt->rt_refcnt--;
		if (rt->rt_ifa != ifa) {
			logMsg("rtinit: wrong ifa (%x) was (%x)\n", (int) ifa,
				(int) rt->rt_ifa, 0, 0, 0, 0);
			if (rt->rt_ifa->ifa_rtrequest)
			    rt->rt_ifa->ifa_rtrequest(RTM_DELETE, rt, SA(0));
			IFAFREE(rt->rt_ifa);
			rt->rt_ifa = ifa;
			rt->rt_ifp = ifa->ifa_ifp;
			ifa->ifa_refcnt++;
			if (ifa->ifa_rtrequest)
			    ifa->ifa_rtrequest(RTM_ADD, rt, SA(0));
		}
		if (_rtNewAddrMsgHook)
                    (*_rtNewAddrMsgHook) (cmd, ifa, error, nrt);
	}
	return (error);
}
