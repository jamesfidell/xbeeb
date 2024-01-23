/*
 *
 * $Id: pch.c,v 1.4 1996/10/10 22:15:14 james Exp $
 *
 * Copyright (c) James Fidell 1995, 1996.
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
 * $Log: pch.c,v $
 * Revision 1.4  1996/10/10 22:15:14  james
 * Oops!  Got the traps the wrong way around.
 *
 * Revision 1.3  1996/09/24 23:05:31  james
 * Update copyright dates.
 *
 * Revision 1.2  1996/09/24 22:39:46  james
 * Massive overhaul of instruction decoding code.  Includes :
 *
 *   Changed the EFS dummy opcodes because of a clash with the undocumented
 *   NMOS 6502 DCP instructions.  The new trap values are now codes that would
 *   normally halt the CPU.
 *
 * Revision 1.1  1996/09/21 18:46:28  james
 * Added pch.c for patching the OS1.2 ROM
 *
 */


#include <stdio.h>
#include <fcntl.h>

main()
{
	unsigned char	x [ 16384 ];
	int				fd;

	fd = open ( "OS1.2p1.rom", O_RDWR );
	read ( fd, x, 16384 );

	/*
	 * The OSFILE trap
	 */

	x [ 0x327d ] = 0x22;

	/*
	 * The OSFSC trap
	 */

	x [ 0x31b1 ] = 0x02;

	lseek ( fd, 0, SEEK_SET );
	write ( fd, x, 16384 );
	close ( fd );
}
