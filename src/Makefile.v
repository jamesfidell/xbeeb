###
### Copyright (c) James Fidell 1994.
###
### Permission to use, copy, modify, distribute, and sell this software
### and its documentation for any purpose is hereby granted without fee,
### provided that the above copyright notice appear in all copies and
### that both that copyright notice and this permission notice appear in
### supporting documentation, and that the name of the copyright holder
### not be used in advertising or publicity pertaining to distribution
### of the software without specific, written prior permission. The
### copyright holder makes no representations about the suitability of
### this software for any purpose. It is provided "as is" without express
### or implied warranty.
###
### THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
### SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
### FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
### SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
### RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
### CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
### CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
###
###

###
### Configuration
###

#
# set NETLIBS to the libraries to link for networking support (sockets etc.)
#
# NETLIBS		= -lsocket -lnsl	# SVR4
# NETLIBS		= -lsocket			# SCO
NETLIBS		=					# Linux

#
# C compiler and C compiler options
#
CC			= gcc
CFLAGS		= -Wall -O2

#
# makedepend
#
# DEPEND		= makedepend	# Most systems
DEPEND		= gccmakedep	# Linux

#
# ... and a few other bits...
#

RM			= rm -f
LDFLAGS		=

###
### End of Makefile configuration
###

SRCS		= Beeb.c Memory.c Fred.c Jim.c Sheila.c Crtc.c Acia.c \
			  SerialUla.c VideoUla.c RomSelect.c SystemVia.c UserVia.c \
			  Disk.c Econet.c ADC.c TubeUla.c 6522Via.c Keyboard.c \
			  Screen.c Modes.c Teletext.c Display.c Sound.c Bitmap.c EFS.c

OBJS		= $(SRCS:.c=.o)

PROG		= xbeeb
LIBS		= -lX11 $(NETLIBS)


all			: $(PROG)

$(PROG)		: $(OBJS)
	$(RM) $@
	$(CC) $(LDFLAGS) -o $(PROG) $(OBJS) $(LIBS)

profile		:
	make CFLAGS="$(CFLAGS) -p" LDFLAGS=-p

lprofile	:
	make CC=cc CFLAGS=-ql LDFLAGS=-ql

clean		:
	$(RM) $(OBJS) $(PROG)

###
### Dependencies
###

Beeb.o		: Beeb.c Patchlevel.h Config.h Beeb.h 6502.h Display.h ADC.h \
			  Acia.h Crtc.h Econet.h Disk.h RomSelect.h Fred.h Jim.h Sheila.h \
			  Memory.h SystemVia.h 6522Via.h UserVia.h Keyboard.h Screen.h \
			  TubeUla.h SerialUla.h VideoUla.h Ops.h opcodes.h EFS.h 6502.c

Memory.o	: Memory.c Config.h 6502.h Memory.h Fred.h Jim.h Sheila.h \
  			  Screen.h Modes.h

Fred.o		: Fred.c Config.h Fred.h

Jim.o		: Jim.c Config.h Jim.h

Sheila.o	: Sheila.c Config.h Sheila.h Crtc.h Acia.h SerialUla.h VideoUla.h \
			  RomSelect.h UserVia.h 6522Via.h SystemVia.h Disk.h Econet.h \
			  ADC.h TubeUla.h

Crtc.o		: Crtc.c Config.h Crtc.h Screen.h Modes.h Bitmap.h VideoUla.h \
			  Memory.h

Acia.o		: Acia.c Config.h SerialUla.h Acia.h

VideoUla.o	: VideoUla.c Config.h VideoUla.h Crtc.h Modes.h Bitmap.h Screen.h

RomSelect.o	: RomSelect.c Config.h RomSelect.h Memory.h

SystemVia.o	: SystemVia.c Config.h 6502.h Beeb.h SystemVia.h 6522Via.h \
			  Keyboard.h Screen.h Sound.h

UserVia.o	: UserVia.c Config.h 6502.h Beeb.h UserVia.h 6522Via.h

Disk.o		: Disk.c Config.h Disk.h

Econet.o	: Econet.c Config.h Econet.h

ADC.o		: ADC.c Config.h ADC.h

TubeUla.o	: TubeUla.c Config.h TubeUla.h

6522Via.o	: 6522Via.c Config.h 6522Via.h SystemVia.h UserVia.h Screen.h \
			  Modes.h

Keyboard.o	: Keyboard.c Config.h Keyboard.h SystemVia.h 6522Via.h

Screen.o	: Screen.c Config.h Beeb.h Screen.h Modes.h Teletext.h Bitmap.h
			  Keyboard.h Memory.h EFS.h

Modes.o		: Modes.c Config.h Modes.h

Teletext.o	: Teletext.c Config.h Teletext.h Memory.h Screen.h Modes.h Crtc.h \
			  VideoUla.h

Display.o	: Display.c Config.h 6502.h

Sound.o		: Sound.c Config.h Sound.h

Bitmap.o	: Bitmap.c Config.h Bitmap.h Memory.h Screen.h Modes.h Crtc.h \
			  VideoUla.h

EFS.o		: EFS.c Config.h EFS.h Memory.h