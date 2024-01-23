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


#include <stdio.h>
#include <unistd.h>

#include "Config.h"
#include "Fred.h"

unsigned char
ReadFred ( int addr )
{
	/*
	 * FIX ME
	 */

	fprintf ( stderr, "Attempt to read Fred (addr = %x)\n", addr );
	fprintf ( stderr, "Not yet implemented\n" );
	exit ( 1 );
	return 0;
}

void
WriteFred ( int addr, unsigned char val )
{
	/*
	 * FIX ME
	 */

	fprintf ( stderr, "Attempt to write Fred (addr = %x)\n", addr );
	fprintf ( stderr, "Not yet implemented\n" );
	exit ( 1 );
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
