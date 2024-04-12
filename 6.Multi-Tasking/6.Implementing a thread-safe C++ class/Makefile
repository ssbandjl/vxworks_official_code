##################################################################
# Makefile - makefile skeleton
#
# Copyright (C) 1998 Wind River Systems, Inc.
#


CPU            = ARM7TDMI
TOOL           = gnu


include $(WIND_BASE)/target/h/make/defs.bsp
include $(WIND_BASE)/target/h/make/make.$(CPU)$(TOOL)
include $(WIND_BASE)/target/h/make/defs.$(WIND_HOST_TYPE)
include $(WIND_BASE)/target/h/make/rules.bsp



##################################################################
## New and redefined macros 

PROJECT        = tQue
DEBUG_OPT      = -O2
ADDED_C++FLAGS = $(DEBUG_OPT)
ADDED_CFLAGS   = $(DEBUG_OPT)



##################################################################
## ADDITIONAL OBJECT MODULES ##

OBJECTS = testQ.o cpstring.o



##################################################################
## PROJECT RULES


$(PROJECT).out : $(PROJECT).o
	$(NM) $(PROJECT).o | munch > ctdt.c
	$(MAKE) CC_COMPILER="-traditional" ctdt.o
	$(LD) $(LD_PARTIAL_FLAGS) -o $@ $(PROJECT).o ctdt.o
	@ $(RM) ctdt.c ctdt.o

$(PROJECT).o : $(OBJECTS)
	$(LD) $(LD_PARTIAL_FLAGS) -o $@ $(OBJECTS)



##################################################################
## Inference rules for C++ compiles on files with a .cc extension

.cc.o :
	@ $(RM) $@
	$(CXX) $(C++FLAGS) -c $<

# C++ compile and munch
.cc.out :
	@ $(RM) $@
	$(CXX) $(C++FLAGS) -c $<
	@ $(RM) $@ ctdt.c
	$(NM) $*.o | munch > ctdt.c
	$(MAKE) CC_COMPILER="-traditional" ctdt.o
	$(LD) $(LD_PARTIAL_FLAGS) -o $@ $*.o ctdt.o
	@ $(RM) ctdt.c ctdt.o




##################################################################
## END MAKEFILE
