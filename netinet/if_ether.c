/* if_ether.c - network Ethernet address resolution protocol */

/* Copyright 1984 - 2000 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/*
 * Copyright (c) 1982, 1986, 1988, 1993
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
 *	@(#)if_ether.c	8.2 (Berkeley) 9/26/94
 */

/*
 * Ethernet address resolution protocol.
 * TODO:
 *	add "inuse/lock" bit (or ref. count) along with valid bit
 */

/*
modification history
--------------------
01p,04apr00,pul  remove  FASTPATH references
01o,22feb00,pul  made db_print_sa() extern
01n,22feb00,spm  disabled debugging statements; restored prototypes to private
01m,11feb00,gnn  first checkin to openstack (Siemens ONLY)
01l,04jan99,pul  Fast Path Ip Modifications: call to  fpDelArpEntry()
01k,08nov99,pul  merging T2 cumulative patch 2
01j,08Nov98,pul  changed the argument ordering to ipEtherResolveRtn
01i,29apr99,pul  Upgraded NPT phase3 code to tor2.0.0
01h,10may99,spm  fixed SIOCSARP handling to support ATF_PUBL flag (SPR #24397)
01g,03sep98,n_s  fixed handling of IFF_NOARP flag in arpresolve. spr #22280 01g,13jul98,n_s  added arpRxmitTicks.  spr # 21577
01f,10jul97,vin  ifdefed revarp code, fixed warnings.
01e,23apr97,vin  fixed SPR8445, rt_expire for arp route entry being set to 0.
01d,29jan97,vin  fixed a bug in arpioctl(), sdl_alen field.
01c,16dec96,vin  removed unnecessary htons in arprequest() and arpinput().
01b,31oct96,vin	 changed m_gethdr(..) to mHdrClGet(..).  
01a,03mar96,vin  created from BSD4.4 stuff,integrated with 02n of if_ether.c
*/

/*
 * Ethernet address resolution protocol.
 * TODO:
 *      run at splnet (add ARP protocol intr.)
 *      link entries onto hash chains, keep free list
 *      add "inuse/lock" bit (or ref. count) along with valid bit
 */
#include "vxWorks.h"
#include "logLib.h"

#include "net/mbuf.h"
#include "sys/socket.h"
#include "errno.h"
#include "sys/ioctl.h"

#include "net/if.h"
#include "net/if_dl.h"
#include "net/route.h"
#include "netinet/in.h"
#include "netinet/in_systm.h"
#include "netinet/in_var.h"
#include "netinet/ip.h"
#include "netinet/if_ether.h"
#include "wdLib.h"
#include "tickLib.h"
#include "net/systm.h"

#ifdef INET

/* defines */

#define PROXY_HOOK		/* include proxy hook code */

#define SIN(s) ((struct sockaddr_in *)s)
#define SDL(s) ((struct sockaddr_dl *)s)
#define SRP(s) ((struct sockaddr_inarp *)s)

/*
 * ARP trailer negotiation.  Trailer protocol is not IP specific,
 * but ARP request/response use IP addresses.
 */
#define ETHERTYPE_IPTRAILERS ETHERTYPE_TRAIL

#define	rt_expire rt_rmx.rmx_expire

#define ARP_RXMIT_TICKS_DFLT sysClkRateGet ()      /* Minimum number of ticks */
					           /* between retransmission  */
						   /* of ARP request          */

/* externs */

IMPORT void netJobAdd ();
IMPORT sysClkRateGet ();

extern	struct ifnet loif;
extern  _insque ();
extern  _remque ();

/* globals */

void arptfree (struct llinfo_arp *);

struct	llinfo_arp llinfo_arp = {&llinfo_arp, &llinfo_arp};
struct	ifqueue arpintrq = {0, 0, 0, 50};
int	arp_inuse, arp_allocated, arp_intimer;
int	arp_maxtries = 5;
int	useloopback = 1;	/* use loopback interface for local traffic */
int	arpinit_done = 0;

/* timer values */
int	arpt_prune = (1*60);	/* walk list every 1 minutes */
int	arpt_keep = (20*60);	/* once resolved, good for 20 more minutes */
int	arpt_down = 20;		/* once declared down, don't send for 20 secs */
int     arpRxmitTicks = -1;     /* Minimum number of ticks between */
                                /*   retranmission of ARP request */

/* proxy ary hook */

#ifdef PROXY_HOOK
FUNCPTR	 proxyArpHook = NULL;
#endif

/* locals */

/* revarp state */
#ifdef INCLUDE_REVARP /* XXX currently not supported */
static struct	in_addr myip, srv_ip;
static int	myip_initialized = 0;
static int	revarp_in_progress = 0;
static struct	ifnet *myip_ifp = NULL;
#endif /* INCLUDE_REVARP */

LOCAL WDOG_ID arptimerWd;	/* watchdog timer for arptimer routine */
static char 	digits[] = "0123456789abcdef";

/* forward declarations */

static	void arprequest (struct arpcom *, u_long *, u_long *, u_char *);
static	void arptimer (void *);
static	struct llinfo_arp *arplookup (u_long, int, int);
static	void in_arpinput (struct mbuf *);

/*
 * Timeout routine.  Age arp_tab entries periodically.
 */
