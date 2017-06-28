.SUFFIXES:
.SUFFIXES: .c .o
################################################################################
#                                                                              #
#   Dark Risings Source                                                        #
#   Makefile written by Glenn K. Lockwood (05/26/09)                           #
#   GAME PORT and BUILD PORT                                                   #
################################################################################

# default "make" is for x86/Linux using GNU compilers
CC	= gcc
PROF	= -O -Wall -g -finline-functions
PROF	= -Wall -g -Dunix
CFLAGS	= $(PROF)
LFLAGS	= $(PROF)
LIBS	=  -lcrypt -lnsl
ARCHIVE_PREFIX = $(shell date +"%Y%m%d-%H.%M.%S")
BUILD_PORT_EXE = ~/build-port/src/rom
TAR = tar

# Compile using Sun Studio
#CC	= cc
#PROF	= -erroff=%none -O -g
#CFLAGS	= $(PROF)
#LFLAGS	= $(PROF)
#LIBS	= -lcrypt -lnsl -lsocket

OBJS = act_comm.o act_enter.o act_info.o act_move.o act_obj.o act_wiz.o \
	ban.o clans.o comm.o const.o db.o db2.o effects.o fight.o flags.o \
	handler.o healer.o interp.o note.o lookup.o magic.o magic2.o \
	music.o recycle.o save.o scan.o skills.o special.o tables.o \
	update.o mob_cmds.o mob_prog.o olc.o olc_act.o olc_save.o bit.o \
	mem.o string.o olc_mpcode.o bank.o wild.o were.o vampire.o \
	starmap.o pueblo.o multilogin.o trades.o social-edit.o \
	templar.o journal.o brawler.o seraphs.o ninja.o jousting.o

%.o: %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.o
	$(CC) -MM $(CFLAGS) $*.c > $*.d

rom: $(OBJS)
	$(CC) $(LFLAGS) -o rom $(OBJS) $(LIBS)

.PHONY: clean distclean archive install

clean:	
	rm -f rom $(OBJS) *.d

distclean: clean
	rm -f \#* *~

archive:
	$(TAR) czf $(ARCHIVE_PREFIX)-drsrc-game-port.tar.gz *.c *.h Makefile

install: rom
	mv $(BUILD_PORT_EXE) $(BUILD_PORT_EXE)-$(ARCHIVE_PREFIX)-bak
	cp rom $(BUILD_PORT_EXE)
