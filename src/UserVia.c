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
#include "6502.h"
#include "Beeb.h"
#include "UserVia.h"
#include "6522Via.h"

/*
 * FIX ME
 *
 * There ought to be a power-on state for all of the registers, but I
 * haven't got a clue what it is, so I write &00 to all of them, except
 * DDRB, which needs to be 0xff or PB7 doesn't pulse.
 */

Via		UserVia =
{
	0x00, 0x00, 0xff, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};


int					UserViaTimer1 = 0xffff;
int					UserViaTimer2 = 0xffff;

/*
 * FIX ME
 *
 * I'm not sure that Timer2InterruptEnable is needed.  In my docs. it
 * states that the T2 interrupt can only be re-enabled once triggered
 * by writing to T2CH or reading from T2CL.  It doesn't say if this is
 * because doing so resets the interrupt flag, or whether they act
 * independently.  I've assumed that they act independently because it
 * seems safer to do so.
 */

unsigned char		UserViaTimer2InterruptEnable = 0;
unsigned char		UserViaTimer1Continuous = 1;
unsigned char		UserViaTimer1SetPB7 = 0;
unsigned char		UserViaTimer2PulseCount = 0;

static void			ClearPortAInterrupts ( void );
static void			ClearPortBInterrupts ( void );


byteval
ReadUserVia ( int addr )
{
	switch ( addr )
	{
		case IRB :
			/*
			 * Calls ClearPortBInterrupts to clear the relevant flags
			 * in the interrupt flag register.
			 *
			 * POSSIBLE ENHANCEMENT ?
			 *
			 * The contents of this register should be correct -- however,
			 * it's necessary for any hardware emulation using this
			 * port to set any input values depending on the setting of
			 * DDRB.
			 *
			 */

#ifdef	INFO
			printf ( "read user VIA IRB, 0x%02x\n", UserVia [ IRB ] );
#endif
			ClearPortBInterrupts();
			return ( UserVia [ IRB ] );
			break;

		case IRA :
			/*
			 * POSSIBLE ENHANCEMENT ?
			 *
			 * Any input values here will have come from the
			 * hardware emulation, set at the last active transition
			 * of CA1.  In fact, this is the printer port, so I doubt
			 * there's much of interest going on here anyway...
			 */

#ifdef	INFO
			printf ( "read user VIA IRA, %2x\n", UserVia [ IRA ] );
#endif
			ClearPortAInterrupts();
			return ( UserVia [ ORA ] );
			break;

		/*
		 * FIX ME
		 *
		 * I assume that reading DDRA and DDRB just returns the current
		 * settings...
		 *
		 */

		case DDRB :
			return ( UserVia [ DDRB ] );
			break;

		case DDRA :
			return ( UserVia [ DDRA ] );
			break;

		case T1CL :
			UserViaClearInterrupt ( INT_T1 );
			return UserViaTimer1 & 0xff;
			break;

		case T1CH :
			return UserViaTimer1 >> 8;
			break;

		case T1LL :
			return UserVia [ T1LL ];
			break;

		case T1LH :
			return UserVia [ T1LH ];
			break;

		case T2CL :
#ifdef	INFO
			printf ( "read user VIA T2CL = 0x%2x\n", UserVia [ T2CL ] );
#endif
			UserViaTimer2InterruptEnable = 1;
			return ( UserVia [ T2CL ] );
			break;

		case T2CH :
#ifdef	INFO
			printf ( "read user VIA T2CH = 0x%2x\n", UserVia [ T2CH ] );
#endif
			return ( UserVia [ T2CH ] );
			break;

		case SR :
			/*
			 * FIX ME
			 */

			fprintf ( stderr, "read from user VIA SR\n" );
			fprintf ( stderr, "Not yet implemented\n\n" );
			ViaDump ( UserVia );
			FatalError();
			break;

		case PCR :
			/*
			 * FIX ME
			 *
			 * I assume that reading this register just returns the
			 * value written.
			 */

			return UserVia [ PCR ];
			break;

		case ACR :
			/*
			 * FIX ME
			 *
			 * I assume that reading this register just returns the
			 * value written.
			 */

			return UserVia [ ACR ];
			break;

		case IER :
			/*
			 * On reads bit 7 of the IER is always read as high.
			 */
#ifdef	INFO
			printf ( "read user VIA IER = %2x\n", UserVia [ IER ] | 0x80 );
#endif
			return ( UserVia [ IER ] | 0x80 );
			break;

		case IFR :
#ifdef	INFO
			printf ( "read user VIA IFR = %2x\n", UserVia [ IFR ] );
#endif
			return ( UserVia [ IFR ] );
			break;

		case IRA_nh :
			/*
			 * FIX ME
			 *
			 * Read/Write to ORA is supposed to clear the IRQ flag in the
			 * IFR.  I don't think read/write to ORA_nh/IRA_nh is supposed
			 * to do the same thing, but I'm not sure.
			 *
			 */

#ifdef	INFO
			printf ( "read user VIA ORA(nh) = %2x\n", UserVia [ ORA_nh ] );
#endif
			return ( UserVia [ ORA_nh ] );
			break;

		default :
			fprintf ( stderr, "out of range " );
			fprintf ( stderr, "read from user VIA (addr = %x)\n", addr );
			FatalError();
	}
	return 0;
}