/* ARGSUSED */
static void
arptimer(arg)
	void *arg;
    {
    register struct llinfo_arp *la = llinfo_arp.la_next;

    wdStart (arptimerWd, arpt_prune * sysClkRateGet(), (FUNCPTR) netJobAdd,
	     (int)arptimer);

    while (la != &llinfo_arp) 
	{
	register struct rtentry *rt = la->la_rt;
	la = la->la_next;
	if (rt->rt_expire && rt->rt_expire <= tickGet())
	    {
	    arptfree(la->la_prev); /* timer has expired; clear */
	    }
	}
    }


/* IP multicast address to Ethernet mulitcast address mapping */

void ipToEtherMCastMap
     ( 
     struct in_addr* ipaddr,
     u_char* enaddr
     )
     {
     enaddr[0] = 0x01;
     enaddr[1] = 0x00;
     enaddr[2] = 0x5e;
     enaddr[3] = ((u_char *)ipaddr)[1] & 0x7f;
     enaddr[4] = ((u_char *)ipaddr)[2];
     enaddr[5] = ((u_char *)ipaddr)[3];
     return;
     }

/* IP to Ethernet address resolution protocol. This is just a wrapper for arpresolve
* being called from ipOutput. This routine calls arpresolve with the appropriate
* set of arguments. 
*
* Returns 1 if the address is resolved and 0 if the address is not resolved.
*/

int ipEtherResolvRtn
    (
    FUNCPTR ipArpCallBackRtn,
    struct mbuf* pMbuf,
    struct sockaddr* dstIpAddr,
    struct ifnet* ifp,
    struct rtentry* rt,
    char* dstBuff
    )
    {
    return(arpresolve(ifp,rt,pMbuf,dstIpAddr,dstBuff));
    }


/*
 * Parallel to llc_rtrequest.
 */
void
arp_rtrequest(req, rt, sa)
	int req;
	register struct rtentry *rt;
	struct sockaddr *sa;
{
	register struct sockaddr *gate = rt->rt_gateway;
	register struct llinfo_arp *la = (struct llinfo_arp *)rt->rt_llinfo;
	static struct sockaddr_dl null_sdl = {sizeof(null_sdl), AF_LINK};
	

	if (!arpinit_done) {
	/*
	 * if this routine is executed for the first time, then 
	 * create the watch dog timer and kick of the arp timer.
	 */
		arpinit_done = 1;
		arptimerWd = wdCreate ();
		netJobAdd (arptimer);
	}
	if (rt->rt_flags & RTF_GATEWAY)
		return;
	switch (req) {

	case RTM_ADD:
		/*
		 * XXX: If this is a manually added route to interface
		 * such as older version of routed or gated might provide,
		 * restore cloning bit.
		 */
		if ((rt->rt_flags & RTF_HOST) == 0 &&
		    SIN(rt_mask(rt))->sin_addr.s_addr != 0xffffffff)
			rt->rt_flags |= RTF_CLONING;
		if (rt->rt_flags & RTF_CLONING) {
			/*
			 * Case 1: This route should come from a route to iface.
			 */
			rt_setgate(rt, rt_key(rt),
					(struct sockaddr *)&null_sdl);
			gate = rt->rt_gateway;
			SDL(gate)->sdl_type = rt->rt_ifp->if_type;
			SDL(gate)->sdl_index = rt->rt_ifp->if_index;
			/*
			 * Give this route an expiration time, even though
			 * it's a "permanent" route, so that routes cloned
			 * from it do not need their expiration time set.
			 */
			rt->rt_expire = tickGet();
                        /*
                         * rt_expire could be zero if the interface is
                         * initialized before the first tick of the system
                         * typically 1/60th of a second.
                         * All the arp routes cloned from this route will
                         * inherit all the route metrics properties.
                         * if rt_expire is 0 for an arp route entry,
                         * it never kicks in arpwhohas () in arpresolve ().
                         * This would result in not generating an arp request
                         * for the host. This problem was seen on the MIPS
                         * board p4000. --- Vinai. Fix for SPR 8445
                         */
                        if (rt->rt_expire == 0)
                            rt->rt_expire++;

			break;
		}
		/* Announce a new entry if requested. */
		if (rt->rt_flags & RTF_ANNOUNCE)
			arprequest((struct arpcom *)rt->rt_ifp,
			    &SIN(rt_key(rt))->sin_addr.s_addr,
			    &SIN(rt_key(rt))->sin_addr.s_addr,
			    (u_char *)LLADDR(SDL(gate)));
		/*FALLTHROUGH*/
	case RTM_RESOLVE:
		if (gate->sa_family != AF_LINK ||
		    gate->sa_len < sizeof(null_sdl)) {
			logMsg("arp_rtrequest: bad gateway value",0,0,0,0,0,0);
			break;
		}
		SDL(gate)->sdl_type = rt->rt_ifp->if_type;
		SDL(gate)->sdl_index = rt->rt_ifp->if_index;
		if (la != 0)
			break; /* This happens on a route change */
		/*
		 * Case 2:  This route may come from cloning, or a manual route
		 * add with a LL address.
		 */
		R_Malloc(la, struct llinfo_arp *, sizeof(*la));
		rt->rt_llinfo = (caddr_t)la;
		if (la == 0) {
			logMsg("arp_rtrequest: malloc failed\n",0,0,0,0,0,0);
			break;
		}
		arp_inuse++, arp_allocated++;
		Bzero(la, sizeof(*la));
		la->la_rt = rt;
		rt->rt_flags |= RTF_LLINFO;
		insque(la, &llinfo_arp);
		if (SIN(rt_key(rt))->sin_addr.s_addr ==
		    (IA_SIN(rt->rt_ifa))->sin_addr.s_addr) {
		    /*
		     * This test used to be
		     *	if (loif.if_flags & IFF_UP)
		     * It allowed local traffic to be forced
		     * through the hardware by configuring the loopback down.
		     * However, it causes problems during network configuration
		     * for boards that can't receive packets they send.
		     * It is now necessary to clear "useloopback" and remove
		     * the route to force traffic out to the hardware.
		     */
			rt->rt_expire = 0;
			Bcopy(((struct arpcom *)rt->rt_ifp)->ac_enaddr,
				LLADDR(SDL(gate)), SDL(gate)->sdl_alen = 6);
			if (useloopback)
				rt->rt_ifp = &loif;

		}
		break;

	case RTM_DELETE:
		if (la == 0)
			break;
		arp_inuse--;
		remque(la);
		rt->rt_llinfo = 0;
		rt->rt_flags &= ~RTF_LLINFO;
		if (la->la_hold)
			m_freem(la->la_hold);
		Free((caddr_t)la);
	}
}

