#
# (C) Copyright 2000
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	= lib$(BOARD).a

OBJS	:=
OBJS	+= $(BOARD).o
OBJS	+= cmd_bootmenu_cmds.o
OBJS	+= video.o
OBJS	+= front_dev.o
OBJS	+= ../common/cmd_bootmenu.o
OBJS	+= ../common/flash.o
OBJS	+= ../common/front.o
OBJS	+= ../common/gpio.o
OBJS	+= ../common/serial_upgrade.o
OBJS	+= ../common/upgrade.o
OBJS	+= ../common/vendor.o
OBJS	+= ../common/pdiusbd12/chap_9.o
OBJS	+= ../common/pdiusbd12/d12ci.o
OBJS	+= ../common/pdiusbd12/epphal.o
OBJS	+= ../common/pdiusbd12/isr.o
OBJS	+= ../common/pdiusbd12/mainloop.o

SOBJS	= init.o

all: $(LIB)

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $^

clean:
	rm -f $(SOBJS) $(OBJS)

distclean:	clean
	rm -f $(LIB) core *.bak .depend

#########################################################################

.depend: Makefile $(SOBJS:.o=.S) $(OBJS:.o=.c)
	rm -f .depend;
	$(foreach src, $(OBJS:.o=.c), $(CC) $(CFLAGS) -M -MT $(src:.c=.o) $(src) >> $@ && ) true
	$(foreach src, $(SOBJS:.o=.S), $(CC) $(CFLAGS) -M -MT $(src:.S=.o) $(src) >> $@ && ) true

sinclude .depend

#########################################################################
