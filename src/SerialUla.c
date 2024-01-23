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

#include "Config.h"
#include "SerialUla.h"
#include "Acia.h"

static	unsigned char		MotorLED;
static	unsigned char		RS423;
static	unsigned char		XmitRate;
static	unsigned char		RcvRate;

static	unsigned char		Register0;

byteval
ReadSerialUla ( int addr )
{
	/*
	 * FIX ME
	 */

	fprintf ( stderr, "Attempt to read serial ULA (addr = %x)\n", addr );
	fprintf ( stderr, "Not yet implemented\n" );
	FatalError();
	return 0;
}


void
WriteSerialUla ( int addr, byteval val )
{
	if ( addr == 0 )
	{
		Register0 = val;
		XmitRate = Register0 & 0x7;
		RcvRate = ( Register0 >> 3 ) & 0x7;
		if ( RS423 = ( Register0 & 0x40 ))
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
		MotorLED = Register0 & 0x80;
#ifdef	INFO
		printf ( "Serial ULA CR set, RS423 %sabled, motor o%s\n",
					RS423 ? "en" : "dis", MotorLED ? "n" : "ff" );
#endif
	}
	else
	{
		fprintf ( stderr, "illegal write serial ULA (addr = %x)\n", addr );
		FatalError();
	}
	return;
}


int
SaveSerialUla ( int fd )
{
	byteval			serial [ 8 ];

	serial [ 0 ] = Register0;

	if ( write ( fd, serial, 8 ) != 8 )
		return -1;

	return 0;
}


int
RestoreSerialUla ( int fd, unsigned int ver )
{
	byteval			serial [ 8 ];

	if ( ver > 1 )
		return -1;

	if ( read ( fd, serial, 8 ) != 8 )
		return -1;

	Register0 = serial [ 0 ];

	XmitRate = Register0 & 0x7;
	RcvRate = ( Register0 >> 3 ) & 0x7;
	RS423 = Register0 & 0x40;
	MotorLED = Register0 & 0x80;

	return 0;
}
