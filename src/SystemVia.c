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
#include "SystemVia.h"
#include "6522Via.h"
#include "Keyboard.h"
#include "Screen.h"
#include "Sound.h"


/*
 * FIX ME
 *
 * There ought to be a power-on state for all of the registers, but I
 * haven't got a clue what it is, so I write &00 to all of them apart from
 * IRB, which has the Speech unit INTERRUPT and READY signals hard-wired
 * high and the joystick button press lines hardwared as if the buttons
 * are not pressed.
 */

Via		SystemVia =
{
	0xf0, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};


int					SystemViaTimer1 = 0xffff;
int					SystemViaTimer2 = 0xffff;

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

unsigned char		SystemViaTimer2InterruptEnable = 0;
unsigned char		SystemViaTimer1Continuous = 1;
unsigned char		SystemViaTimer2PulseCount = 0;

/*
 * This function is commented out because it's never used.
static void			ClearPortAInterrupts ( void );
*/
static void			ClearPortBInterrupts ( void );

byteval
ReadSystemVia ( int addr )
{
	switch ( addr )
	{
		case IRB :
			/*
			 * In the default configuration, PB0-PB3 are outputs and
			 * PB4-PB7 are inputs.
			 *
			 * Call ClearPortBInterrupts to clear the relevant flags
			 * in the interrupt flag register.
			 */

			/*
			 * FIX ME
			 *
			 * PB4,5 are low when the joystick buttons are pressed.
			 *
			 * PB6,7 are "ready" and "interrupt" from the speech processor.
			 * From looking at the OS code it looks like these are low
			 * when there's something to do.
			 *
			 * These should be set by the relevant bit of hardware
			 * emulation, rather than using the hardwired settings
			 * when the VIA is initialised above.
			 */

#ifdef	INFO
			printf ( "read system VIA IRB, %2x\n", SystemVia [ IRB ] );
#endif
			ClearPortBInterrupts();
			return ( SystemVia [ IRB ] );
			break;

		case IRA :
			/*
			 * FIX ME
			 *
			 * Port A is used, but only through the non-handshaking
			 * register.  If this one is being used then the hardware
			 * is being screwed around with in some very strange
			 * ways that I'm not sure how to handle, so I'll leave
			 * this one for the time being.
			 *
			 */

			fprintf ( stderr, "read from system VIA IRA" );
			fprintf ( stderr, "Not yet implemented\n\n" );
			ViaDump ( SystemVia );
			FatalError();
			break;

		/*
		 * FIX ME
		 *
		 * I assume that reading DDRA and DDRB just returns the current
		 * settings...
		 *
		 */

		case DDRB :
			return ( SystemVia [ DDRB ] );
			break;

		case DDRA :
			return ( SystemVia [ DDRA ] );
			break;

		case T1CL :
			SystemViaClearInterrupt ( INT_T1 );
			return SystemViaTimer1 & 0xff;
			break;

		case T1CH :
			return SystemViaTimer1 >> 8;
			break;

		case T1LL :
			return SystemVia [ T1LL ];
			break;

		case T1LH :
			return SystemVia [ T1LH ];
			break;

		case T2CL :
#ifdef	INFO
			printf ( "read system VIA T2CL = 0x%2x\n", SystemVia [ T2CL ] );
#endif
			SystemViaTimer2InterruptEnable = 1;
			return ( SystemVia [ T2CL ] );
			break;

		case T2CH :
#ifdef	INFO
			printf ( "read system VIA T2CH = 0x%2x\n", SystemVia [ T2CH ] );
#endif
			return ( SystemVia [ T2CH ] );
			break;

		case SR :
			/*
			 * FIX ME
			 *
			 * This will need to do something once there's some serial
			 * port emulation.
			 *
			 */

			fprintf ( stderr, "read from system VIA SR\n" );
			fprintf ( stderr, "Not yet implemented\n\n" );
			ViaDump ( SystemVia );
			FatalError();
			break;

		case PCR :
			/*
			 * FIX ME
			 *
			 * I assume that reading from this register just returns
			 * the value written.
			 */

			return SystemVia [ PCR ];
			break;

		case ACR :
			/*
			 * FIX ME
			 *
			 * I assume that reading from this register just returns
			 * the value written.
			 */

			return SystemVia [ ACR ];
			break;

		case IER :
			/*
			 * On reads bit 7 of the IER is always read as high.
			 */
#ifdef	INFO
			printf ( "read system VIA IER = %2x\n", SystemVia [ IER ] | 0x80 );
#endif
			return ( SystemVia [ IER ] | 0x80 );
			break;

		case IFR :
#ifdef	INFO
			printf ( "read system VIA IFR = %2x\n", SystemVia [ IFR ] );
#endif
			return ( SystemVia [ IFR ] );
			break;

		case IRA_nh :
			/*
			 * FIX ME
			 *
			 * Read/Write to ORA is supposed to clear the IRQ flag in the
			 * IFR.  I don't think read/write to ORA_nh/IRA_nh is supposed
			 * to do the same thing, but I'm not sure.
			 *
			 * This is an input from whichever device was last selected
			 * using the 8-bit latch on port B.  I don't believe that
			 * it's possible that the data could have changed since
			 * then, but it's a potential problem.
			 *
			 */

#ifdef	INFO
			printf ( "read system VIA ORA(nh) = %2x\n", SystemVia [ ORA_nh ] );
#endif
			return ( SystemVia [ ORA_nh ] );
			break;

		default :
			fprintf ( stderr, "out of range " );
			fprintf ( stderr, "read from system VIA (addr = %x)\n", addr );
			FatalError();
	}
	return 0;
}