void
WriteUserVia ( int addr, byteval val )
{
	switch ( addr )
	{
		case ORB :
		{
			/*
			 * Only change the lines which are valid outputs...
			 */

			byteval		old, new, ddr_in, ddr_out;

			old = UserVia [ ORB ];
			ddr_in = ~UserVia [ DDRB ];
			ddr_out = UserVia [ DDRB ];

			new = ( old & ddr_in ) | ( val & ddr_out );
			UserVia [ ORB ] = new;
#ifdef	INFO
			printf ( "User VIA ORB = %2x\n", new );
#endif

			/*
			 * Clear the interrupt flags
			 */

			ClearPortBInterrupts();
			break;
		}

		case ORA :
		{
			byteval		old, new, ddr_in, ddr_out;

			old = UserVia [ ORA ];
			ddr_in = ~UserVia [ DDRA ];
			ddr_out = UserVia [ DDRA ];
			new = ( old & ddr_in ) | ( val & ddr_out );
			UserVia [ ORA ] = new;
			ClearPortAInterrupts();

			/*
			 * FIX ME
			 *
			 * Ought to do something with the new value...
			 *
			 */

#ifdef	INFO
			fprintf ( stderr, "WARNING: write user VIA ORA does nothing\n" );
#endif
			break;
		}
		case DDRB :
		case DDRA :
			UserVia [ addr ] = val;
#ifdef	INFO
			printf ( "User VIA DDR%s = %2x\n",
								( addr == DDRA ) ? "A" : "B", val );
#endif
			return;
			break;

		case T1CL :
		case T1LL :
			/*
			 * The contents of this register will be written to T1CL
			 * when T1CH is written.
			 *
			 */

			UserVia [ T1LL ] = val;
			break;

		case T1LH :
			UserVia [ T1LH ] = val;
			break;

		case T1CH :
			/*
			 * When this register is written, the value is also loaded into
			 * T1LH, T1LL is copied to T1CL and the T1 interrupt flag in
			 * the IFR is cleared.  Also, PB7 goes low if it is an
			 * output.
			 */

			UserVia [ T1CH ] = UserVia [ T1LH ] = val;
			UserVia [ T1CL ] = UserVia [ T1LL ];
			UserViaTimer1 = UserVia [ T1CH ] * 256 + UserVia [ T1CL ];
			UserViaClearInterrupt ( INT_T1 );
			WriteUserVia ( ORB, ReadUserVia ( ORB ) & 0x7f );
			break;

		case T2CL :
			/*
			 * A write to here actually writes the T2 low-order latch.
			 */

			UserVia [ T2LL ] = val;
			break;

		case T2CH :
			/*
			 * Writing this also copies T2LL to T2CL and clears the T2
			 * interrupt flag in the IFR.
			 */

			UserVia [ T2CH ] = val;
			UserVia [ T2CL ] = UserVia [ T2LL ];
			UserViaTimer2 = UserVia [ T2CH ] * 256 + UserVia [ T2CL ];
			UserViaTimer2InterruptEnable = 1;
			UserViaClearInterrupt ( INT_T2 );
			break;

		case SR :
			/*
			 * FIX ME
			 *
			 * Ought to do something with this...
			 *
			 */

			fprintf ( stderr, "write to user VIA SR\n" );
			fprintf ( stderr, "Not yet implemented\n\n" );
			ViaDump ( UserVia );
			FatalError();
			break;

		case ACR :
			/*
			 * Controls what happens with the timers, shift register, PB7
			 * and latching.
			 *
			 */

			/*
			 * FIX ME
			 *
			 * Haven't dealt with the other bits of the register yet...
			 */

			UserVia [ ACR ] = val;
			UserViaTimer1SetPB7 = UserVia [ ACR ] & 0x80;
			UserViaTimer1Continuous = UserVia [ ACR ] & 0x40;
			UserViaTimer2PulseCount = UserVia [ ACR ] & 0x20;
#ifdef	INFO
			printf ( "User VIA ACR = %2x\n", UserVia [ ACR ] );
#endif
			break;

		case PCR :
			/*
			 * FIX ME
			 *
			 * Controls what happens to CA1, CA2, CB1, CB2 and when
			 * they might cause interrupts.
			 *
			 */

			UserVia [ PCR ] = val;
#ifdef	INFO
			fprintf ( stderr, "WARNING: User VIA PCR = %2x\n", UserVia[PCR] );
#endif
			break;

		case IER :
			/*
			 * If bit 7 of the value written is zero, then each set bit
			 * clears the corresponding bit in the IER.
			 *
			 * If bit 7 is set, then each set bit in sets the corresponding
			 * bit in the IER.
			 *
			 */

			if ( val & 0x80 )
				UserVia [ IER ] |= ( val & 0x7f );
			else
				UserVia [ IER ] &= ( val ^ 0x7f );
#ifdef	INFO
			printf ( "User VIA IER = %2x\n", UserVia [ IER ] );
#endif
			/*
			 * Now cause an IRQ if we've just enabled one...
			 */

			UserViaSetInterrupt ( UserVia [ IFR ] );
			break;

		case IFR :
			UserViaClearInterrupt ( val );
#ifdef	INFO
			printf ( "User VIA IFR = %2x\n", UserVia [ IFR ] );
#endif
			break;

		case ORA_nh :
		{
			/*
			 * FIX ME
			 *
			 * The hardware emulation ought to deal with any values set
			 * here.  This is the printer port, however, so it probably
			 * shouldn't even be using this register.
			 */

			/*
			 * Only change the lines which are valid outputs...
			 */

			byteval		old, new, ddr_in, ddr_out;

			old = UserVia [ addr ];
			ddr_in = ~UserVia [ DDRA ];
			ddr_out = UserVia [ DDRA ];

			new = ( old & ddr_in ) | ( val & ddr_out );
			UserVia [ addr ] = new;

#ifdef	INFO
			printf ( "User VIA ORA(nh) = %2x\n", new );
#endif
			break;
		}
		default :
			fprintf ( stderr, "out of range " );
			fprintf ( stderr, "write to user VIA (addr = %x)\n", addr );
			FatalError();
	}
	return;
}


