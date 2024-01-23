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


#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <memory.h>
#include <sys/stat.h>

#include "Config.h"
#include "6502.h"
#include "Memory.h"
#include "Fred.h"
#include "Jim.h"
#include "Sheila.h"
#include "Screen.h"
#include "Modes.h"

/*
 * Memory for direct access and for the screen change mirror
 *
 */

byteval 			ScreenCheck [ 32768 ];
byteval 			Mem [ 65536 ];

/*
 * Memory from 0x8000 to 0xbfff -- language and utility ROMS and possibly
 * also some RAM.  There are potentially 16 of these slots, paged in on
 * the contents of the Paged ROM selector switch.
 *
 * To work out whether to disable writes to these sections of memory,
 * we also keep a list of which pages are writeable.
 */

byteval				PagedMem [ 16 ][ 16384 ];
unsigned char		PageWrite [ 16 ] =
{
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0
};

unsigned char		PagedRAMChanged = 0;


void
WriteByte ( unsigned int addr, byteval val )
{
	/*
	 * Writing user RAM is easy -- just set the value at the
	 * appropriate address.
	 *
	 * However, we also have to check that the address is in screen
	 * memory and make sure that the screen image gets updated
	 * accordingly.
	 */

	if ( addr < 0x8000 )
	{
		Mem [ addr ] = val;
		if ( addr >= StartOfScreenMemory )
		{
			ScreenCheck [ addr ] = 1;
			ScreenChanged++;
		}
		return;
	}

	/*
	 * Paged RAM/ROM isn't too hard either -- just make sure we get the
	 * right one.  Of course, the ROM might not be writeable, so check
	 * that too.
	 */

	if ( addr < 0xc000 )
	{
		byteval		page;

		page = ReadSheila ( PagedRomSelector );
		if ( PageWrite [ page ] )
		{
			PagedRAMChanged = 1;
			Mem [ addr ] = val;
		}
		return;
	}

	/*
	 * Now skip the first bit of the OS, because that's not writeable.
	 */

#ifdef NO_FRED_JIM
	if ( addr < 0xfe00 )
#else
	if ( addr < 0xfc00 )
#endif
		return;

	/*
	 * The three memory-mapped IO pages are dealt with separately
	 */

#ifdef	NO_FRED_JIM

	if ( addr < 0xfd00 )
	{
		WriteFred ( addr, val );
		return;
	}

	if ( addr < 0xfe00 )
	{
		WriteJim ( addr, val );
		return;
	}

#endif	/* NO_FRED_JIM */

	if ( addr < 0xff00 )
	{
		WriteSheila ( addr, val );
		return;
	}

	/*
	 * Now anything left that's valid is in the OS ROM, and we
	 * know that's not writeable, so just exit.
	 */

	return;
}


void
LoadOS ( char *osname )
{
	int			fd;
	char		FnameBuffer [ PATH_MAX ];

	/*
	 * First try the name by itself, then appended to the default
	 * ROM locations...
	 */

	if (( fd = open ( osname, O_RDONLY )) < 0 )
	{
		( void ) strcpy ( FnameBuffer, XBEEBROMS );
		( void ) strcat ( FnameBuffer, osname );

		if (( fd = open ( FnameBuffer, O_RDONLY )) < 0 )
		{
			fprintf ( stderr, "Couldn't open OS ROM %s\n", osname );
			exit ( 1 );
		}
	}

	/*
	 * We assume that the OS ROM is going to be the full 16K.  Seems
	 * reasonable to me...
	 */

	if (( read ( fd, ( void* ) &Mem[49152], 16384 )) != 16384 )
	{
		fprintf ( stderr, "Couldn't read OS ROM %s\n", osname );
		close ( fd );
		exit ( 1 );
	}

	close ( fd );
	return;
}


void
LoadPagedRom ( char *romname, byteval socket )
{
	int				fd;
	unsigned long	RomSize;
	struct stat		sb;
	char			FnameBuffer [ PATH_MAX ];

	if ( stat ( romname, &sb ) < 0 )
	{
		( void ) strcpy ( FnameBuffer, XBEEBROMS );
		( void ) strcat ( FnameBuffer, romname );

		/*
		 * A short-cut for later...
		 */
		romname = FnameBuffer;
		
		if ( stat ( romname, &sb ) < 0 )
		{
			fprintf ( stderr, "Couldn't stat paged ROM %s\n", romname );
			exit ( 1 );
		}
	}

	if (( RomSize = sb.st_size ) > 16384 )
	{
		fprintf ( stderr, "Paged ROM %s is bigger than 16K\n", romname );
		exit ( 1 );
	}

	if (( fd = open ( romname, O_RDONLY )) == 0 )
	{
		fprintf ( stderr, "Couldn't open paged ROM %s\n", romname );
		exit ( 1 );
	}

	if (( read ( fd, ( void* ) PagedMem [ socket ], RomSize )) != RomSize )
	{
		fprintf ( stderr, "Couldn't read paged ROM %s\n", romname );
		close ( fd );
		exit ( 1 );
	}

	close ( fd );
	return;
}


int
SaveUserMemory ( int fd )
{
	if ( write ( fd, Mem, 32768 ) != 32768 )
		return -1;
	return 0;
}


int
RestoreUserMemory ( int fd, unsigned int ver )
{
	if ( ver > 1 )
		return -1;

	if ( read ( fd, Mem, 32768 ) != 32768 )
		return -1;

	/*
	 * Mark the whole screen as needing update
	 */

	memset (( void* ) ScreenCheck, 1, 32768 );

	return 0;
}


/*
 * And this is just to help debugging...
 */

void
SaveUserRam ( char *fname )
{
	int				fd;

	fd = open ( fname, O_WRONLY | O_CREAT | O_TRUNC, 0644 );
	write ( fd, ( void* ) Mem, 32768 );
	close ( fd );
}