/*
 * Broadcast an ARP packet, asking who has addr on interface ac.
 */
void
arpwhohas(ac, addr)
	register struct arpcom *ac;
	register struct in_addr *addr;
{
	arprequest(ac, &ac->ac_ipaddr.s_addr, &addr->s_addr, ac->ac_enaddr);
}

/*
 * Broadcast an ARP request. Caller specifies:
 *	- arp header source ip address
 *	- arp header target ip address
 *	- arp header source ethernet address
 */
static void
arprequest(ac, sip, tip, enaddr)
	register struct arpcom *ac;
	register u_long *sip, *tip;
	register u_char *enaddr;
{
	register struct mbuf *m;
	register struct ether_header *eh;
	register struct ether_arp *ea;
	struct sockaddr sa;

	if ((m = mHdrClGet(M_DONTWAIT, MT_DATA, 
			   sizeof(*ea), TRUE)) == NULL)
	    return;
	m->m_len = sizeof(*ea);
	m->m_pkthdr.len = sizeof(*ea);
	MH_ALIGN(m, sizeof(*ea));
	ea = mtod(m, struct ether_arp *);
	eh = (struct ether_header *)sa.sa_data;
	bzero((caddr_t)ea, sizeof (*ea));
	bcopy((caddr_t)etherbroadcastaddr, (caddr_t)eh->ether_dhost,
	    sizeof(eh->ether_dhost));
	eh->ether_type = ETHERTYPE_ARP;		/* if_output will swap */
	ea->arp_hrd = htons(ARPHRD_ETHER);
	ea->arp_pro = htons(ETHERTYPE_IP);
	ea->arp_hln = sizeof(ea->arp_sha);	/* hardware address length */
	ea->arp_pln = sizeof(ea->arp_spa);	/* protocol address length */
	ea->arp_op = htons(ARPOP_REQUEST);
	bcopy((caddr_t)enaddr, (caddr_t)ea->arp_sha, sizeof(ea->arp_sha));
	bcopy((caddr_t)sip, (caddr_t)ea->arp_spa, sizeof(ea->arp_spa));
	bcopy((caddr_t)tip, (caddr_t)ea->arp_tpa, sizeof(ea->arp_tpa));
	sa.sa_family = AF_UNSPEC;
	sa.sa_len = sizeof(sa);
	(*ac->ac_if.if_output)(&ac->ac_if, m, &sa, (struct rtentry *)0);
}

/*
 * Resolve an IP address into an ethernet address.  If success,
 * desten is filled in.  If there is no entry in arptab,
 * set one up and broadcast a request for the IP address.
 * Hold onto this mbuf and resend it once the address
 * is finally resolved.  A return value of 1 indicates
 * that desten has been filled in and the packet should be sent
 * normally; a 0 return indicates that the packet has been
 * taken over here, either now or for later transmission.
 */
