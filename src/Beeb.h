/*
 *
 * $Id: Beeb.h,v 1.14 2002/01/15 15:46:43 james Exp $
 *
 * Copyright (C) James Fidell 1994-2002.
 *
 * Permission to use, copy, modify and distribute this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of the copyright holder
 * not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission. The
 * copyright holder makes no representations about the suitability of
 * this software for any purpose. It is provided "as is" without express
 * or implied warranty.
 *
 * THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

/*
 * Modification History
 *
 * $Log: Beeb.h,v $
 * Revision 1.14  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.13  2000/08/16 17:58:26  james
 * Update copyright message
 *
 * Revision 1.12  1996/11/24 21:54:06  james
 * Change MIN to XBEEB_MIN to avoid clashes with some OSes.
 *
 * Revision 1.11  1996/10/13 12:13:27  james
 * Parenthesise all parameters to #defined macros.
 *
 * Revision 1.10  1996/10/01 22:09:58  james
 * Split keyboard handling into kEYMAP_STRICT and KEYMAP_LEGEND models.
 *
 * Revision 1.9  1996/09/30 23:39:30  james
 * Split out option processing into Options.[ch].  Updated the help message,
 * added support for the Model A using the -a switch (and added the
 * MODEL_B_ONLY #define in Config.h, added the -m and -s switches to set the
 * initial screen mode and keyboard DIP switches.
 *
 * Revision 1.8  1996/09/30 22:59:07  james
 * Prevent multiple inclusion of header files.
 *
 * Revision 1.7  1996/09/24 23:05:34  james
 * Update copyright dates.
 *
 * Revision 1.6  1996/09/23 16:36:08  james
 * Added OS file-handling call vector #defines
 *
 * Revision 1.5  1996/09/22 21:00:54  james
 * Changed IRQ-handling code to be called only when I think an IRQ may have
 * happened.  Converted it all to #defines so that it can be compiled inline.
 *
 * Revision 1.4  1996/09/22 20:20:38  james
 * Corrections to DISASS code.
 *
 * Revision 1.3  1996/09/21 23:07:35  james
 * Call FatalError() rather than exit() so that screen stuff etc. can
 * be cleaned up.
 *
 * Revision 1.2  1996/09/21 22:39:52  james
 * Improved handling of instruction disassembly.
 *
 * Revision 1.1  1996/09/21 17:20:36  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#ifndef	BEEB_H
#define	BEEB_H

extern char						*OsRomName;
extern char						*Rom15Name;
extern char						*SnapshotName;

extern unsigned char			SnapshotRequested;
extern unsigned char			QuitEmulator;
extern unsigned char			BreakKeypress;
extern unsigned char			DebugLevel;
extern unsigned char			MaskableInterruptRequest;

#define	SetProgramCounter(v)	EmulatorPC = Mem + ( v )
#define	GetProgramCounter		( EmulatorPC - Mem )

/*
 * The addresses of the OS call vectors
 */

#define	BYTEV	0x20a
#define	FILEV	0x212
#define	ARGSV	0x214
#define	BGETV	0x216
#define	BPUTV	0x218
#define	GBPBV	0x21a
#define	FINDV	0x21c
#define	FSCV	0x21e

/*
 * Cause an IRQ
 */

#define	IRQ()				MaskableInterruptRequest++

/*
 * Return the minimum of two values
 */

#define	XBEEB_MIN(x,y)		((( x ) < ( y )) ? ( x ) : ( y ))


/*
 * Debugging Levels
 */

#define	DISASSEMBLE			0x01

/*
 * funcs.
 */

extern void					FatalError();

#endif	/* BEEB_H */
