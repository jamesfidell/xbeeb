/*
 *
 * $Id: Display.c,v 1.4 1996/10/09 20:49:18 james Exp $
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
 * $Log: Display.c,v $
 * Revision 1.4  1996/10/09 20:49:18  james
 * Display register values with leading zeros.
 *
 * Revision 1.3  1996/09/24 23:05:37  james
 * Update copyright dates.
 *
 * Revision 1.2  1996/09/21 22:54:49  james
 * Fix code for displaying registers (because the variables are no longer
 * globals)
 *
 * Revision 1.1  1996/09/21 17:20:36  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#include <stdio.h>

#include "Config.h"
#include "6502.h"

void
DisplayRegisters ( byteval A, byteval X, byteval Y, int PC, byteval SP )
{
	printf ( "A  = 0x%02x\n", A );
	printf ( "X  = 0x%02x\n", X );
	printf ( "Y  = 0x%02x\n", Y );
	printf ( "PC = 0x%04x\n", PC );
	printf ( "SP = 0x%02x\n", SP );

	printf ( "ST = " );
	printf ( "%c", NegativeFlag ? 'N' : 'n' );
	printf ( "%c", OverflowFlag ? 'V' : 'v' );
	printf ( "-B" );
	printf ( "%c", DecimalModeFlag ? 'D' : 'd' );
	printf ( "%c", IRQDisableFlag ? 'I' : 'i' );
	printf ( "%c", ZeroFlag ? 'Z' : 'z' );
	printf ( "%c", CarryFlag ? 'C' : 'c' );
	printf ( "\n" );
}