int
arpresolve(ac, rt, m, dst, desten)
	register struct arpcom *ac;
	register struct rtentry *rt;
	struct mbuf *m;
	register struct sockaddr *dst;
	register u_char *desten;
{
	register struct llinfo_arp *la;
	struct sockaddr_dl *sdl;
	int alen;                          /* Length of MAC address */
	u_long lna;                        /* Host portion if IP address */

	if (m != NULL)		/* check if mbuf is not null */
	    {
	    if (m->m_flags & M_BCAST)
		{	/* broadcast */
		bcopy((caddr_t)etherbroadcastaddr, (caddr_t)desten,
		      sizeof(etherbroadcastaddr));
		return (1);
		}
	    if (m->m_flags & M_MCAST)
		{	/* multicast */
		ETHER_MAP_IP_MULTICAST(&SIN(dst)->sin_addr, desten);
		return(1);
		}
	    }
	
	if (rt)
		la = (struct llinfo_arp *)rt->rt_llinfo;
	else {
		if ((la = arplookup(SIN(dst)->sin_addr.s_addr, 1, 0)))
			rt = la->la_rt;
	}
	if (la == 0 || rt == 0) {
		logMsg ("arpresolve: can't allocate llinfo",0,0,0,0,0,0);
		m_freem(m);
		return (0);
	}
	sdl = SDL(rt->rt_gateway);
	/*
	 * Check the address family and length is valid, the address
	 * is resolved; otherwise, try to resolve.
	 */
	if ((rt->rt_expire == 0 || rt->rt_expire > tickGet()) &&
	    sdl->sdl_family == AF_LINK && sdl->sdl_alen != 0) {
		bcopy(LLADDR(sdl), (char *)desten, sdl->sdl_alen);
		return 1;
	}

	/* If IFF_NOARP then make LL addr from IP host addr */
	 
	if (ac->ac_if.if_flags & IFF_NOARP) 
	    {

	    /* Set expiration of Route/ARP entry */

	    if (rt->rt_expire == 0)
		{
		rt->rt_expire = tickGet ();
		}

	    /* Set length of MAC address.  Default to Ethernet length. */

	    alen = (sdl->sdl_alen > 0 ? sdl->sdl_alen : 
		    sizeof (struct ether_addr));

	    if (alen < 3)
		{
		return (0);
		}

	    bcopy((caddr_t)ac->ac_enaddr, (char *)desten, alen - 3);

	    lna = in_lnaof(SIN(dst)->sin_addr);
	    desten[alen - 1]     = lna & 0xff;
	    desten[alen - 1 - 1] = (lna >> 8) & 0xff;
	    desten[alen - 1 - 2] = (lna >> 16) & 0x7f;
	    return (1);
	    }

	/*
	 * There is an arptab entry, but no ethernet address
	 * response yet.  Replace the held mbuf with this
	 * latest one.
	 */
	if (la->la_hold)
		m_freem(la->la_hold);
	la->la_hold = m;
	if (rt->rt_expire) {
		rt->rt_flags &= ~RTF_REJECT;
		if (arpRxmitTicks < 0)
		    arpRxmitTicks = ARP_RXMIT_TICKS_DFLT;
		if (la->la_asked == 0 || 
		    (tickGet () - rt->rt_expire >= arpRxmitTicks)) {
			rt->rt_expire = tickGet();
			if (la->la_asked++ < arp_maxtries)
				arpwhohas(ac, &(SIN(dst)->sin_addr));
			else {
				rt->rt_flags |= RTF_REJECT;
				rt->rt_expire += (sysClkRateGet() * arpt_down);
				la->la_asked = 0;
			}
		}
	}
	return (0);
}

/*
 * Common length and type checks are done here,
 * then the protocol-specific routine is called.
 */
void
arpintr()
{
	register struct mbuf *m;
	register struct arphdr *ar;
	int s;

	while (arpintrq.ifq_head) {
		s = splimp();
		IF_DEQUEUE(&arpintrq, m);
		splx(s);
		if (m == 0 || (m->m_flags & M_PKTHDR) == 0)
        {
            if ( m == 0 )
                panic("arpintr: m is NULL\n");
            if ( (m->m_flags & M_PKTHDR) == 0 )
                panic("arpintr: M_PKTHDR is NOT set\n");

			panic("arpintr");
        }
		if (m->m_len >= sizeof(struct arphdr) &&
		    (ar = mtod(m, struct arphdr *)) &&
		    ntohs(ar->ar_hrd) == ARPHRD_ETHER &&
		    m->m_len >=
		      sizeof(struct arphdr) + 2 * (ar->ar_hln + ar->ar_pln))
			switch (ntohs(ar->ar_pro)) {

			case ETHERTYPE_IP:
			case ETHERTYPE_IPTRAILERS:
#ifdef PROXY_HOOK
			        if (proxyArpHook != NULL)
				    (* proxyArpHook)
					((struct arpcom *)m->m_pkthdr.rcvif,m);
#endif
				in_arpinput(m);
				continue;
			}
		m_freem(m);
	}
}

/*
 * ARP for Internet protocols on 10 Mb/s Ethernet.
 * Algorithm is that given in RFC 826.
 * In addition, a sanity check is performed on the sender
 * protocol address, to catch impersonators.
 * We no longer handle negotiations for use of trailer protocol:
 * Formerly, ARP replied for protocol type ETHERTYPE_TRAIL sent
 * along with IP replies if we wanted trailers sent to us,
 * and also sent them in response to IP replies.
 * This allowed either end to announce the desire to receive
 * trailer packets.
 * We no longer reply to requests for ETHERTYPE_TRAIL protocol either,
 * but formerly didn't normally send requests.
 */
static void
in_arpinput(m)
	struct mbuf *m;
{
	register struct ether_arp *ea;
	register struct arpcom *ac = (struct arpcom *)m->m_pkthdr.rcvif;
	struct ether_header *eh;
	register struct llinfo_arp *la = 0;
	register struct rtentry *rt;
	struct in_ifaddr *ia, *maybe_ia = 0;
	struct sockaddr_dl *sdl;
	struct sockaddr sa;
	struct in_addr isaddr, itaddr, myaddr;
	int op;

