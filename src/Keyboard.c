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
#include <unistd.h>
#include <memory.h>

#include "Config.h"
#include "Keyboard.h"
#include "SystemVia.h"

static	unsigned char		CapsLockLED;
static	unsigned char		ShiftLockLED;


unsigned char				Keys [ 80 ];


void
KeyboardWrite ( byteval ddr, byteval *ora )
{
	/*
	 * The keyboard is polled as an 8x10 array, using the system VIA
	 * bits PA0-PA3 as one index and PA4-PA6 as the other.  PA7
	 * is an input that reflects the state of the indexed key.
	 * 
	 * Polling takes place when the write enable value is low.
	 * When write enable goes high, I don't care what happens.
	 *
	 */

	unsigned char		vert, horiz, idx, op;

	vert = ddr & *ora & 0x0f;
	horiz = ( ddr & *ora & 0x70 ) >> 4;

	idx = horiz * 10 + vert;
	op = Keys [ idx ] ? 0x80 : 0x0;
#ifdef	INFO
	printf ( "Checking key index 0x%x. val = %d\n", idx, op );
#endif

	/*
	 * The new value of the ORA is the old outputs combined with
	 * whatever keys were found to be pressed now and are allowed
	 * to be inputs.
	 */

		*ora = ( *ora & ddr ) | ( op & ~ddr );

	/*
	 * In addition to changing the values in IRA, an interrupt will
	 * also be caused if a key on this column of the keyboard scan
	 * matrix is pressed.
	 */

	op = 0;
	for ( idx = vert; idx < 80; idx += 10 )
		op += Keys [ idx ];

	if ( op )
		SystemViaSetInterrupt ( INT_CA2 );

	return;
}


void
LedSetCapsLock ( byteval val )
{
	/*
	 * FIX ME
	 *
	 * It would be nice to display this on the screen somewhere...
	 */

	CapsLockLED = val ? 0 : 1;
#ifdef	INFO
	printf ( "CAPS LOCK is %s\n", CapsLockLED ? "ON" : "OFF" );
#endif
	return;
}


void
LedSetShiftLock ( unsigned char val )
{
	/*
	 * FIX ME
	 *
	 * It would be nice to display this on the screen somewhere...
	 */

	ShiftLockLED = val ? 0 : 1;
#ifdef	INFO
	printf ( "SHIFT LOCK is %s\n", ShiftLockLED ? "ON" : "OFF" );
#endif
	return;
}


void
InitialiseKeyboard()
{
	int			i;

	for ( i = 0; i < 80; i++ )
	{
		Keys [ i ] = 0;
	}

	/*
	 * FIX ME
	 *
	 * Set up the keyboard DIP switches...
	 *
	 */

	return;
}


void
KeyboardMatrixUpdate ( unsigned char key, signed char action )
{
	/*
	 * Mark the key array to show that the key has been pressed
	 */

	Keys [ key ] += action;

	/*
	 * FIX ME
	 *
	 * The PCR also controls how CA2 works -- this should be checked to
	 * see that we're handling it correctly.
	 */

	/*
	 * At this point CA2 generates an interrupt for all keys not on row
	 * zero of the matrix.  We do so as long as the contents of the
     * IER allow.
	 *
	 * Note that we don't need to clear the interrupt when the key is
	 * released, because the interrupt is triggered by CA2 going
	 * low only, not when it goes high.  The interrupt is eventually
	 * cleared by the OS.
	 *
	 */

#ifdef	INFO
		printf ( "Key %d %s\n", key, action == KEY_PRESSED ? "press" : "rel" );
#endif

	if ( action == KEY_PRESSED && key > 9 )
		SystemViaSetInterrupt ( INT_CA2 );

	return;
}


int
SaveKeyboard ( int fd )
{
	unsigned char		kbd [ 96 ];

	memcpy ( kbd, Keys, 80 );
	kbd [ 88 ] = CapsLockLED;
	kbd [ 89 ] = ShiftLockLED;

	if ( write ( fd, kbd, 96 ) != 96 )
		return -1;

	return 0;
}


int
RestoreKeyboard ( int fd, unsigned int val )
{
	unsigned char		kbd [ 96 ];

	if ( val > 1 )
		return -1;

	if ( read ( fd, kbd, 96 ) != 96 )
		return -1;

	memcpy ( Keys, kbd, 80 );

	CapsLockLED = kbd [ 88 ];
	ShiftLockLED = kbd [ 89 ];

	return 0;
}
