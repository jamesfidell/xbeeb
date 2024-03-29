/*
 *
 * $Id: Acia.c,v 1.9 2002/01/15 15:46:43 james Exp $
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
 * $Log: Acia.c,v $
 * Revision 1.9  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.8  2000/08/16 17:58:26  james
 * Update copyright message
 *
 * Revision 1.7  1996/10/01 00:32:57  james
 * Created separate hardware reset code for each emulated unit and called
 * these from the main initialisation section of the code to do all of the
 * setup necessary.
 *
 * Revision 1.6  1996/09/30 23:54:37  james
 * Added correct address wrap-around on control registers.
 *
 * Revision 1.5  1996/09/24 23:05:34  james
 * Update copyright dates.
 *
 * Revision 1.4  1996/09/22 21:35:05  james
 * New implementation of the (partial) ACIA emulation.
 *
 * Revision 1.3  1996/09/21 23:07:34  james
 * Call FatalError() rather than exit() so that screen stuff etc. can
 * be cleaned up.
 *
 * Revision 1.2  1996/09/21 22:13:46  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.1  1996/09/21 17:20:35  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#include	<stdio.h>
#include	<unistd.h>

#include "Config.h"
#include "Acia.h"
#include "Beeb.h"


/*
 * The transmit control register bit values...
 */

static byteval		CounterDivide;
static byteval		WordSelect;
static byteval		XmitRTS;
static byteval		XmitIRQEnable = 0;
static byteval		RcvIRQEnable = 0;

/*
 * The ACIA registers
 *
 * FIX ME
 *
 * I don't know that the initialised values of the SR are
 * necessarily correct.
 *
 * I'm working on the assumption that :
 *
 * RDR is not full
 * TDR is not empty
 * DCD and CTS don't matter, but set DCD low, CTS high
 * Framing errors have not occurred
 * Receiver Overrun hasn't occurred
 * Parity Error hasn't occurred
 * IRQ isn't requested
 *
 */

static byteval		CR;
static byteval		SR = SR_CTS;
static byteval		TDR;
static byteval		RDR;

/*
 * Generation of interrupts from the ACIA is problematic to code because
 * it will probably be possible to cause an interrupt straight
 * away (because the data will be available or processed immediately).
 *
 * Hmmm.  That's not too clear, is it ?  What I mean is (I think :-)
 * that whereas the real ACIA would spend some time processing data
 * and then generate an interrupt to inform the OS that it had been done,
 * it's quite probable that in an emulated situation, the data might be
 * available immediately.  Generating an interrupt immediately to deal
 * with this might screw up the OS's idea of timing.
 *
 * Unfortunately, I don't really see any way around this.
 *
 * One of the things I do need to know is whether the any interrupt was
 * generated by the send or receive side of the hardware, because the
 * interrupts can be separately enabled.  Thats what the two variables
 * below are for.
 *
 */ 

static byteval		XmitIRQ = 0;
static byteval		RcvIRQ = 0;


void
ResetAcia ( void )
{
	/*
	 * FIX ME
	 *
	 * I have no idea what happens when the ACIA is powered up/reset.
	 */

	return;
}


byteval
ReadAcia ( int addr )
{
	/*
	 * Because the ACIA only takes up two of the eight addresses assigned
	 * to it in the memory map, the others all wrap around onto those
	 * first two.
	 */

	switch ( addr & 0x1 )
	{
		case 0x0 :
			/*
			 * Make sure the SR is up-to-date
			 */

			AciaSRSet ( 0 );
#ifdef	INFO
			printf ( "ACIA SR read %02x\n", SR );
#endif
			return SR;
			break;

		case 0x1 :
			/*
			 * FIX ME
			 *
			 * There's no data to put in this register yet -- it's just
			 * a dummy for some future emulation.
			 */

#ifdef	INFO
			printf ( "ACIA RDR read\n" );
#endif
			RcvIRQ = 0;
			AciaSRClear ( SR_RDRF | SR_OVRN | SR_PE );
			return RDR;
	}

	/* NOTREACHED */

	/*
	 * FIX ME
	 *
	 * Should return a fatal error here ?
	 */

	return 0xff;
}


