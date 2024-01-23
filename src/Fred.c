/*
 *
 * $Id: Fred.c,v 1.5 1996/09/24 23:05:38 james Exp $
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
 * $Log: Fred.c,v $
 * Revision 1.5  1996/09/24 23:05:38  james
 * Update copyright dates.
 *
 * Revision 1.4  1996/09/22 21:10:28  james
 * Include Beeb.h to get prototype for FatalError()
 *
 * Revision 1.3  1996/09/22 20:36:13  james
 * More conversions of exit(x) to FatalError()
 *
 * Revision 1.2  1996/09/21 22:13:48  james
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


#include <stdio.h>
#include <unistd.h>

#include "Config.h"
#include "Fred.h"
#include "Beeb.h"

byteval
ReadFred ( int addr )
{
	/*
	 * FIX ME
	 */

	fprintf ( stderr, "Attempt to read Fred (addr = %x)\n", addr );
	fprintf ( stderr, "Not yet implemented\n" );
	FatalError();
	return 0;
}

void
WriteFred ( int addr, byteval val )
{
	/*
	 * FIX ME
	 */

	fprintf ( stderr, "Attempt to write Fred (addr = %x)\n", addr );
	fprintf ( stderr, "Not yet implemented\n" );
	FatalError();
	return;
}


int
SaveFred ( int fd )
{
	/*
	 * FIX ME
	 */

	return 0;
}


int
RestoreFred ( int fd, unsigned int ver )
{
	/*
	 * FIX ME
	 */

	return 0;
}
