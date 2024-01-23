/*
 *
 * $Id: Keyboard.c,v 1.13 2002/01/15 15:46:43 james Exp $
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
 * $Log: Keyboard.c,v $
 * Revision 1.13  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.12  2002/01/13 23:59:12  james
 * Fixed coredump on snapshot
 *
 * Revision 1.11  2002/01/13 23:40:37  james
 * More keyboard-handling changes from Russell Marks
 *
 * Revision 1.10  2000/08/16 17:58:27  james
 * Update copyright message
 *
 * Revision 1.9  1996/12/01 21:26:24  james
 * Improved keyboard handling from DRS.
 *
 * Revision 1.8  1996/11/15 00:03:39  james
 * Wrap vertical keyboard scan index when it reaches 9 (it's a 4-bit value).
 * (Fix from David Ralph Stacey).
 *
 * Revision 1.7  1996/10/09 22:09:42  james
 * Corrected setting of CA2 when a key is pressed.
 *
 * Revision 1.6  1996/10/08 00:04:31  james
 * Added InfoWindow to show LED status.  Also required addition of the
 * SHIFTLOCK_SOUND_HACK to prevent the Shift Lock LED being light up
 * whenever the sound buffer is full, which means that far too much
 * time can be spent re-drawing the LED.
 *
 * Revision 1.5  1996/09/30 23:39:33  james
 * Split out option processing into Options.[ch].  Updated the help message,
 * added support for the Model A using the -a switch (and added the
 * MODEL_B_ONLY #define in Config.h, added the -m and -s switches to set the
 * initial screen mode and keyboard DIP switches.
 *
 * Revision 1.4  1996/09/25 19:19:57  james
 * Major overhaul of VIA emulation code :
 *
 *   Enabled toggling of PB7 in system VIA depending on ACR bit 6 and the
 *   one-shot/free-run mode of T1
 *
 *   Implemented User VIA T1 free-running mode.  Set the initial value of
 *   the User VIA ORA to 0x80.  Planetoid/Defender now works for the first
 *   time!
 *
 *   Corrected value returned by read from VIA T2CL and T2CH.  Frak! now
 *   works.
 *
 *   Set up dummy return for reads from the system VIA IRA and SR.
 *
 *   Implemented address wrap-around for memory-mapped registers in the VIA.
 *
 *   Set up dummy return for reads from the user VIA SR.
 *
 *   Implemented address wrap-around for memory-mapped registers in the VIA.
 *
 *   Updated 6522 VIA emulation to have correct initial values for VIA
 *   registers wherever possible.
 *
 *   Heavily modified 6522 VIA code to separate out the input/output
 *   registers and what is actually on the data pins.  This has the benefits
 *   of tidying up the whole VIA i/o emulation and not requiring any nasty
 *   configuration hacks to get software to work (apart from those that exist
 *   because of uncompleted emulation).
 *
 *   Tidied up 6522Via interrupt handling code.
 *
 * Revision 1.3  1996/09/24 23:05:39  james
 * Update copyright dates.
 *
 * Revision 1.2  1996/09/21 22:13:48  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.1  1996/09/21 17:20:38  james
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
#include "Keyboard.h"
#include "SystemVia.h"
#include "InfoWindow.h"

unsigned char				CapsLockLED = 0;
unsigned char				ShiftLockLED = 0;
#ifdef	SHIFTLOCK_SOUND_HACK
unsigned char				LockKeysChanged = 0;
#endif


#define	MAX_KEYS			128
unsigned char				Keys [ MAX_KEYS ];
unsigned long				DIPSwitches = DIP_SWITCHES;


byteval
KeyboardWrite ( byteval idx )
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

	unsigned char		vert, op, result;

	vert = idx & 0xf;
	result = Keys [ idx ] ? 0x80 : 0x0;

#ifdef	SHIFTLOCK_SOUND_HACK
	if ( result && ( idx == KEY_CAPSLOCK || idx == KEY_SHIFTLOCK ))
		LockKeysChanged = 1;
#endif	/* SHIFTLOCK_SOUND_HACK */

#ifdef	INFO
	printf ( "Checking key index 0x%x. val = %d\n", idx, op );
#endif

	/*
	 * An interrupt will be caused if a key on this column of the
	 * keyboard scan matrix is pressed.
	 */

	op = 0;
	for ( idx = vert; idx < MAX_KEYS; idx += 0x10 )
		op += Keys [ idx ];

	if ( op )
		SystemViaSetCA2 ( 1 );

	/*
	 * The new value of on the input pins is the old outputs combined with
	 * whatever keys were found to be pressed now and are allowed
	 * to be inputs.
	 */

	return result;
}


void
LedSetCapsLock ( byteval val )
{
	byteval		oldCapsLockLED = CapsLockLED;

	if (( CapsLockLED = val ? 0 : 1 ) != oldCapsLockLED )
		DrawCapsLockLED();
#ifdef	INFO
	printf ( "CAPS LOCK is %s\n", CapsLockLED ? "ON" : "OFF" );
#endif
	return;
}


void
LedSetShiftLock ( unsigned char val )
{
	byteval		oldShiftLockLED = ShiftLockLED;

	if (( ShiftLockLED = val ? 0 : 1 ) != oldShiftLockLED )
		DrawShiftLockLED();
#ifdef	INFO
	printf ( "SHIFT LOCK is %s\n", ShiftLockLED ? "ON" : "OFF" );
#endif
	return;
}


void
InitialiseKeyboard()
{
	int				i;
	unsigned long	d = DIPSwitches;

	for ( i = 0; i < MAX_KEYS; i++ )
	{
		Keys [ i ] = 0;
	}

	for ( i = SW2_0; i >= SW2_7; i-- )
	{
		Keys [ i ] = d & 0x01;
		d >>= 1;
	}
	return;
}


void
KeyboardMatrixUpdate ( unsigned char key, signed char action )
{
	/*
	 * Mark the key array to show that the key has been pressed
	 */

	Keys [ key ] = action;

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

	if ( key > 9 )
		SystemViaSetCA2 ( action == KEY_PRESSED ? 1 : 0 );

	return;
}


int
SaveKeyboard ( int fd )
{
	unsigned char		kbd [ MAX_KEYS + 16 ];

	memcpy ( kbd, Keys, MAX_KEYS );
	kbd [ MAX_KEYS + 1 ] = CapsLockLED;
	kbd [ MAX_KEYS + 2 ] = ShiftLockLED;

	if ( write ( fd, kbd, MAX_KEYS + 16 ) != ( MAX_KEYS + 16 ))
		return -1;

	return 0;
}


int
RestoreKeyboard ( int fd, unsigned int val )
{
	unsigned char		kbd [ MAX_KEYS + 16 ];

	if ( val > 1 )
		return -1;

	if ( read ( fd, kbd, MAX_KEYS + 16 ) != ( MAX_KEYS + 16 ))
		return -1;

	memcpy ( Keys, kbd, MAX_KEYS );

	CapsLockLED = kbd [ MAX_KEYS + 1 ];
	ShiftLockLED = kbd [ MAX_KEYS + 2 ];

	return 0;
}
