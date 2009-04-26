
include $(TOPDIR)/config.mk

LIB	= lib$(BOARD).a

OBJS	:=
OBJS	+= $(BOARD).o
OBJS	+= ../common/ppc_ide.o
OBJS	+= ../common/mmc.o
OBJS	+= ../common/cmd_bootmenu_cmds.o
OBJS	+= ../common/flash_imgs.o
OBJS	+= ../common/front_dev_7seg.o
OBJS	+= ../common/video.o
OBJS	+= ../common/logo.o
OBJS	+= ../common/mIDSelect.o
OBJS    += ../common/vmode_Select.o
OBJS	+= ../common/cmd_bootmenu.o
OBJS	+= ../common/eeprom_db.o
OBJS	+= ../common/flash.o
OBJS	+= ../common/front.o
OBJS	+= ../common/gpio.o
OBJS	+= ../common/serial_set2set.o
OBJS	+= ../common/serial_upgrade.o
OBJS	+= ../common/upgrade.o
OBJS    += ../common/netBoot.o

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