void
WriteAcia ( int addr, byteval val )
{
	/*
	 * Again we have to account for the wrap-around of memory addresses
	 * here...
	 */

	switch ( addr & 0x1 )
	{
		case 0x0 :		/* Write control register */
		{
			byteval		tmp;

			/*
			 * FIX ME
			 *
			 * There may be some clean-up work to be done once all these
			 * bits are set up.  I'm not sure yet, but since the ACIA
			 * emulation doesn't exactly do a great deal, best not worry
			 * about it yet...
			 */

			CR = val;
			if (( tmp = CR & 0x3 ) == 0x03 )
			{
				/*
				 * Do a master reset
				 *
				 * FIX ME
				 *
				 * Do PE and FE need to be reset here ?
				 */

				AciaSRClear ( SR_RDRF | SR_OVRN );
			}
			else
				CounterDivide = tmp;

			WordSelect = ( CR >> 2 ) & 0x7;
			XmitRTS = CR & 0x20;
			XmitIRQEnable = CR & 0x40;
			RcvIRQEnable = CR & 0x80;

			/*
			 * Now writing the control register may allow an outstanding
			 * interrupt request to be processed.
			 */

			if (( XmitIRQEnable && XmitIRQ ) || ( RcvIRQEnable && RcvIRQ ))
				IRQ();

#ifdef	INFO
			printf ( "ACIA CR set, Xmit IRQ %sabled, Rcv IRQ %sabled\n",
				XmitIRQEnable ? "en" : "dis", RcvIRQEnable ? "en" : "dis" );
#endif
			break;
		}
		case 0x1 :		/* Transmit Data Register */
			/*
			 * FIX ME
			 *
			 * What happens to this data now  ?  Really it should
			 * be send down whichever serial line we're using to emulate
			 * the serial connection and the appropriate flag set in the
			 * status register.
			 *
			 */

			TDR = val;
			XmitIRQ = 0;
			AciaSRClear ( SR_TDRE );
#ifdef	INFO
			printf ( "value %02x stored in TDR\n", TDR );
#endif
			break;
	}

	return;
}


void
AciaSRClear ( byteval val )
{
	SR &= ( val ^ 0x7f );
	SR |= ( XmitIRQ | RcvIRQ );
	return;
}


void
AciaSRSet ( byteval val )
{
	SR &= 0x7f;
	SR |= ( XmitIRQ | RcvIRQ | val );
	return;
}


int
SaveACIA ( int fd )
{
	byteval		acia [ 8 ];

	/*
	 * Need to update the SR in case we haven't yet caught up
	 * with the Xmit/Rcv IRQ flag settings
	 */

	AciaSRSet ( 0 );

	acia [ 0 ] = CR;
	acia [ 1 ] = TDR;
	acia [ 2 ] = SR;
	acia [ 3 ] = RDR;
	acia [ 4 ] = XmitIRQ;
	acia [ 5 ] = RcvIRQ;

	if ( write ( fd, acia, 8 ) != 8 )
		return -1;

	return 0;
}


int
RestoreACIA ( int fd, unsigned int ver )
{
	byteval		acia [ 8 ];

	if ( ver > 1 )
		return -1;

	if ( read ( fd, acia, 8 ) != 8 )
		return -1;

	CR = acia [ 0 ];
	TDR = acia [ 1 ];
	SR = acia [ 2 ];
	RDR = acia [ 3 ];
	XmitIRQ = acia [ 4 ];
	RcvIRQ = acia [ 5 ];

	CounterDivide = CR & 0x3;
	WordSelect = ( CR >> 2 ) & 0x7;
	XmitRTS = CR & 0x20;
	XmitIRQEnable = CR & 0x40;
	RcvIRQEnable = CR & 0x80;

	return 0;
}
