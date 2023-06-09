#------------------------------------------------------------------------------
# Copyright (c) 2007, Stanford University
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of Stanford University nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#------------------------------------------------------------------------------ 


#------------------------------------------------------------------------------
# File: Makefile
# 
# Note: This Makefile requires GNU make.
#------------------------------------------------------------------------------

CC = gcc

OSTYPE = $(shell uname)

ifeq ($(OSTYPE),CYGWIN_NT-5.1)
ARCH = -D_CYGWIN_
endif

ifeq ($(OSTYPE),Linux)
ARCH = -D_LINUX_
endif

ifeq ($(OSTYPE),SunOS)
ARCH =  -D_SOLARIS_
endif

ifeq ($(OSTYPE),Darwin)
ARCH = -D_DARWIN_
endif

PATH_OUT = ./$(OSTYPE)/

CFLAGS = -Wall $(ARCH) -O3
#LIBS = -lm -lpthread -lrt -lcpc
LIBS = -lm -lpthread -lrt 

STR_MATCH_SER_OBJS = string_match_serial.o
STR_MATCH_PTHREADS = string_match_pthreads.o
STR_MATCH_OBJS = string_match.o
COM_OBJS = MapReduceScheduler.o

STR_MATCH_DRAMAP_OBJS = string_match_dramap.o

PROGS = string_match string_match_serial string_match_pthreads

.PHONY: default clean

all: $(PROGS)

#string_match_serial: $(COM_OBJS) $(STR_MATCH_SER_OBJS)
string_match_serial: $(STR_MATCH_SER_OBJS)
	$(CC) $(CFLAGS) -o $@ $(STR_MATCH_SER_OBJS) $(LIBS)
	
string_match_pthreads: $(COM_OBJS) $(STR_MATCH_PTHREADS)
	$(CC) $(CFLAGS) -o $@ $(COM_OBJS) $(STR_MATCH_PTHREADS) $(LIBS)
	
string_match: $(COM_OBJS) $(STR_MATCH_OBJS)
	$(CC) $(CFLAGS) -o $@ $(COM_OBJS) $(STR_MATCH_OBJS) $(LIBS)

string_match_dramap: $(STR_MATCH_DRAMAP_OBJS)
	$(CC) $(CFLAGS) -o $@ $(STR_MATCH_DRAMAP_OBJS) $(LIBS)	

clean:
	rm -f $(PROGS) *.o *.a *~ *.tmp *.bak *.log
