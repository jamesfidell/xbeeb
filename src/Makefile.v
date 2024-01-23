###
### $Id: Makefile.v,v 1.15 2002/01/15 15:46:43 james Exp $
###
### Copyright (C) James Fidell 1994-2002.
###
### Permission to use, copy, modify and distribute this software
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
### Modification History
###
### $Log: Makefile.v,v $
### Revision 1.15  2002/01/15 15:46:43  james
### *** empty log message ***
###
### Revision 1.14  2000/09/02 18:48:26  james
### Changed all VoxWare references to OSS
###
### Revision 1.13  2000/08/16 17:58:28  james
### Update copyright message
###
### Revision 1.12  1996/10/12 16:02:13  james
### Moved my own version of the strcasecmp function into Compat.c
###
### Revision 1.11  1996/10/08 00:04:32  james
### Added InfoWindow to show LED status.  Also required addition of the
### SHIFTLOCK_SOUND_HACK to prevent the Shift Lock LED being light up
### whenever the sound buffer is full, which means that far too much
### time can be spent re-drawing the LED.
###
### Revision 1.10  1996/10/01 22:30:33  james
### Added VoxWare sound code from James Murray <jsm@jsm-net.demon.co.uk>.
###
### Revision 1.9  1996/10/01 22:10:01  james
### Split keyboard handling into kEYMAP_STRICT and KEYMAP_LEGEND models.
###
### Revision 1.8  1996/09/30 23:39:34  james
### Split out option processing into Options.[ch].  Updated the help message,
### added support for the Model A using the -a switch (and added the
### MODEL_B_ONLY #define in Config.h, added the -m and -s switches to set the
### initial screen mode and keyboard DIP switches.
###
### Revision 1.7  1996/09/24 23:05:39  james
### Update copyright dates.
###
### Revision 1.6  1996/09/23 19:07:45  james
### Mode7.c is now called Teletext.c
###
### Revision 1.5  1996/09/23 16:09:51  james
### Initial implementation of bitmap MODEs -- including modification of
### screen handling to use different windows for teletext and bitmapped
### modes and corrections/improvements to colour- and cursor-handling
### code.
###
### Revision 1.4  1996/09/22 19:23:21  james
### Add the emulated filing system code.
###
### Revision 1.3  1996/09/21 18:32:59  james
### Renamed Floppy.[ch] to Disk.[ch]
###
### Revision 1.2  1996/09/21 18:16:41  james
### Todo.c should be Sound.c
###
### Revision 1.1  1996/09/21 17:20:38  james
### Source files moved to src directory.
###
### Revision 1.1.1.1  1996/09/21 13:52:48  james
### Xbeeb v0.1 initial release
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
CFLAGS		= -Wall -O

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
			  Screen.c Modes.c Teletext.c Display.c Sound.c Bitmap.c EFS.c \
			  SoundOSS.c Options.c KeymapStrict.c KeymapLegend.c InfoWindow.c \
			  Compat.c

OBJS		= $(SRCS:.c=.o)

PROG		= xbeeb
LIBS		= -lX11 $(NETLIBS) -lm


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

depend		:
	$(DEPEND) -s '# DO NOT DELETE' -- -- $(SRCS)

#
# Below are dependencies created by makedepend...
# DO NOT DELETE
