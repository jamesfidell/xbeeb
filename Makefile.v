###
### $Id$
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
### Modification History
###
### $Log$
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
			  Floppy.c Econet.c ADC.c TubeUla.c 6522Via.c Keyboard.c \
			  Screen.c Modes.c Mode7.c Display.c \
			  Todo.c

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

depend		:
	$(DEPEND) -s '# DO NOT DELETE' -- -- $(SRCS)

#
# Below are dependencies created by makedepend...
# DO NOT DELETE
