# Makefile - makefile for vw/src/netinet/ppp
#
# modification history
# --------------------
# 01b,14jul97,vin  removed ppp_slcompress.o
# 01a,18jun96,yp   created from 01d of MakeSkel
#
# DESCRIPTION
# This file contains the makefile rules for building the VxWorks PPP product
#
#*/

TGT_DIR=$(WIND_BASE)/target

DOC_FILES=	pppLib.c pppShow.c pppSecretLib.c pppHookLib.c

LIBNAME=lib$(CPU)$(TOOL)vx.a
LIBDIRNAME=obj$(CPU)$(TOOL)vx

OBJS=	pppLib.o magic.o fsm.o lcp.o ipcp.o upap.o chap.o ppp_md5.o crypt.o \
	auth.o options.o ppp_vxworks.o if_ppp.o random.o \
	pppShow.o pppSecretLib.o pppHookLib.o

include $(TGT_DIR)/h/make/rules.library

