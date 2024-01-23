/*
 *
 * $Id: Disk.c,v 1.9 2002/01/15 15:46:43 james Exp $
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
 * $Log: Disk.c,v $
 * Revision 1.9  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.8  2000/08/16 17:58:27  james
 * Update copyright message
 *
 * Revision 1.7  1996/10/01 00:33:00  james
 * Created separate hardware reset code for each emulated unit and called
 * these from the main initialisation section of the code to do all of the
 * setup necessary.
 *
 * Revision 1.6  1996/10/01 00:02:05  james
 * Set up dummy accesses to FDC and handled address wrap-around for FDC
 * addresses.
 *
 * Revision 1.5  1996/09/24 23:05:36  james
 * Update copyright dates.
 *
 * Revision 1.4  1996/09/22 21:10:27  james
 * Include Beeb.h to get prototype for FatalError()
 *
 * Revision 1.3  1996/09/22 20:36:12  james
 * More conversions of exit(x) to FatalError()
 *
 * Revision 1.2  1996/09/21 22:13:47  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.1  1996/09/21 18:32:59  james
 * Renamed Floppy.[ch] to Disk.[ch]
 *
 * Revision 1.1  1996/09/21 18:28:49  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#include	<stdio.h>

#include	"Config.h"
#include	"Disk.h"
#include	"Beeb.h"


void
ResetDiskController ( void )
{
	/*
	 * FIX ME
	 *
	 * When I get around to implementing this one,
	 * From the advanced disk user guide, after a reset
	 *    the status register is zero
	 *    all output signals to the disk drive are forced low
	 *    the 8271 goes idle
	 *    the head is unloaded
	 */

	return;
}


byteval
ReadDiskController ( int addr )
{
	/*
	 * Handle address wrap-around...
	 */

	switch ( addr & 0x7 )
	{
		case 0x0 :
		case 0x1 :
		case 0x2 :
		case 0x3 :
		case 0x4 :
			/*
			 * FIX ME
			 *
			 * These are legal reads, but just aren't implemented yet...
			 */

#ifdef	WARNINGS
			fprintf ( stderr, "WARNING: read of FDC not yet implemented " );
			fprintf ( stderr, "address = %02x\n", addr );
#endif
			return 0;
			break;

		default :
			/*
			 * FIX ME
			 *
			 * These are illegal addresses, so lacking other information
			 * we'll return zero.
			 */

			fprintf ( stderr, "WARNING: out of range read on FDC " );
			fprintf ( stderr, "address = %02x\n", addr );
			return 0;
			break;
	}
}


void
WriteDiskController ( int addr, byteval val )
{
	/*
	 * Handle address wrap-around...
	 */

	switch ( addr & 0x7 )
	{
		case 0x0 :
		case 0x1 :
		case 0x2 :
		case 0x3 :
		case 0x4 :
			/*
			 * FIX ME
			 *
			 * These are legal writes, but just aren't implemented yet...
			 */

#ifdef	WARNINGS
			fprintf ( stderr, "WARNING: write of FDC not yet implemented " );
			fprintf ( stderr, "address = %02x\n", addr );
#endif
			break;

		default :
			/*
			 * FIX ME
			 *
			 * These are illegal addresses, so lacking other information
			 * we'll just say that something nasty happened
			 */

			fprintf ( stderr, "WARNING: out of range write to FDC " );
			fprintf ( stderr, "address = %02x\n", addr );
			break;
	}
}


int
SaveDisk ( int fd )
{
	/*
	 * FIX ME
	 */

	return 0;
}


int
RestoreDisk ( int fd, unsigned int ver )
{
	/*
	 * FIX ME
	 */

	return 0;
}