	ea = mtod(m, struct ether_arp *);
	op = ntohs(ea->arp_op);
	bcopy((caddr_t)ea->arp_spa, (caddr_t)&isaddr, sizeof (isaddr));
	bcopy((caddr_t)ea->arp_tpa, (caddr_t)&itaddr, sizeof (itaddr));
	for (ia = in_ifaddr; ia; ia = ia->ia_next)
		if (ia->ia_ifp == &ac->ac_if) {
			maybe_ia = ia;
			if ((itaddr.s_addr == ia->ia_addr.sin_addr.s_addr) ||
			     (isaddr.s_addr == ia->ia_addr.sin_addr.s_addr))
				break;
		}
	if (maybe_ia == 0)
		goto out;
	myaddr = ia ? ia->ia_addr.sin_addr : maybe_ia->ia_addr.sin_addr;
	if (!bcmp((caddr_t)ea->arp_sha, (caddr_t)ac->ac_enaddr,
	    sizeof (ea->arp_sha)))
		goto out;	/* it's from me, ignore it. */
	if (!bcmp((caddr_t)ea->arp_sha, (caddr_t)etherbroadcastaddr,
	    sizeof (ea->arp_sha))) {
		logMsg (
		    "arp: ether address is broadcast for IP address %x!\n",
		    (int)ntohl(isaddr.s_addr), 0,0,0,0,0);
		goto out;
	}
	if (isaddr.s_addr == myaddr.s_addr) {
		logMsg(
		   "duplicate IP address %08x sent from ethernet address %s\n",
		   (int)ntohl(isaddr.s_addr), (int)ether_sprintf(ea->arp_sha),
		   0,0,0,0);
		itaddr = myaddr;
		goto reply;
	}
	la = arplookup(isaddr.s_addr, itaddr.s_addr == myaddr.s_addr, 0);
	if (la && (rt = la->la_rt) && (sdl = SDL(rt->rt_gateway))) {
		if (sdl->sdl_alen &&
		    bcmp((caddr_t)ea->arp_sha, LLADDR(sdl), sdl->sdl_alen))
			logMsg("arp info overwritten for %08x by %s\n",
			    (int) ntohl(isaddr.s_addr), 
			    (int) ether_sprintf(ea->arp_sha),0,0,0,0);
		bcopy((caddr_t)ea->arp_sha, LLADDR(sdl),
		    sdl->sdl_alen = sizeof(ea->arp_sha));
		if (rt->rt_expire)
			rt->rt_expire = tickGet() + (sysClkRateGet() * 
						     arpt_keep);
		rt->rt_flags &= ~RTF_REJECT;
		la->la_asked = 0;
		if (la->la_hold) {
			(*ac->ac_if.if_output)(&ac->ac_if, la->la_hold,
				rt_key(rt), rt);
			la->la_hold = 0;
		}
	}
reply:
	if (op != ARPOP_REQUEST) {
	out:
		m_freem(m);
		return;
	}
	if (itaddr.s_addr == myaddr.s_addr) {
		/* I am the target */
		bcopy((caddr_t)ea->arp_sha, (caddr_t)ea->arp_tha,
		    sizeof(ea->arp_sha));
		bcopy((caddr_t)ac->ac_enaddr, (caddr_t)ea->arp_sha,
		    sizeof(ea->arp_sha));
	} else {
		la = arplookup(itaddr.s_addr, 0, SIN_PROXY);
		if (la == 0)
			goto out;
		rt = la->la_rt;
		bcopy((caddr_t)ea->arp_sha, (caddr_t)ea->arp_tha,
		    sizeof(ea->arp_sha));
		sdl = SDL(rt->rt_gateway);
		bcopy(LLADDR(sdl), (caddr_t)ea->arp_sha, sizeof(ea->arp_sha));
	}

	bcopy((caddr_t)ea->arp_spa, (caddr_t)ea->arp_tpa, sizeof(ea->arp_spa));
	bcopy((caddr_t)&itaddr, (caddr_t)ea->arp_spa, sizeof(ea->arp_spa));
	ea->arp_op = htons(ARPOP_REPLY);
	ea->arp_pro = htons(ETHERTYPE_IP); /* let's be sure! */
	eh = (struct ether_header *)sa.sa_data;
	bcopy((caddr_t)ea->arp_tha, (caddr_t)eh->ether_dhost,
	    sizeof(eh->ether_dhost));
	eh->ether_type = ETHERTYPE_ARP;
	sa.sa_family = AF_UNSPEC;
	sa.sa_len = sizeof(sa);
	(*ac->ac_if.if_output)(&ac->ac_if, m, &sa, (struct rtentry *)0);
	return;
}

/*
 * Free an arp entry.
 */
void
arptfree(la)
	register struct llinfo_arp *la;
{
	register struct rtentry *rt = la->la_rt;
	register struct sockaddr_dl *sdl;

	if (rt == 0)
		panic("arptfree");
	if (rt->rt_refcnt > 0 && (sdl = SDL(rt->rt_gateway)) &&
	    sdl->sdl_family == AF_LINK) {
		sdl->sdl_alen = 0;
		la->la_asked = 0;
		rt->rt_flags &= ~RTF_REJECT;
		return;
	}

	rtrequest(RTM_DELETE, rt_key(rt), (struct sockaddr *)0, rt_mask(rt),
	    0, (struct rtentry **)0);
}

/*
 * Lookup or enter a new address in arptab.
 */
static struct llinfo_arp *
arplookup(addr, create, proxy)
	u_long addr;
	int create, proxy;
{
	register struct rtentry *rt;
	static struct sockaddr_inarp sin = {sizeof(sin), AF_INET };

	sin.sin_addr.s_addr = addr;
	sin.sin_other = proxy ? SIN_PROXY : 0;
	rt = rtalloc1((struct sockaddr *)&sin, create);
	if (rt == 0)
		return (0);
	rt->rt_refcnt--;
	if ((rt->rt_flags & RTF_GATEWAY) || (rt->rt_flags & RTF_LLINFO) == 0 ||
	    rt->rt_gateway->sa_family != AF_LINK) {
		if (create)
			logMsg("arptnew failed on %x\n", (int)ntohl(addr),
			0,0,0,0,0);
		return (0);
	}
	return ((struct llinfo_arp *)rt->rt_llinfo);
}

