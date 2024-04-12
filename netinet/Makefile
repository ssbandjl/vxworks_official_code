# Makefile - makefile for vw/src/netinet
#
# modification history
# --------------------
# 01d,11feb00,gnn  first checkin to openstack (Siemens ONLY)
# 01c,04nov99,pul  addint debugging symbols
# 01b,28feb97,vin  sorted and merged bsd44 to tor1_0_1
# 01a,18jun96,yp   created from 01b of MakeSkel
#
# DESCRIPTION
# This file contains the makefile rules for building the vx library
#
#*/

TGT_DIR=$(WIND_BASE)/target

LIBNAME=lib$(CPU)$(TOOL)vx.a
LIBDIRNAME=obj$(CPU)$(TOOL)vx
ADDED_CFLAGS=-g -O0

OBJS=	if.o if_ether.o if_subr.o igmp.o in.o in_cksum.o in_pcb.o \
	in_proto.o ip_icmp.o ip_input.o ip_mroute.o ip_output.o radix.o \
	raw_cb.o raw_ip.o raw_usrreq.o route.o rtsock.o sl_compress.o \
	sys_socket.o tcp_debug.o tcp_input.o tcp_output.o tcp_subr.o \
	tcp_timer.o tcp_usrreq.o udp_usrreq.o uipc_dom.o uipc_mbuf.o \
	uipc_sock.o uipc_sock2.o unixLib.o \

include $(TGT_DIR)/h/make/rules.library

