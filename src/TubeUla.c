/*
 *
 * $Id: TubeUla.c,v 1.6 2002/01/15 15:46:43 james Exp $
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
 * $Log: TubeUla.c,v $
 * Revision 1.6  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.5  2000/08/16 17:58:29  james
 * Update copyright message
 *
 * Revision 1.4  1996/10/01 00:33:05  james
 * Created separate hardware reset code for each emulated unit and called
 * these from the main initialisation section of the code to do all of the
 * setup necessary.
 *
 * Revision 1.3  1996/09/24 23:05:45  james
 * Update copyright dates.
 *
 * Revision 1.2  1996/09/21 22:13:52  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.1  1996/09/21 17:20:42  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#include <stdio.h>

#include "Config.h"
#include "TubeUla.h"

/*
 * FIX ME
 *
 * I don't have any information on the Tube interface, so writing to the
 * interface doesn't do anything, reading returns whatever my Beeb
 * returned what I read the registers.
 */

static byteval			ReadRegs [ 32 ] =
{
	0xae, 0xde, 0xae, 0xee, 0xae, 0xee, 0xae, 0xee,
	0xae, 0xae, 0xae, 0xfe, 0xae, 0xee, 0xae, 0xae,
	0xae, 0xbe, 0xae, 0xfe, 0xae, 0xbe, 0xae, 0xfe,
	0xae, 0xfe, 0xbe, 0xfe, 0xbe, 0xfe, 0xae, 0xae
};


void
ResetTubeUla ( void )
{
	/*
	 * FIX ME
	 *
	 * I don't have any idea what the power-up/reset state of the Tube
	 * ULA should be, so for the moment it's left as set in ReadRegs,
	 * above.
	 */

	return;
}


byteval
ReadTubeUla ( int addr )
{
/*
	fprintf ( stderr, "read from tube ULA (addr = %x)\n", addr );
*/
	return ReadRegs [ addr ];
}


void
WriteTubeUla ( int addr, byteval val )
{
/*
	fprintf ( stderr, "write to tube ULA (addr = %x)\n", addr );
*/
	return;
}


int
SaveTubeUla ( int fd )
{
	/*
	 * FIX ME
	 */

	return 0;
}


int
RestoreTubeUla ( int fd, unsigned int ver )
{
	/*
	 * FIX ME
	 */

	return 0;
}
