/*
 *
 * $Id$
 *
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

/*
 * Modification History
 *
 * $Log$
 *
 */


#ifndef	MEMORY_H
#define	MEMORY_H

extern	void			LoadOS ( char* );
extern	void			LoadPagedRom ( char*, unsigned char );
extern	int				SaveUserMemory ( int );
extern	int				RestoreUserMemory ( int, unsigned int );
extern	void			WriteByte ( unsigned int, unsigned char );

extern	unsigned char	Mem [ 65536 ];
extern	unsigned char	PageWrite [ 16 ];
extern	unsigned char	PagedMem [ 16 ][ 16384 ];
extern	unsigned char	PagedRAMChanged;

#ifdef	LITTLE_ENDIAN

#define	ReadWordAtPC()	*(( unsigned short * ) ( EmulatorPC ))
#define	ReadWord(a)		( *(( unsigned short * ) &( Mem [ a ] )))
#define	WriteWord(a,v)	*(( unsigned short * ) &( Mem [ a ] )) = v

#else

#define ReadWordAtPC()	*(( EmulatorPC ) + 256 * ( * ( EmulatorPC + 1 )))
#define	ReadWord(a)		( Mem [ a ] + 256 * Mem [ a + 1 ] )
#define	WriteWord(a,v)	Mem [ a ] = v & 0xff; Mem [ a + 1 ] = v >> 8

#endif	/* LITTLE_ENDIAN */


#ifdef NO_FRED_JIM

#define ReadByte(a) \
	((( a >> 8 ) != 0xfe ) ? Mem [ a ] : \
				ReadSheila ( a ))

#else

#define ReadByte(a) \
	(( a < 0xfc00 || a >= 0xff00 ) ? Mem [ a ] : \
		(( a < 0xfd00 ) ? ReadFred ( a ) : \
			(( a < 0xfe00 ) ? ReadJim ( a ) : \
				ReadSheila ( a ))))

#endif	/* NO_FRED_JIM */


#define ReadFastByte(a)			Mem [ a ]


/*
 * Functions to speed up access to pages zero and one
 */

#ifdef	LO_PAGE

#define	ReadLoPageByte(a)		( Mem [ a ] )
#define	WriteLoPageByte(a,v)	Mem [ a ] = v

#else	/* LO_PAGE */

#define	ReadLoPageByte			ReadByte
#define	WriteLoPageByte			WriteByte

#endif	/* LO_PAGE */

#endif	/* MEMORY_H */
