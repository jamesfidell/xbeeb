/*
 *
 * $Id: UserVia.c,v 1.16 2002/01/15 15:46:43 james Exp $
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
 * $Log: UserVia.c,v $
 * Revision 1.16  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.15  2002/01/13 22:27:19  james
 * Fix compile-time warnings
 *
 * Revision 1.14  2000/08/16 17:58:29  james
 * Update copyright message
 *
 * Revision 1.13  1996/11/24 22:13:28  james
 * Timer values need to be updated before they are read or over-written
 * in the 6522 User and System VIA code.  From a fix by David Ralph Stacey.
 *
 * Revision 1.12  1996/11/19 00:56:08  james
 * Writing to T2CL should clear any outstanding T2 interrupt.  Fix from
 * David Ralph Stacey.
 *
 * Revision 1.11  1996/11/15 08:50:59  james
 * When the IER is written, disabled interrupts need to be cleared as well
 * as setting newly enabled interrupts.  (Fix from David Ralph Stacey.)
 *
 * Revision 1.10  1996/10/10 23:20:52  james
 * Corrections to some hideous counter roll-over problems.
 *
 * Revision 1.9  1996/10/01 00:33:06  james
 * Created separate hardware reset code for each emulated unit and called
 * these from the main initialisation section of the code to do all of the
 * setup necessary.
 *
 * Revision 1.8  1996/09/25 19:19:58  james
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
 * Revision 1.7  1996/09/24 23:05:45  james
 * Update copyright dates.
 *
 * Revision 1.6  1996/09/23 19:02:14  james
 * Major overhaul of the VIA emulation code.
 *
 * Revision 1.5  1996/09/23 16:09:53  james
 * Initial implementation of bitmap MODEs -- including modification of
 * screen handling to use different windows for teletext and bitmapped
 * modes and corrections/improvements to colour- and cursor-handling
 * code.
 *
 * Revision 1.4  1996/09/22 21:46:31  james
 * Call FatalError() instead of exit(x);
 *
 * Revision 1.3  1996/09/22 21:00:54  james
 * Changed IRQ-handling code to be called only when I think an IRQ may have
 * happened.  Converted it all to #defines so that it can be compiled inline.
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
#include <unistd.h>
#include <string.h>

#include "Config.h"
#include "6502.h"
#include "Beeb.h"
#include "UserVia.h"
#include "6522Via.h"


Via						UserVia;
int						UserViaTimer1;
int						UserViaTimer2;
byteval					UserViaPortAPinLevel;
byteval					UserViaPortBPinLevel;
unsigned char			UserViaTimer2InterruptEnable;
unsigned char			UserViaTimer1Continuous;
unsigned char			UserViaTimer1SetPB7;
unsigned char			UserViaTimer2PulseCount;
static unsigned char	PortALatchEnable;
static unsigned char	PortBLatchEnable;
static byteval			CA1Control, CA2Control, CB1Control, CB2Control;

static void				ClearPortAInterrupts ( void );
static void				ClearPortBInterrupts ( void );


void
ResetUserVia()
{
	/*
	 * The 6522 resets all registers to zero when it is powered up
	 * or reset, except the counters and latches, which are random.
	 * The SR should be random, too, but it always appears to be 0xff
	 * on my machine.
	 */

	UserViaPortAPinLevel = DEF_LOGIC;
	UserViaPortBPinLevel = DEF_LOGIC;

	UserVia [ IRB ] = 0x00;
	UserVia [ IRB ] = 0x00;
	UserVia [ ORA ] = 0x00;
	UserVia [ ORA ] = 0x00;
	UserVia [ DDRB ] = 0x00;
	UserVia [ DDRA ] = 0x00;

	/*
	 * FIX ME
	 *
	 * Set these to random values...
	 */

	/*
	 * This takes care of the counters...
	 */

	UserViaTimer1 = 0x1234;
	UserViaTimer2 = 0x9876;

	UserVia [ T1LL ] = 0xaa;
	UserVia [ T1LH ] = 0x55;
	UserVia [ T2LL ] = 0xa5;

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

	UserViaTimer2InterruptEnable = 1;

	/*
	 * FIX ME
	 *
	 * This should really be 0x00.  However, the way the hardware is
	 * designed, it looks like 0xff is a sensible default for this until
	 * a more complete emulation is implemented.
	 */

	UserVia [ SR ] = 0xff;

	/*
	 * FIX ME
	 *
	 * Need to handle the initial value of the SR control bits here, too.
	 */

	UserVia [ ACR ] = 0x00;
	UserViaTimer1SetPB7 = 0;
	UserViaTimer1Continuous = 0;
	UserViaTimer2PulseCount = 0;
	PortALatchEnable = 0;
	PortBLatchEnable = 0;

	UserVia [ PCR ] = 0x0;
	CA1Control = CA2Control = CB1Control = CB2Control = 0x0;

	UserVia [ IER ] = 0x0;
	UserVia [ IFR ] = 0x0;

	return;
}