void
WriteSystemVia ( int addr, byteval val )
{
	switch ( addr )
	{
		case ORB :
		{
			/*
			 * Only change the lines which are valid outputs...
			 */

			byteval		old, new, ddr_in, ddr_out, latch, enable;

			old = SystemVia [ ORB ];
			ddr_in = ~SystemVia [ DDRB ];
			ddr_out = SystemVia [ DDRB ];

			new = ( old & ddr_in ) | ( val & ddr_out );
			SystemVia [ ORB ] = new;
#ifdef	INFO
			printf ( "System VIA ORB = %2x\n", new );
#endif

			/*
			 * We have an 8-bit latch addressed using ORB & 0x07, with
			 * the new data value for the latch in ORB & 0x08.
			 *
			 */

			latch = new & 0x7;
			enable = new & 0x8;

			switch ( latch )
			{
				case 0x0 :		/* Write enable for sound generator */
					if ( !enable )
						SoundWrite ( SystemVia [ ORA_nh ] );
					break;

				case 0x1 :		/* Read enable for speech processor */
					if ( !enable )
						SpeechRead ( SystemVia[DDRA], &SystemVia [ ORA_nh ] );
					break;

				case 0x2 :		/* Write enable for speech processor */
					if ( !enable )
						SpeechWrite ( SystemVia [ ORA_nh ] );
					break;

				case 0x3 :		/* Write enable for keyboard */
					if ( !enable )
						KeyboardWrite ( SystemVia[DDRA], &SystemVia[ORA_nh] );
					break;

				case 0x4 :		/* Screen start for hardware scroll */
					ScreenAddressStartHi ( enable );
					break;

				case 0x5 :		/* Screen start for hardware scroll */
					ScreenAddressStartLo ( enable );
					break;

				case 0x6 :		/* CAPS LOCK led */
					LedSetCapsLock ( enable );
					break;

				case 0x7 :		/* Shift LOCK led */
					LedSetShiftLock ( enable );
					break;
			}

			/*
			 * Clear the interrupt flags
			 */

			ClearPortBInterrupts();
			break;
		}
		case ORA :
			/*
			 * FIX ME
			 *
			 * Don't know what do do about handling this at the moment.
			 * Port A is used by the machine, but only through the non-
			 * handshaking register.  Anything that deals with this
			 * would be screwing with the hardware in some very strange
			 * ways that I'm not sure how to handle.
			 *
			 */

			fprintf ( stderr, "write to system VIA ORA\n" );
			fprintf ( stderr, "Not yet implemented\n\n" );
			ViaDump ( SystemVia );
			FatalError();
			break;

		case DDRB :
		case DDRA :
			SystemVia [ addr ] = val;
#ifdef	INFO
			printf ( "System VIA DDR%s = %2x\n",
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

			SystemVia [ T1LL ] = val;
			break;

		case T1LH :
			SystemVia [ T1LH ] = val;
			break;

		case T1CH :
			/*
			 * When this register is written, the value is also loaded into
			 * T1LH, T1LL is copied to T1CL and the T1 interrupt flag in
			 * the IFR is cleared.  Also, PB7 goes low if it is an
			 * output
			 */

			SystemVia [ T1CH ] = SystemVia [ T1LH ] = val;
			SystemVia [ T1CL ] = SystemVia [ T1LL ];
			SystemViaTimer1 = SystemVia [ T1CH ] * 256 + SystemVia [ T1CL ];
			SystemViaClearInterrupt ( INT_T1 );
			WriteSystemVia ( ORB, ReadSystemVia ( ORB ) & 0x7f );
			break;

		case T2CL :
			/*
			 * A write to here actually writes the T2 low-order latch.
			 */

			SystemVia [ T2LL ] = val;
			break;

		case T2CH :
			/*
			 * Writing this also copies T2LL to T2CL and clears the T2
			 * interrupt flag in the IFR.
			 */

			SystemVia [ T2CH ] = val;
			SystemVia [ T2CL ] = SystemVia [ T2LL ];
			SystemViaTimer2 = SystemVia [ T2CH ] * 256 + SystemVia [ T2CL ];
			SystemViaTimer2InterruptEnable = 1;
			SystemViaClearInterrupt ( INT_T2 );
			break;

		case SR :
			/*
			 * FIX ME
			 *
			 * Ignore this for the time being, but it will need to be
			 * sorted out before the serial emulation can be completed.
			 *
			 */

			fprintf ( stderr, "write to system VIA SR\n" );
			fprintf ( stderr, "Not yet implemented\n\n" );
			ViaDump ( SystemVia );
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

			SystemVia [ ACR ] = val;
			SystemViaTimer1Continuous = SystemVia [ ACR ] & 0x40;
			SystemViaTimer2PulseCount = SystemVia [ ACR ] & 0x20;
#ifdef	INFO
			printf ( "System VIA ACR = %2x\n", SystemVia [ ACR ] );
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

			SystemVia [ PCR ] = val;
#ifdef	INFO
			fprintf( stderr, "WARNING: System VIA PCR = %2x\n",SystemVia[PCR]);
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
				SystemVia [ IER ] |= ( val & 0x7f );
			else
				SystemVia [ IER ] &= ( val ^ 0x7f );
#ifdef	INFO
			printf ( "System VIA IER = %2x\n", SystemVia [ IER ] );
#endif

			/*
			 * Now check to see if we need to cause an IRQ.
			 */

			SystemViaSetInterrupt ( SystemVia [ IFR ] );
			break;

		case IFR :
			SystemViaClearInterrupt ( val );
#ifdef	INFO
			printf ( "System VIA IFR = %2x\n", SystemVia [ IFR ] );
#endif
			break;

		case ORA_nh :
		{
			/*
			 * Only change the lines which are valid outputs...
			 */

			byteval		old, new, ddr_in, ddr_out, latch, enable;

			old = SystemVia [ addr ];
			ddr_in = ~SystemVia [ DDRA ];
			ddr_out = SystemVia [ DDRA ];

			new = ( old & ddr_in ) | ( val & ddr_out );
			SystemVia [ addr ] = new;

#ifdef	INFO
			printf ( "System VIA ORA(nh) = %2x\n", new );
#endif
			/*
			 * Now the outputs have been set they should be picked up
			 * by the correct device when the 3-bit latch in port B
			 * is written.
			 *
			 * We only have to deal with devices which respond to
			 * writes on this port, though.
			 *
			 */

			latch = SystemVia [ ORB ] & 0x7;
			enable = SystemVia [ ORB ] & 0x8;

			switch ( latch )
			{
				case 0x0 :		/* Write enable for sound generator */
					if ( !enable )
						SoundWrite ( SystemVia [ ORA_nh ] );
					break;

				/*
				 * Skip case 0x1 because it's for reading data into Port A,
				 * not writing it...
				 */

				case 0x2 :		/* Write enable for speech processor */
					if ( !enable )
						SpeechWrite ( SystemVia [ ORA_nh ] );
					break;

				case 0x3 :		/* Write enable for keyboard */
					if ( !enable )
						KeyboardWrite ( SystemVia[DDRA], &SystemVia[ORA_nh] );
					break;

				default :
					break;
			}

			break;
		}

		default :
			fprintf ( stderr, "out of range " );
			fprintf ( stderr, "write to system VIA (addr = %x)\n", addr );
			FatalError();
	}
	return;
}


void
SystemViaClearInterrupt ( byteval val )
{
	byteval		new;

	/*
	 * If bits 0 to 6 are clear, then bit 7 is clear
	 * otherwise bit 7 is set.
	 *
	 * It is not possible to explicitly write to bit 7.
	 * Writing to a bit clears it.
	 */

	new = ( SystemVia [ IFR ] & ~val ) & 0x7f;
	if ( new & SystemVia [ IER ] )
		new |= INT_ANY;
	SystemVia [ IFR ] = new;
	return;
}


void
SystemViaSetInterrupt( byteval IFR_flag )
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

	SystemVia [ IFR ] |= IFR_flag;

	/*
	 * Cause an interrupt if we are allowed to do so.
	 */

#ifdef	INFO
	printf ( "Interrupt requested by sys VIA, IFR flag 0x%x\n", IFR_flag );
#endif

	if ( SystemVia [ IER ] & IFR_flag )
	{
#ifdef	INFO
		printf ( "Interrupt made\n" );
#endif
		SystemVia [ IFR ] |= INT_ANY;
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

	pcr = SystemVia [ PCR ] & 0xe0;
	flags &= (( pcr == 0x20 || pcr == 0x60 ) ? 0x08 : 0x0 );

	SystemViaClearInterrupt ( flags );
	return;
}


/*
 * This function is commented out because it's never used...
 */
#if 0
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

	pcr = SystemVia [ PCR ] & 0x0e;
	flags &= (( pcr == 0x02 || pcr == 0x06 ) ? 0x01 : 0x0 );

	SystemViaClearInterrupt ( flags );
	return;
}
#endif