void
UserViaClearInterrupt ( byteval val )
{
	byteval		new;

	/*
	 * If bits 0 to 6 are clear, then bit 7 is clear
	 * otherwise bit 7 is set.
	 *
	 * It is not possible to explicitly write to bit 7.
	 * Writing to a bit clears it.
	 */

	new = ( UserVia [ IFR ] & ~val ) & 0x7f;
	if ( new & UserVia [ IER ] )
		new |= INT_ANY;
	UserVia [ IFR ] = new;
	return;
}


void
UserViaSetInterrupt( byteval IFR_flag )
{
	/*
	 * It's not entirely clear how this works from the documentation
	 * but I suspect that flags will be set in the IFR even if the IER
	 * doesn't allow an interrupt.  All that the IER does is appear to
	 * control whether an IRQ condition occurs, at which point bit 7
	 * of the IFR will also be set.
	 */

	/*
	 * Set the flag for this type of interrupt.
	 */

	UserVia [ IFR ] |= IFR_flag;

	/*
	 * Cause an interrupt if we are allowed to do so.
	 */

/*
#ifdef	INFO
	printf ( "Interrupt requested by user VIA, IFR flag 0x%x\n", IFR_flag );
#endif
*/

	if ( UserVia [ IER ] & IFR_flag )
	{
#ifdef	INFO
		printf ( "Interrupt made\n" );
#endif
		UserVia [ IFR ] |= INT_ANY;
		IRQ();
	}
	return;
}


static void
ClearPortBInterrupts()
{
	/*
	 * Always clear the CB1 active edge flag
	 */

	byteval		flags = 0x10;
	byteval		pcr;

	/*
	 * Clear the CB2 active edge flag unless the PCR indicates that
	 * CB2 is in ``independent'' mode.
	 */

	pcr = UserVia [ PCR ] & 0xe0;
	flags &= (( pcr == 0x20 || pcr == 0x60 ) ? 0x08 : 0x0 );

	UserViaClearInterrupt ( flags );
	return;
}


static void
ClearPortAInterrupts()
{
	/*
	 * Always clear the CA1 active edge flag
	 */

	byteval		flags = 0x02;
	byteval		pcr;

	/*
	 * Clear the CA2 active edge flag unless the PCR indicates that
	 * CA2 is in ``independent'' mode.
	 */

	pcr = UserVia [ PCR ] & 0x0e;
	flags &= (( pcr == 0x02 || pcr == 0x06 ) ? 0x01 : 0x0 );

	UserViaClearInterrupt ( flags );
	return;
}


int
SaveUserVia ( int fd )
{
	byteval		via [ 32 ];

	UserVia [ T1CL ] = UserViaTimer1 & 0xff;
	UserVia [ T1CH ] = UserViaTimer1 >> 8;
	UserVia [ T2CL ] = UserViaTimer2 & 0xff;
	UserVia [ T2CH ] = UserViaTimer2 >> 8;

	memcpy ( via, UserVia, 17 );
	via [ 24 ] = UserViaTimer2InterruptEnable;

	if ( write ( fd, via, 32 ) != 32 )
		return -1;

	return 0;
}


int
RestoreUserVia ( int fd, unsigned int ver )
{
	byteval		via [ 32 ];

	if ( read ( fd, via, 32 ) != 32 )
		return -1;

	memcpy ( UserVia, via, 17 );
	UserViaTimer1 = UserVia [ T1CL ] + ( UserVia [ T1CH ] << 8 );
	UserViaTimer2 = UserVia [ T2CL ] + ( UserVia [ T2CH ] << 8 );
	UserViaTimer2InterruptEnable = via [ 24 ];
	UserViaTimer1SetPB7 = UserVia [ ACR ] & 0x80;
	UserViaTimer1Continuous = UserVia [ ACR ] & 0x40;
	UserViaTimer2PulseCount = UserVia [ ACR ] & 0x20;

	return 0;
}
