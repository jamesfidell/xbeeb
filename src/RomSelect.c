/*
 *
 * $Id: RomSelect.c,v 1.7 2002/01/15 15:46:43 james Exp $
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
 * $Log: RomSelect.c,v $
 * Revision 1.7  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.6  2000/08/16 17:58:28  james
 * Update copyright message
 *
 * Revision 1.5  1996/10/10 22:00:54  james
 * Added missing brackets around code block for checking for paged RAM changes
 *
 * Revision 1.4  1996/09/30 23:51:03  james
 * Fixed handling of paged RAM banks.
 *
 * Revision 1.3  1996/09/24 23:05:42  james
 * Update copyright dates.
 *
 * Revision 1.2  1996/09/21 22:13:49  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.1  1996/09/21 17:20:39  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#include <stdio.h>
#include <unistd.h>
#include <memory.h>

#include "Config.h"
#include "RomSelect.h"
#include "Memory.h"

byteval			RomSelectRegister;

byteval
ReadPagedRomSelector ( int addr )
{
	/*
	 * The manuals say that this is a write-only register, but the
	 * OS definitely tries to read it often enough, so I'll try to
	 * do something sane and return the value it contains.
	 */

#ifdef	DEBUG
	fprintf ( stderr, "Warning: read from ROM select register\n" );
#endif
	return RomSelectRegister;
}


void
WritePagedRomSelector ( int addr, byteval val )
{
	/*
	 * All addresses map to the same register here, so there's
	 * no need to check the address -- just write a value (a 4-bit
	 * value, though ).
	 *
	 * Don't do anything if this register is already set to
	 * the same value
	 */

	val &= 0xf;
	if ( val == RomSelectRegister )
		return;

	/*
	 * check if we need to modify any of the paged RAM slots
	 */

	if ( PageWrite [ RomSelectRegister ] )
	{
		if ( PagedRAMChanged )
		{
			memcpy ( PagedMem [ RomSelectRegister ], &Mem [ 32768 ], 16384 );
			PagedRAMChanged = 0;
		}
	}

	RomSelectRegister = val;
#ifdef	INFO
	printf ( "ROM select register set to %x\n", RomSelectRegister );
#endif

	/*
	 * Now copy the new page into real memory.
	 */

	memcpy ( &Mem [ 32768 ], PagedMem [ val ], 16384 );
	return;
}


int
SaveRomSelect ( int fd )
{
	byteval				romsel [ 8 ];

	romsel [ 0 ] = RomSelectRegister;

	if ( write ( fd, romsel, 8 ) != 8 )
		return -1;

	return 0;
}


int
RestoreRomSelect ( int fd, unsigned int ver )
{
	byteval				romsel [ 8 ];

	if ( ver > 1 )
		return -1;

	if ( read ( fd, romsel, 8 ) != 8 )
		return -1;

	RomSelectRegister = romsel [ 0 ];
	memcpy ( &Mem [ 32768 ], PagedMem [ RomSelectRegister ], 16384 );

	return 0;
}