int
SaveSystemVia ( int fd )
{
	byteval		via [ 32 ];

	SystemVia [ T1CL ] = SystemViaTimer1 & 0xff;
	SystemVia [ T1CH ] = SystemViaTimer1 >> 8;
	SystemVia [ T2CL ] = SystemViaTimer2 & 0xff;
	SystemVia [ T2CH ] = SystemViaTimer2 >> 8;

	memcpy ( via, SystemVia, 17 );
	via [ 24 ] = SystemViaTimer2InterruptEnable;
	via [ 25 ] = ScreenLengthIndex;

	if ( write ( fd, via, 32 ) != 32 )
		return -1;

	return 0;
}


int
RestoreSystemVia ( int fd, unsigned int ver )
{
	byteval		via [ 32 ];

	if ( read ( fd, via, 32 ) != 32 )
		return -1;

	memcpy ( SystemVia, via, 17 );
	SystemViaTimer1 = SystemVia [ T1CL ] + ( SystemVia [ T1CH ] << 8 );
	SystemViaTimer2 = SystemVia [ T2CL ] + ( SystemVia [ T2CH ] << 8 );
	SystemViaTimer2InterruptEnable = via [ 24 ];
	SystemViaTimer1Continuous = SystemVia [ ACR ] & 0x40;
	SystemViaTimer2PulseCount = SystemVia [ ACR ] & 0x20;

	ScreenLengthIndex = via [ 25 ];

	return 0;
}