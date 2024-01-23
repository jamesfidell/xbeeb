/*
 * Copyright (c) James Fidell 1994.
 *
 * Permission to use, copy, modify, distribute, and sell this software
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


extern byteval					SnapshotRequested;
extern byteval					QuitEmulator;
extern unsigned char			DebugLevel;
extern byteval					MaskableInterruptRequest;

#define	SetProgramCounter(v)	EmulatorPC = Mem + v
#define	GetProgramCounter		( EmulatorPC - Mem )

/*
 * The addresses of the OS call vectors
 */

#define	BYTEV	0x20a
#define FILEV	0x212
#define	ARGSV	0x214
#define	BGETV	0x216
#define	BPUTV	0x218
#define	GBPBV	0x21a
#define	FINDV	0x21c
#define	FSCV	0x21e


/*
 * Cause an IRQ
 */

#define	IRQ()	 MaskableInterruptRequest++


/*
 * Debugging Levels
 */

#define	DISASSEMBLE		0x01

/*
 * funcs.
 */

extern void		FatalError();