int arpioctl
    (
    int 	cmd,
    caddr_t	data
    )
    {
    register struct arpreq *		ar = (struct arpreq *)data;
    register struct sockaddr_in *	soInAddr;

    register struct llinfo_arp *	la = NULL;
    register struct rtentry *		rt = NULL;
    struct rtentry * 			pNewRt;
    struct sockaddr_dl * 		sdl;
    struct sockaddr_in 			rtmask;
    struct sockaddr_in * 		pMask;

    struct sockaddr_inarp 		ipaddr;
    struct sockaddr_dl 			arpaddr;
    int error = OK;
    int flags = 0;

    BOOL proxy = FALSE;
    BOOL export = FALSE;

    if (ar->arp_pa.sa_family != AF_INET ||
	ar->arp_ha.sa_family != AF_UNSPEC)
	return (EAFNOSUPPORT);

    soInAddr = (struct sockaddr_in *)&ar->arp_pa;

    switch (cmd) 
	{
	case SIOCSARP:
            if (ar->arp_flags & ATF_PUBL)
                {
                flags |= RTF_ANNOUNCE;
                proxy = TRUE;
                }

            /* Search for existing entry with same address. */
   
            bzero ( (char *)&ipaddr, sizeof (ipaddr));
            ipaddr.sin_len = sizeof (ipaddr);
            ipaddr.sin_family = AF_INET;
            ipaddr.sin_addr.s_addr = soInAddr->sin_addr.s_addr;

            rt = rtalloc1 ((struct sockaddr *)&ipaddr, 0);
            if (rt)
                {
                rt->rt_refcnt--;
                if (soInAddr->sin_addr.s_addr ==
                        ((struct sockaddr_in *)rt_key (rt))->sin_addr.s_addr)
                    {
                    /* Matching entry found. Check address type. */

                    if ( (rt->rt_flags & RTF_GATEWAY) || 
                              (rt->rt_flags & RTF_LLINFO) == 0 || 
                                     rt->rt_gateway->sa_family != AF_LINK)
                        {
                        /* Host entry found: must create proxy entry. */

                        if ( !(ar->arp_flags & ATF_PUBL))
                            return (EINVAL);

                        /* 
                         * Set flag to prevent replacement of current entry 
                         * by creating entry which includes SIN_PROXY flag.
                         */

                        export = TRUE;
                        }
                    }
                }

            /*
             * Search completed. Get the interface type and index value
             * from the parent route.
             */

            if (rt)
                {
                /*
                 * For remote hosts or networks, the gateway's associated
                 * route entry contains the interface values.
                 */

                if (rt->rt_flags & RTF_GATEWAY)
                    {
                    if (rt->rt_gwroute == 0)
                        return (EHOSTUNREACH);
                    else
                        sdl = SDL (rt->rt_gwroute->rt_gateway);
                    }
                else
                    sdl = SDL (rt->rt_gateway);

                if (sdl->sdl_family != AF_LINK)
                    return (EINVAL);
                }
            else
                return (ENETUNREACH);

            bzero ( (char *)&rtmask, sizeof (rtmask));
            rtmask.sin_len = 8;
            rtmask.sin_addr.s_addr = 0xffffffff;

            bzero ( (char *)&ipaddr, sizeof (ipaddr));
            ipaddr.sin_len = sizeof (ipaddr);
            ipaddr.sin_family = AF_INET;
            ipaddr.sin_addr.s_addr = soInAddr->sin_addr.s_addr;

            bzero ( (char *)&arpaddr, sizeof (arpaddr));
	    arpaddr.sdl_len = sizeof (arpaddr);
            arpaddr.sdl_family = AF_LINK;

            arpaddr.sdl_type = sdl->sdl_type;
            arpaddr.sdl_index = sdl->sdl_index;
 
	    bcopy((caddr_t)ar->arp_ha.sa_data, LLADDR(&arpaddr),
		  arpaddr.sdl_alen = sizeof(struct ether_addr)); 

            flags |= (RTF_HOST | RTF_STATIC);

            /* 
             * A netmask of 0 compares each route entry against the entire
             * key during lookups. If an overlapping network route already
             * exists, set it to all ones so that queries with SIN_PROXY set
             * will still succeed. If a matching host route exists, set the
             * SIN_PROXY flag in the new entry instead.
             */

            pMask = NULL;
            if (proxy)
                {
                if (export)
                    ipaddr.sin_other = SIN_PROXY;
                else
                    {
                    pMask = &rtmask;
                    flags &= ~RTF_HOST;
                    }
                }

            error = rtrequest (RTM_ADD, (struct sockaddr *)&ipaddr,
                               (struct sockaddr *)&arpaddr,
                               (struct sockaddr *)pMask, flags, &pNewRt);
            if (error == 0 && pNewRt)
                {
	        if (ar->arp_flags & ATF_PERM)
		    pNewRt->rt_expire = 0; 
	        else 
		    pNewRt->rt_expire = tickGet() + (sysClkRateGet() *
                                                     arpt_keep);
                pNewRt->rt_refcnt--;
                }
	    break;

	case SIOCDARP:		/* delete entry */
	    if ((la = arplookup(soInAddr->sin_addr.s_addr, 0, 0)) == NULL)
		return (EADDRNOTAVAIL); 
	    arptfree(la);
	    break;

	case SIOCGARP:		/* get entry */
	    if ((la = arplookup(soInAddr->sin_addr.s_addr, 0, 0)) == NULL)
		return (EADDRNOTAVAIL); 

	    rt = la->la_rt; 
            sdl = SDL(rt->rt_gateway);
	    ar->arp_flags = 0; 	/* initialize the flags */

	    if (sdl->sdl_alen)
		{
		bcopy(LLADDR(sdl), (caddr_t)ar->arp_ha.sa_data, sdl->sdl_alen);
		ar->arp_flags |= ATF_COM; 
		}
	    else
		return (EADDRNOTAVAIL); 
		
	    if (rt->rt_flags & RTF_UP)
		ar->arp_flags |= ATF_INUSE; 

	    if (rt->rt_flags & RTF_ANNOUNCE)
		ar->arp_flags |= ATF_PUBL; 
	    
	    if (rt->rt_expire == 0) 
		ar->arp_flags |= ATF_PERM;

	    break;

	default : 
	    return (EINVAL); 
	}
    return (error); 
    }

