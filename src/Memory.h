/*
 *
 * $Id: Memory.h,v 1.12 2002/01/15 15:46:43 james Exp $
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
 * $Log: Memory.h,v $
 * Revision 1.12  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.11  2000/08/16 17:58:28  james
 * Update copyright message
 *
 * Revision 1.10  1996/11/19 00:37:00  james
 * Improvements/corrections to stack/lo-memory handling, including one
 * nasty patch to allow the stack pointer to wrap around correctly.
 *
 * Revision 1.9  1996/10/13 12:13:32  james
 * Parenthesise all parameters to #defined macros.
 *
 * Revision 1.8  1996/10/09 22:18:55  james
 * Remove ReadFastByte macro.
 *
 * Revision 1.7  1996/09/30 23:39:35  james
 * Split out option processing into Options.[ch].  Updated the help message,
 * added support for the Model A using the -a switch (and added the
 * MODEL_B_ONLY #define in Config.h, added the -m and -s switches to set the
 * initial screen mode and keyboard DIP switches.
 *
 * Revision 1.6  1996/09/24 23:05:39  james
 * Update copyright dates.
 *
 * Revision 1.5  1996/09/24 18:22:34  james
 * Change LITTLE_ENDIAN #define to ENDIAN_6502 because of a clash with
 * header files on some systems.
 *
 * Revision 1.4  1996/09/23 16:09:51  james
 * Initial implementation of bitmap MODEs -- including modification of
 * screen handling to use different windows for teletext and bitmapped
 * modes and corrections/improvements to colour- and cursor-handling
 * code.
 *
 * Revision 1.3  1996/09/21 22:13:49  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.2  1996/09/21 18:17:25  james
 * Correction to ReadWordAtPC, add parentheses to ReadWord and WriteWord for
 * those systems that require them.
 *
 * Revision 1.1  1996/09/21 17:20:38  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#ifndef	MEMORY_H
#define	MEMORY_H

extern	void			LoadOS ( char* );
extern	void			LoadPagedRom ( char*, byteval );
extern	int				SaveUserMemory ( int );
extern	int				RestoreUserMemory ( int, unsigned int );
extern	void			WriteByte ( unsigned int, byteval );

extern	byteval			Mem [ 65536 ];
extern	byteval			ScreenCheck [ 32768 ];
extern	byteval			PageWrite [ 16 ];
extern	byteval			PagedMem [ 16 ][ 16384 ];
extern	unsigned char	PagedRAMChanged;
extern	unsigned int	MaxRAMAddress;


/*
 * FIX ME
 *
 * When MODEL_B_ONLY is not defined, ReadWordAtPC, ReadWord, WriteWord and
 * ReadByte should all take account of the missing memory.
 *
 * They don't at the moment because I'm assuming that everything will
 * stay reasonably sane without doing this...
 */

/*
 * FIX ME
 *
 * There's a potential wrap-around problem here if a == 0xffff.  I'm
 * not going to worry about that at the moment, but it may bite some
 * time.
 */

#ifdef	ENDIAN_6502

#define	ReadWordAtPC()	*(( unsigned short * ) ( EmulatorPC ))
#define	ReadWord(a)		( *(( unsigned short * ) &( Mem [ a ] )))
#define	WriteWord(a,v)	*(( unsigned short * ) &( Mem [ a ] )) = v

#else

#define ReadWordAtPC()	(( *EmulatorPC ) + 256 * ( * ( EmulatorPC + 1 )))
#define	ReadWord(a)		( Mem [ a ] + 256 * Mem [ (a) + 1 ] )
#define	WriteWord(a,v)	Mem [ a ] = (v) & 0xff; Mem [ (a) + 1 ] = (v) >> 8

#endif	/* ENDIAN_6502 */

#define	ReadStackWord(a)	( Mem [(( a + 1 ) & 0xff ) | STACK_PAGE ] << 8 | \
						Mem [(a) | STACK_PAGE ])


#ifdef NO_FRED_JIM

#define ReadByte(a) \
	(((( a ) >> 8 ) != 0xfe ) ? Mem [ a ] : \
				ReadSheila ( a ))

#else

#define ReadByte(a) \
	((( a  ) < 0xfc00 || ( a ) >= 0xff00 ) ? Mem [ a ] : \
		((( a ) < 0xfd00 ) ? ReadFred ( a ) : \
			((( a ) < 0xfe00 ) ? ReadJim ( a ) : \
				ReadSheila ( a ))))

#endif	/* NO_FRED_JIM */


/*
 * Functions to speed up access to pages zero and one.  ReadLoPageByte
 * can remain a direct memory access even if we're being paranoid, because
 * reading direct from memory that we know isn't connected with any nasty
 * hardware type stuff should be safe.
 */

#ifdef	LO_PAGE

#define	ReadLoPageByte(a)		( Mem [ a ] )
#define	WriteLoPageByte(a,v)	Mem [ a ] = v

#else	/* LO_PAGE */

#define	ReadLoPageByte(a)		( Mem [(a)] )
#define	WriteLoPageByte			WriteByte

#endif	/* LO_PAGE */

#endif	/* MEMORY_H */