byteval
ReadUserVia ( int addr )
{
	/*
	 * Handle the address wrap-around...
	 */

	addr &= 0x0f;
	switch ( addr )
	{
		case ORB :
			/*
			 * Returns the bit in the ORB when the corresponding DDRB bit
			 * is set and either the pin level or the IRB bit when the
			 * corresponding DDRB bit is clear, for non-latching/latching
			 * modes respectively.
			 */

#ifdef	INFO
			printf ( "read user VIA IRB\n" );
#endif
			/*
			 * Call ClearPortBInterrupts to clear the relevant flags
			 * in the interrupt flag register.
			 */

			ClearPortBInterrupts();

			if ( PortBLatchEnable )
				return (( UserVia [ ORB ] & UserVia [ DDRB ] ) |
					( UserVia [ IRB ] & ~UserVia [ DDRB ] ));
			else
				return (( UserVia [ ORB ] & UserVia [ DDRB ] ) |
					( UserViaPortBPinLevel & ~UserVia [ DDRB ] ));
			break;

		case ORA :
		case ORA_nh :
			/*
			 * FIX ME
			 *
			 * That AUG says that reading this should return the level on
			 * the PA pin if latching is disabled or the IRA bit if latching
			 * is enabled.  Seems strange that you can't read the ORA.  Can
			 * that be correct ?
			 */

			/*
			 * FIX ME
			 *
			 * Furthermore, I assume that reading the non-handshaking version
			 * of the register always returns the pin levels.
			 */

#ifdef	INFO
			printf ( "read user VIA IRA/IRA(nh)\n" );
#endif
			ClearPortAInterrupts();

			if (( !PortBLatchEnable ) || ( addr == ORA_nh ))
				return UserViaPortAPinLevel;
			else
				return UserVia [ IRA ];
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
#ifdef	FASTCLOCK
			ViaClockUpdate ( ClockCyclesSoFar );
			ClockCyclesSoFar = 0;
#endif	/* FASTCLOCK */
			UserViaClearInterrupt ( INT_T1 );
			return UserViaTimer1 & 0xff;
			break;

		case T1CH :
#ifdef	FASTCLOCK
			ViaClockUpdate ( ClockCyclesSoFar );
			ClockCyclesSoFar = 0;
#endif	/* FASTCLOCK */
			return UserViaTimer1 >> 8;
			break;

		case T1LL :
			return UserVia [ T1LL ];
			break;

		case T1LH :
			return UserVia [ T1LH ];
			break;

		case T2CL :
#ifdef	FASTCLOCK
			ViaClockUpdate ( ClockCyclesSoFar );
			ClockCyclesSoFar = 0;
#endif	/* FASTCLOCK */
			UserViaTimer2InterruptEnable = 1;
			UserViaClearInterrupt ( INT_T2 );
			return UserViaTimer2 & 0xff;
			break;

		case T2CH :
#ifdef	FASTCLOCK
			ViaClockUpdate ( ClockCyclesSoFar );
			ClockCyclesSoFar = 0;
#endif	/* FASTCLOCK */
			return UserViaTimer2 >> 8;
			break;

		case SR :
			/*
			 * FIX ME
			 */

#ifdef	WARNINGS
			fprintf ( stderr, "WARNING: read from user VIA SR " );
			fprintf ( stderr, "not yet implemented\n" );
#endif
			return UserVia [ SR ];
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
	}

	/* NOTREACHED */

	/*
	 * FIX ME
	 *
	 * Should return a fatal error here
	 */

	return 0xff;
}


