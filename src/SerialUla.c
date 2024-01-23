/*
 *
 * $Id: SerialUla.c,v 1.12 2002/01/15 15:46:43 james Exp $
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
 * $Log: SerialUla.c,v $
 * Revision 1.12  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.11  2000/08/16 17:58:28  james
 * Update copyright message
 *
 * Revision 1.10  1996/10/13 17:24:36  james
 * Renamed Register0 to SerRegister0 to valid confusion with the Video ULA
 * Register 0.
 *
 * Revision 1.9  1996/10/08 00:04:34  james
 * Added InfoWindow to show LED status.  Also required addition of the
 * SHIFTLOCK_SOUND_HACK to prevent the Shift Lock LED being light up
 * whenever the sound buffer is full, which means that far too much
 * time can be spent re-drawing the LED.
 *
 * Revision 1.8  1996/10/01 00:33:02  james
 * Created separate hardware reset code for each emulated unit and called
 * these from the main initialisation section of the code to do all of the
 * setup necessary.
 *
 * Revision 1.7  1996/10/01 00:00:17  james
 * Corrected address wrap-around for control registers.
 *
 * Revision 1.6  1996/09/24 23:05:43  james
 * Update copyright dates.
 *
 * Revision 1.5  1996/09/22 21:35:05  james
 * New implementation of the (partial) ACIA emulation.
 *
 * Revision 1.4  1996/09/22 21:10:28  james
 * Include Beeb.h to get prototype for FatalError()
 *
 * Revision 1.3  1996/09/22 20:36:13  james
 * More conversions of exit(x) to FatalError()
 *
 * Revision 1.2  1996/09/21 22:13:50  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.1  1996/09/21 17:20:40  james
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
#include "SerialUla.h"
#include "Beeb.h"
#include "Acia.h"
#include "InfoWindow.h"


unsigned char				MotorLED = 0;
static	unsigned char		RS423;
static	unsigned char		XmitRate;
static	unsigned char		RcvRate;

static	unsigned char		SerRegister0;


void
ResetSerialUla ( void )
{
	/*
	 * FIX ME
	 *
	 * Haven't a clue what happens on power-up/reset
	 */

	return;
}


byteval
ReadSerialUla ( int addr )
{
	/*
	 * There's only one memory location involved here, so all others in the
	 * memory map will wrap onto that one.
	 */

	/*
	 * FIX ME
	 *
	 * I don't think it's quite this simple.  Reading the register always
	 * appears to return zero, but in the tests I've done, it also turns
	 * the cassette motor off if it's currently on.  Perhaps it sets the
	 * control register to the value just read ?
	 *
	 * I know for a fact that reading this location WILL turn off the
	 * cassette motor.
	 */

	return 0;
}


void
WriteSerialUla ( int addr, byteval val )
{
	byteval		oldMotorLED = MotorLED;

	/*
	 * Again, we can actually ignore the address we're writing to, because
	 * all the memory mapped addresses map to the same register.
	 */

	SerRegister0 = val;
	XmitRate = SerRegister0 & 0x7;
	RcvRate = ( SerRegister0 >> 3 ) & 0x7;
	if (( RS423 = ( SerRegister0 & 0x40 )))
	{
		/*
		 * Have to clear the DCD bit in the ACIA status register
		 * when the RS423 port is enabled
		 */

		AciaSRClear ( SR_DCD );
	}
	else
	{
		/*
		 * Have to take ACIA SR CTS low when the cassette is active,
		 * but to fake the 2400Hz carrier for the cassette system,
		 * we'll also raise DCD.
		 */

		AciaSRClear ( SR_CTS );
		AciaSRSet ( SR_DCD );
	}

	if (( MotorLED = SerRegister0 & 0x80 ) != oldMotorLED )
		DrawMotorLED();

#ifdef	INFO
	printf ( "Serial ULA CR set, RS423 %sabled, motor o%s\n",
				RS423 ? "en" : "dis", MotorLED ? "n" : "ff" );
#endif
	return;
}


int
SaveSerialUla ( int fd )
{
	byteval					serial [ 8 ];

	serial [ 0 ] = SerRegister0;

	if ( write ( fd, serial, 8 ) != 8 )
		return -1;

	return 0;
}


int
RestoreSerialUla ( int fd, unsigned int ver )
{
	byteval					serial [ 8 ];

	if ( ver > 1 )
		return -1;

	if ( read ( fd, serial, 8 ) != 8 )
		return -1;

	SerRegister0 = serial [ 0 ];

	XmitRate = SerRegister0 & 0x7;
	RcvRate = ( SerRegister0 >> 3 ) & 0x7;
	RS423 = SerRegister0 & 0x40;
	MotorLED = SerRegister0 & 0x80;

	return 0;
}
