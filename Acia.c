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


#include	<stdio.h>
#include	<unistd.h>

#include "Config.h"
#include "Acia.h"

static unsigned char		CounterDivide;
static unsigned char		WordSelect;
static unsigned char		XmitControl;
static unsigned char		RcvIntEnable;

static unsigned char		WCR;
static unsigned char		SR;
static unsigned char		TDR;
static unsigned char		RDR;

void						AciaSRclear ( unsigned char );


unsigned char
ReadAcia ( int addr )
{
	switch ( addr )
	{
		case 0x0 :
#ifdef	INFO
			printf ( "ACIA SR read\n" );
#endif
			return SR;
			break;

		case 0x1 :
			/*
			 * FIX ME
			 *
			 * Don't know where this data is supposed to be coming
			 * from yet.
			 */

#ifdef	INFO
			printf ( "ACIA RDR read\n" );
#endif
			return RDR;

		default :
			fprintf ( stderr, "Illegal read from ACIA (addr = %x)\n", addr );
			exit ( 1 );
			break;
	}
}


void
WriteAcia ( int addr, unsigned char val )
{
	switch ( addr )
	{
		case 0x0 :		/* Write control register */
			/*
			 * FIX ME
			 *
			 * Anything else that has to be done once these are set ?
			 */

			WCR = val;
			CounterDivide = WCR & 0x3;
			WordSelect = ( WCR >> 2 ) & 0x7;
			XmitControl = ( WCR >> 5 ) & 0x3;
			RcvIntEnable = WCR & 0x80;
#ifdef	INFO
			printf ( "ACIA CR set, interrupts %sabled\n",
							RcvIntEnable ? "en" : "dis" );
#endif
			break;

		case 0x1 :		/* Transmit Data Register */
			/*
			 * FIX ME
			 *
			 * What happens to this data now  ?  Really it should
			 * be send down whichever serial line we're using to emulate
			 * the serial connection and the appropriate flag set in the
			 * status register.
			 *
			 * I guess the TDR empty flag in the SR should be cleared
			 * when this register is written.
			 */

			TDR = val;
			AciaSRclear ( 0x2 );
			fprintf ( stderr, "Write to ACIA TDR not implemented\n" );
			break;

		default :
			fprintf ( stderr, "Illegal write ACIA (addr = %x)\n", addr );
			exit ( 0 );
	}

	return;
}


void
AciaSRclear ( unsigned char val )
{
	SR &= ( val ^ 0x7f );
	return;
}


int
SaveACIA ( int fd )
{
	unsigned char	acia [ 8 ];

	acia [ 0 ] = WCR;
	acia [ 1 ] = TDR;
	acia [ 2 ] = SR;
	acia [ 3 ] = RDR;

	if ( write ( fd, acia, 8 ) != 8 )
		return -1;

	return 0;
}


int
RestoreACIA ( int fd, unsigned int ver )
{
	unsigned char	acia [ 8 ];

	if ( ver > 1 )
		return -1;

	if ( read ( fd, acia, 8 ) != 8 )
		return -1;

	WCR = acia [ 0 ];
	TDR = acia [ 1 ];
	SR = acia [ 2 ];
	RDR = acia [ 3 ];

	CounterDivide = WCR & 0x3;
	WordSelect = ( WCR >> 2 ) & 0x7;
	XmitControl = ( WCR >> 5 ) & 0x3;
	RcvIntEnable = WCR & 0x80;

	return 0;
}