/*
 * Convert Ethernet address to printable (loggable) representation.
 */
char *
ether_sprintf(ap)
	register u_char *ap;
{
	register i;
	static char etherbuf[18];
	register char *cp = etherbuf;

	for (i = 0; i < 6; i++) {
		*cp++ = digits[*ap >> 4];
		*cp++ = digits[*ap++ & 0xf];
		*cp++ = ':';
	}
	*--cp = 0;
	return (etherbuf);
}

#ifdef INCLUDE_REVARP /* XXX currently not supported */
/*
 * Called from 10 Mb/s Ethernet interrupt handlers
 * when ether packet type ETHERTYPE_REVARP
 * is received.  Common length and type checks are done here,
 * then the protocol-specific routine is called.
 */
void
revarpinput(m)
	struct mbuf *m;
{
	struct arphdr *ar;
	int op, s;

	if (m->m_len < sizeof(struct arphdr))
		goto out;
	ar = mtod(m, struct arphdr *);
	if (ntohs(ar->ar_hrd) != ARPHRD_ETHER)
		goto out;
	if (m->m_len < sizeof(struct arphdr) + 2 * (ar->ar_hln + ar->ar_pln))
		goto out;
	switch (ntohs(ar->ar_pro)) {

	case ETHERTYPE_IP:
	case ETHERTYPE_IPTRAILERS:
		in_revarpinput(m);
		return;

	default:
		break;
	}
out:
	m_freem(m);
}

/*
 * RARP for Internet protocols on 10 Mb/s Ethernet.
 * Algorithm is that given in RFC 903.
 * We are only using for bootstrap purposes to get an ip address for one of
 * our interfaces.  Thus we support no user-interface.
 *
 * Since the contents of the RARP reply are specific to the interface that
 * sent the request, this code must ensure that they are properly associated.
 *
 * Note: also supports ARP via RARP packets, per the RFC.
 */
in_revarpinput(m)
	struct mbuf *m;
{
	struct ifnet *ifp;
	struct ether_arp *ar;
	int op, s;

	ar = mtod(m, struct ether_arp *);
	op = ntohs(ar->arp_op);
	switch (op) {
	case ARPOP_REQUEST:
	case ARPOP_REPLY:	/* per RFC */
		in_arpinput(m);
		return;
	case ARPOP_REVREPLY:
		break;
	case ARPOP_REVREQUEST:	/* handled by rarpd(8) */
	default:
		goto out;
	}
	if (!revarp_in_progress)
		goto out;
	ifp = m->m_pkthdr.rcvif;
	if (ifp != myip_ifp) /* !same interface */
		goto out;
	if (myip_initialized)
		goto wake;
	if (bcmp((char *)ar->arp_tha, 
		 (char *)((struct arpcom *)ifp)->ac_enaddr,
		 sizeof(ar->arp_tha)))
		goto out;
	bcopy((caddr_t)ar->arp_spa, (caddr_t)&srv_ip, sizeof(srv_ip));
	bcopy((caddr_t)ar->arp_tpa, (caddr_t)&myip, sizeof(myip));
	myip_initialized = 1;

wake:	/* Do wakeup every time in case it was missed. */
#if 0 /* XXX to be checked */
	wakeup((caddr_t)&myip);
#endif /* XXX to be checked */

out:
	m_freem(m);
}

/*
 * Send a RARP request for the ip address of the specified interface.
 * The request should be RFC 903-compliant.
 */
void
revarprequest(ifp)
	struct ifnet *ifp;
{
	struct sockaddr sa;
	struct mbuf *m;
	struct ether_header *eh;
	struct ether_arp *ea;
	struct arpcom *ac = (struct arpcom *)ifp;

	if ((m = mHdrClGet(M_DONTWAIT, MT_DATA, 
			   sizeof(*ea), TRUE)) == NULL)
	    return; 
	m->m_len = sizeof(*ea);
	m->m_pkthdr.len = sizeof(*ea);
	MH_ALIGN(m, sizeof(*ea));
	ea = mtod(m, struct ether_arp *);
	eh = (struct ether_header *)sa.sa_data;
	bzero((caddr_t)ea, sizeof(*ea));
	bcopy((caddr_t)etherbroadcastaddr, (caddr_t)eh->ether_dhost,
	    sizeof(eh->ether_dhost));
	eh->ether_type = htons(ETHERTYPE_REVARP);
	ea->arp_hrd = htons(ARPHRD_ETHER);
	ea->arp_pro = htons(ETHERTYPE_IP);
	ea->arp_hln = sizeof(ea->arp_sha);	/* hardware address length */
	ea->arp_pln = sizeof(ea->arp_spa);	/* protocol address length */
	ea->arp_op = htons(ARPOP_REVREQUEST);
	bcopy((caddr_t)ac->ac_enaddr, (caddr_t)ea->arp_sha,
	   sizeof(ea->arp_sha));
	bcopy((caddr_t)ac->ac_enaddr, (caddr_t)ea->arp_tha,
	   sizeof(ea->arp_tha));
	sa.sa_family = AF_UNSPEC;
	sa.sa_len = sizeof(sa);
	ifp->if_output(ifp, m, &sa, (struct rtentry *)0);
}