void
WriteUserVia ( int addr, byteval val )
{
	/*
	 * Handle address wrap-around...
	 */

	switch ( addr & 0xf )
	{
		case ORB :
		{
			/*
			 * The values in ORB always change, but the pin levels only
			 * change as and when DDRB configures the pin as an output.
			 */

			UserVia [ ORB ] = val;

			val &= UserVia [ DDRB ];
			UserViaPortBPinLevel &= ~UserVia [ DDRB ];
			UserViaPortBPinLevel |= val;

			/*
			 * FIX ME
			 *
			 * OK, so what do we do with the new values ?
			 */

			/*
			 * Clear the interrupt flags
			 */

			ClearPortBInterrupts();
#ifdef	INFO
			printf ( "User VIA ORB = %02x, ", UserVia [ ORB ] );
			printf ( "Pin Levels = %02x\n", UserViaPortBPinLevel );
#endif
			break;
		}
		case ORA :
		case ORA_nh :
		{
			/*
			 * The values in ORA always change, but the pin levels only
			 * change as and when DDRA configures the pin as an output.
			 */

			UserVia [ ORA ] = val;

			val &= UserVia [ DDRA ];
			UserViaPortAPinLevel &= ~UserVia [ DDRA ];
			UserViaPortAPinLevel |= val;

			/*
			 * FIX ME
			 *
			 * OK, so what do we do with the new values ?
			 * Handling them will probably be different depending on
			 * whether we wrote ORA or ORA_nh.
			 */

			/*
			 * Clear the interrupt flags
			 */

			ClearPortAInterrupts();
#ifdef	INFO
			printf ( "User VIA ORA = %02x, ", UserVia [ ORA ] );
			printf ( "Pin Levels = %02x\n", UserViaPortAPinLevel );
#endif
			break;
		}
		case DDRB :
			/*
			 * FIX ME
			 *
			 * DEF_LOGIC should at some stage be replaced by whatever
			 * values are put on the input lines by the hardware we're
			 * emulating as attached to user VIA port B.
			 */

			UserVia [ DDRB ] = val;
			UserViaPortBPinLevel = ~val & DEF_LOGIC;
			UserViaPortBPinLevel |= ( val & UserVia [ ORB ] );
#ifdef  INFO
			printf ( "User VIA DDRB = %02x\n", val );
#endif
			return;
			break;

		case DDRA :
			/*
			 * FIX ME
			 *
			 * DEF_LOGIC should at some stage be replaced by whatever
			 * values are put on the input lines by the hardware we're
			 * emulating as attached to user VIA port A.
			 */

			UserVia [ DDRA ] = val;
			UserViaPortAPinLevel = ~val & DEF_LOGIC;
			UserViaPortAPinLevel |= ( val & UserVia [ ORA ] );
#ifdef  INFO
			printf ( "User VIA DDRA = %02x\n", val );
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

#ifdef	FASTCLOCK
			ViaClockUpdate ( ClockCyclesSoFar );
			ClockCyclesSoFar = 0;
#endif	/* FASTCLOCK */
			UserVia [ T1CH ] = UserVia [ T1LH ] = val;
			UserVia [ T1CL ] = UserVia [ T1LL ];
			UserViaTimer1 = UserVia [ T1CH ] * 256 + UserVia [ T1CL ];
			UserViaClearInterrupt ( INT_T1 );
			if ( UserViaTimer1SetPB7 )
			{
				UserViaSetPortBPinLevel ( UserViaPortBPinLevel & 0x7f );
				UserViaTimer1SetPB7 = 1;
			}
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

#ifdef	FASTCLOCK
			ViaClockUpdate ( ClockCyclesSoFar );
			ClockCyclesSoFar = 0;
#endif	/* FASTCLOCK */
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

#ifdef	WARNINGS
			fprintf ( stderr, "write to user VIA SR\n" );
			fprintf ( stderr, "Not yet implemented\n\n" );
#endif
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
			PortBLatchEnable = UserVia [ ACR ] & 0x02;
			PortALatchEnable = UserVia [ ACR ] & 0x01;
#ifdef	INFO
			printf ( "User VIA ACR = %02x\n", UserVia [ ACR ] );
#endif
			break;

		case PCR :
			UserVia [ PCR ] = val;
			CA1Control = val & 0x01;
			CA2Control = ( val >> 1 ) & 0x07;
			CB1Control = ( val >> 4 ) & 0x01;
			CB2Control = ( val >> 5 ) & 0x07;
#ifdef	WARNINGS
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
			 * Need also to cause/clear IRQs that are subsequently enabled
			 * by changing the IER flags.
			 */

			if ( val & 0x80 )
			{
				UserVia [ IER ] |= ( val & 0x7f );
				UserViaSetInterrupt ( UserVia [ IFR ] );
			}
			else
			{
				UserVia [ IER ] &= ( val ^ 0x7f );
				UserViaClearInterrupt ( val );
			}
#ifdef	INFO
			printf ( "User VIA IER = %2x\n", UserVia [ IER ] );
#endif
			break;

		case IFR :
			UserViaClearInterrupt ( val );
#ifdef	INFO
			printf ( "User VIA IFR = %2x\n", UserVia [ IFR ] );
#endif
			break;
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


void
UserViaSetPortBPinLevel ( byteval val )
{
	UserViaPortBPinLevel &= UserVia [ DDRB ];
	UserViaPortBPinLevel |= ( val &= ~UserVia [ DDRB ] );
	return;
}


static void
ClearPortBInterrupts()
{
	/*
	 * Always clear the CB1 active edge flag
	 */

	byteval		flags = INT_CB1;

	/*
	 * Clear the CB2 active edge flag unless the PCR indicates that
	 * CB2 is in ``independent'' mode.
	 */

	if ( CB2Control == HS2_POSITIVE_IND || CB2Control == HS2_NEGATIVE_IND )
		 flags = INT_CB1 | INT_CB2;

	UserViaClearInterrupt ( flags );
	return;
}


static void
ClearPortAInterrupts()
{
	/*
	 * Always clear the CA1 active edge flag
	 */

	byteval		flags = INT_CA1;

	/*
	 * Clear the CA2 active edge flag unless the PCR indicates that
	 * CA2 is in ``independent'' mode.
	 */

	if ( CA2Control == HS2_POSITIVE_IND || CA2Control == HS2_NEGATIVE_IND )
		 flags = INT_CA1 | INT_CA2;

	UserViaClearInterrupt ( flags );
	return;
}


int
SaveUserVia ( int fd )
{
	byteval		via [ 32 ];

#ifdef	FASTCLOCK
	ViaClockUpdate ( ClockCyclesSoFar );
	ClockCyclesSoFar = 0;
#endif	/* FASTCLOCK */
	UserVia [ T1CL ] = UserViaTimer1 & 0xff;
	UserVia [ T1CH ] = UserViaTimer1 >> 8;
	UserVia [ T2CL ] = UserViaTimer2 & 0xff;
	UserVia [ T2CH ] = UserViaTimer2 >> 8;

	memcpy ( via, UserVia, 20 );
	via [ 24 ] = UserViaTimer2InterruptEnable;
	via [ 26 ] = UserViaPortAPinLevel;
	via [ 27 ] = UserViaPortBPinLevel;

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

	memcpy ( UserVia, via, 20 );

	UserViaTimer1 = UserVia [ T1CL ] + ( UserVia [ T1CH ] << 8 );
	UserViaTimer2 = UserVia [ T2CL ] + ( UserVia [ T2CH ] << 8 );

	UserViaTimer2InterruptEnable = via [ 24 ];
	UserViaPortAPinLevel = via [ 26 ];
	UserViaPortBPinLevel = via [ 27 ];

	UserViaTimer1SetPB7 = UserVia [ ACR ] & 0x80;
	UserViaTimer1Continuous = UserVia [ ACR ] & 0x40;
	UserViaTimer2PulseCount = UserVia [ ACR ] & 0x20;
	PortBLatchEnable = UserVia [ ACR ] & 0x02;
	PortALatchEnable = UserVia [ ACR ] & 0x01;

	return 0;
}
