/*
 *
 * $Id: Econet.c,v 1.7 1996/10/01 00:33:01 james Exp $
 *
 * Copyright (c) James Fidell 1994, 1995, 1996.
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
 * $Log: Econet.c,v $
 * Revision 1.7  1996/10/01 00:33:01  james
 * Created separate hardware reset code for each emulated unit and called
 * these from the main initialisation section of the code to do all of the
 * setup necessary.
 *
 * Revision 1.6  1996/10/01 00:10:29  james
 * Added address wrap-around for control registers.
 *
 * Revision 1.5  1996/09/24 23:05:38  james
 * Update copyright dates.
 *
 * Revision 1.4  1996/09/22 21:10:28  james
 * Include Beeb.h to get prototype for FatalError()
 *
 * Revision 1.3  1996/09/22 20:36:12  james
 * More conversions of exit(x) to FatalError()
 *
 * Revision 1.2  1996/09/21 22:13:47  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.1  1996/09/21 17:20:37  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#include	<stdio.h>

#include	"Config.h"
#include	"Econet.h"
#include	"Beeb.h"


void
ResetEconetController ( void )
{
	/*
	 * FIX ME
	 *
	 * Haven't a clue what happens on power-up/reset
	 */

	return;
}


byteval
ReadEconetController ( int addr )
{
	/*
	 * Handle address wrap-around...
	 */

	switch ( addr & 0x3 )
	{
		case 0x0 :
		case 0x1 :
		case 0x2 :
		case 0x3 :
			/*
			 * FIX ME
			 *
			 * These are legal reads, but just aren't implemented yet...
			 */

#ifdef	WARNINGS
			fprintf ( stderr, "WARNING: read of 68B54 not yet implemented " );
			fprintf ( stderr, "address = %02x\n", addr );
#endif
			return 0;
			break;
	}

	/* NOTREACHED */

	/*
	 * FIX ME
	 *
	 * Should return a fatal error here.
	 */

	return 0xff;
}


void
WriteEconetController ( int addr, byteval val )
{
	/*
	 * Handle address wrap-around...
	 */

	switch ( addr & 0x3 )
	{
		case 0x0 :
		case 0x1 :
		case 0x2 :
		case 0x3 :
			/*
			 * FIX ME
			 *
			 * These are legal writes, but just aren't implemented yet...
			 */

#ifdef	WARNINGS
			fprintf ( stderr, "WARNING: write of 68B54 not yet implemented " );
			fprintf ( stderr, "address = %02x\n", addr );
#endif
			break;
	}
}


int
SaveEconet ( int fd )
{
	/*
	 * FIX ME
	 */

	return 0;
}


int
RestoreEconet ( int fd, unsigned int ver )
{
	/*
	 * FIX ME
	 */

	return 0;
}