/*
 * RARP for the ip address of the specified interface, but also
 * save the ip address of the server that sent the answer.
 * Timeout if no response is received.
 */
int
revarpwhoarewe(ifp, serv_in, clnt_in)
	struct ifnet *ifp;
	struct in_addr *serv_in;
	struct in_addr *clnt_in;
{
	int result, count = 20;
	
	if (myip_initialized) 
		return EIO;

	myip_ifp = ifp;
	revarp_in_progress = 1;
	while (count--) {
		revarprequest(ifp);
		
		/* one second delay */

		taskDelay (sysClkRateGet());

#if 0 /* XXX WORKAROUND with TAskDelay provided is it OK??? */
		result = tsleep((caddr_t)&myip, PSOCK, "revarp", hz/2);
		if (result != EWOULDBLOCK)
			break;
#endif  /* XXX */
	}
	revarp_in_progress = 0;
	if (!myip_initialized)
		return ENETUNREACH;
	
	bcopy((caddr_t)&srv_ip, (char *)serv_in, sizeof(*serv_in));
	bcopy((caddr_t)&myip, (char *)clnt_in, sizeof(*clnt_in));
	return 0;
}

/* For compatibility: only saves interface address. */
int
revarpwhoami(in, ifp)
	struct in_addr *in;
	struct ifnet *ifp;
{
	struct in_addr server;
	return (revarpwhoarewe(ifp, &server, in));
}
#endif /* INCLUDE_REVARP XXX currently not supported */

#define db_printf	printf
void
db_print_sa(sa)
	struct sockaddr *sa;
{
	int len;
	u_char *p;

	if (sa == 0) {
		db_printf("[NULL]");
		return;
	}

	p = (u_char*)sa;
	len = sa->sa_len;
	db_printf("[");
	while (len > 0) {
		db_printf("%d", *p);
		p++; len--;
		if (len) db_printf(",");
	}
	db_printf("]\n");
}

#ifdef	DDB
static void
db_print_ifa(ifa)
	struct ifaddr *ifa;
{
	if (ifa == 0)
		return;
	db_printf("  ifa_addr=");
	db_print_sa(ifa->ifa_addr);
	db_printf("  ifa_dsta=");
	db_print_sa(ifa->ifa_dstaddr);
	db_printf("  ifa_mask=");
	db_print_sa(ifa->ifa_netmask);
	db_printf("  flags=0x%x,refcnt=%d,metric=%d\n",
			  ifa->ifa_flags,
			  ifa->ifa_refcnt,
			  ifa->ifa_metric);
}
static void
db_print_llinfo(li)
	caddr_t li;
{
	struct llinfo_arp *la;

	if (li == 0)
		return;
	la = (struct llinfo_arp *)li;
	db_printf("  la_rt=0x%x la_hold=0x%x, la_asked=0x%x\n",
			  la->la_rt, la->la_hold, la->la_asked);
}
/*
 * Function to pass to rn_walktree().
 * Return non-zero error to abort walk.
 */
static int
db_show_radix_node(rn, w)
	struct radix_node *rn;
	void *w;
{
	struct rtentry *rt = (struct rtentry *)rn;

	db_printf("rtentry=0x%x", rt);

	db_printf(" flags=0x%x refcnt=%d use=%d expire=%d\n",
			  rt->rt_flags, rt->rt_refcnt,
			  rt->rt_use, rt->rt_expire);

	db_printf(" key="); db_print_sa(rt_key(rt));
	db_printf(" mask="); db_print_sa(rt_mask(rt));
	db_printf(" gw="); db_print_sa(rt->rt_gateway);

	db_printf(" ifp=0x%x ", rt->rt_ifp);
	if (rt->rt_ifp)
		db_printf("(%s%d)",
				  rt->rt_ifp->if_name,
				  rt->rt_ifp->if_unit);
	else
		db_printf("(NULL)");

	db_printf(" ifa=0x%x\n", rt->rt_ifa);
	db_print_ifa(rt->rt_ifa);

	db_printf(" genmask="); db_print_sa(rt->rt_genmask);

	db_printf(" gwroute=0x%x llinfo=0x%x\n",
			  rt->rt_gwroute, rt->rt_llinfo);
	db_print_llinfo(rt->rt_llinfo);

	return (0);
}
/*
 * Function to print all the route trees.
 * Use this from ddb:  "call db_show_arptab"
 */
db_show_arptab()
{
	struct radix_node_head *rnh;
	rnh = rt_tables[AF_INET];
	db_printf("Route tree for AF_INET\n");
	if (rnh == NULL) {
		db_printf(" (not initialized)\n");
		return (0);
	}
	rn_walktree(rnh, db_show_radix_node, NULL);
	return (0);
}
#endif
#endif /* INET */
