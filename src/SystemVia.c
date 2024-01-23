/*
 *
 * $Id: SystemVia.c,v 1.8 1996/10/01 00:33:03 james Exp $
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
 * $Log: SystemVia.c,v $
 * Revision 1.8  1996/10/01 00:33:03  james
 * Created separate hardware reset code for each emulated unit and called
 * these from the main initialisation section of the code to do all of the
 * setup necessary.
 *
 * Revision 1.7  1996/09/25 19:19:57  james
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
 * Revision 1.6  1996/09/24 23:05:44  james
 * Update copyright dates.
 *
 * Revision 1.5  1996/09/23 19:02:13  james
 * Major overhaul of the VIA emulation code.
 *
 * Revision 1.4  1996/09/22 21:00:54  james
 * Changed IRQ-handling code to be called only when I think an IRQ may have
 * happened.  Converted it all to #defines so that it can be compiled inline.
 *
 * Revision 1.3  1996/09/22 20:36:13  james
 * More conversions of exit(x) to FatalError()
 *
 * Revision 1.2  1996/09/21 22:13:51  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.1  1996/09/21 17:20:41  james
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
#include "6502.h"
#include "Beeb.h"
#include "SystemVia.h"
#include "6522Via.h"
#include "Keyboard.h"
#include "Screen.h"
#include "Sound.h"


static void				ClearPortAInterrupts ( void );
static void				ClearPortBInterrupts ( void );


Via						SystemVia;
int						SystemViaTimer1;
int						SystemViaTimer2;
byteval					SystemViaPortAPinLevel;
byteval					SystemViaPortBPinLevel;
unsigned char			SystemViaTimer2InterruptEnable;
unsigned char			SystemViaTimer1Continuous;
unsigned char			SystemViaTimer1SetPB7;
unsigned char			SystemViaTimer2PulseCount;
static unsigned char	PortALatchEnable;
static unsigned char	PortBLatchEnable;
static byteval			CA1Control, CA2Control, CB1Control, CB2Control;


/*
 * This is a hack to cover bits of the emulation that doesn't exist yet.
 * See where it's used in the code for more detail.
 */

#define	DEFAULT_PORT_B	0xf0


void
ResetSystemVia()
{
	/*
	 * The 6522 resets all registers to zero when it is powered up
	 * or reset, except the counters and latches, which are random.
	 * The SR should be random, too, but it always appears to be 0xff
	 * on my machine.
	 */

	SystemViaPortAPinLevel = DEF_LOGIC;
	SystemViaPortBPinLevel = DEF_LOGIC;

	/*
	 * FIX ME
	 *
	 * IRB has the Speech unit INTERRUPT and READY signals hard-wired
	 * high and the joystick button press lines hardwared as if the buttons
	 * are not pressed.
	 */

	SystemVia [ IRB ] = DEFAULT_PORT_B;
	SystemVia [ IRA ] = 0x00;
	SystemVia [ ORB ] = 0x00;
	SystemVia [ ORA ] = 0x00;
	SystemVia [ DDRB ] = 0x00;
	SystemVia [ DDRA ] = 0x00;

	/*
	 * FIX ME
	 *
	 * Set these to random values...
	 */

	/*
	 * This takes care of the counters...
	 */

	SystemViaTimer1 = 0x1234;
	SystemViaTimer2 = 0x9876;

	SystemVia [ T1LL ] = 0xaa;
	SystemVia [ T1LH ] = 0x55;
	SystemVia [ T2LL ] = 0xa5;

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

	SystemViaTimer2InterruptEnable = 1;

	/*
	 * FIX ME
	 *
	 * This should really be 0x00.  However, the way the hardware is
	 * designed, it looks like 0xff is a sensible default for this until
	 * a more complete emulation is implemented.
	 */

	SystemVia [ SR ] = 0xff;

	/*
	 * FIX ME
	 *
	 * Need to handle the initial value of the SR control bits here, too.
	 */

	SystemVia [ ACR ] = 0x00;
	SystemViaTimer1SetPB7 = 0;
	SystemViaTimer1Continuous = 0;
	SystemViaTimer2PulseCount = 0;
	PortALatchEnable = 0;
	PortBLatchEnable = 0;

	SystemVia [ PCR ] = 0x0;
	CA1Control = CA2Control = CB1Control = CB2Control = 0x0;

	SystemVia [ IER ] = 0x0;
	SystemVia [ IFR ] = 0x0;

	/*
	 * FIX ME
	 *
	 * Some values should be forced at the hardware now the register
	 * values have been set -- write to the sound hardware for instance...
	 */

	return;
}


byteval
ReadSystemVia ( int addr )
{
	/*
	 * Have to handle the address wrap-around -- the VIA only has 16
	 * registers, but 32 memory locations are assigned to it.
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
			 *
			 * In the default configuration, PB0-PB3 are outputs and
			 * PB4-PB7 are inputs.
			 */

			/*
			 * Call ClearPortBInterrupts to clear the relevant flags
			 * in the interrupt flag register.
			 */

			ClearPortBInterrupts();

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

			ClearPortBInterrupts();

			if ( PortBLatchEnable )
			{
#ifdef	INFO
			printf ( "read system VIA IRB = %02x\n", ( SystemVia [ ORB ] &
				SystemVia [ DDRB ] ) | ( SystemVia [ IRB ] &
				~SystemVia [ DDRB ] ));
#endif
				return (( SystemVia [ ORB ] & SystemVia [ DDRB ] ) |
					( SystemVia [ IRB ] & ~SystemVia [ DDRB ] ));
			}
			else
			{
#ifdef	INFO
			printf ( "read system VIA IRB = %02x\n", ( SystemVia [ ORB ] &
				SystemVia [ DDRB ] ) | ( SystemViaPortBPinLevel &
				~SystemVia [ DDRB ] ));
#endif
				return (( SystemVia [ ORB ] & SystemVia [ DDRB ] ) |
					( SystemViaPortBPinLevel & ~SystemVia [ DDRB ] ));
			}
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


			ClearPortAInterrupts();

			if (( !PortBLatchEnable ) || ( addr == ORA_nh ))
			{
#ifdef  INFO
				printf ( "read system VIA IRA(nh) = %02x\n",
													SystemViaPortAPinLevel );
#endif
				return SystemViaPortAPinLevel;
			}
			else
			{
#ifdef	WARNINGS
				fprintf ( stderr, "WARNING: read from system VIA IRA " );
				fprintf ( stderr, "not yet implemented\n" );
#endif
				return SystemVia [ IRA ];
			}
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
			SystemViaTimer2InterruptEnable = 1;
			return SystemViaTimer2 & 0xff;
			break;

		case T2CH :
			return SystemViaTimer2 >> 8;
			break;

		case SR :
			/*
			 * FIX ME
			 *
			 * This will need to do something sane once there's some
			 * serial port emulation.  For the moment, just return a
			 * random value.
			 */

#ifdef	WARNINGS
			fprintf ( stderr, "WARNING: read from system VIA SR " );
			fprintf ( stderr, "not yet implemented\n" );
#endif
			return SystemVia [ SR ];
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
			printf ( "read system VIA IER = %02x\n", SystemVia [ IER ] | 0x80);
#endif
			return ( SystemVia [ IER ] | 0x80 );
			break;

		case IFR :
#ifdef	INFO
			printf ( "read system VIA IFR = %02x\n", SystemVia [ IFR ] );
#endif
			return ( SystemVia [ IFR ] );
			break;
	}

	/* NOTREACHED */

	/*
	 * FIX ME
	 *
	 * Should give a fatal error here ?
	 */

	return 0xff;
}


void
WriteSystemVia ( int addr, byteval val )
{
	/*
	 * Remember to handle the address wrap-around...
	 */

	switch ( addr & 0xf )
	{
		case ORB :
		{
			byteval			latch, enable;

			/*
			 * The values in ORB always change, but the pin levels only
			 * change as and when DDRB configures the pin as an output.
			 */

			SystemVia [ ORB ] = val;

			val &= SystemVia [ DDRB ];
			SystemViaPortBPinLevel &= ~SystemVia [ DDRB ];
			SystemViaPortBPinLevel |= val;

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
			printf ( "System VIA ORB = %02x, ", SystemVia [ ORB ] );
			printf ( "Pin Levels = %02x\n", SystemViaPortBPinLevel );
#endif

			/*
			 * We have an 8-bit latch addressed using pin-levels & 0x07,
			 * with the new data value for the latch in pin-levels & 0x08.
			 */

			latch = SystemViaPortBPinLevel & 0x7;
			enable = SystemViaPortBPinLevel & 0x8;

			switch ( latch )
			{
				case 0x0 :		/* Write enable for sound generator */
					if ( !enable )
						SoundWrite ( SystemViaPortAPinLevel );
					break;

				case 0x1 :		/* Read enable for speech processor */
					if ( !enable )
					{
						SystemViaPortAPinLevel &= SystemVia [ DDRA ];
						SystemViaPortAPinLevel |= ( SpeechRead() &
							~SystemVia [ DDRA ] );
					}
					break;

				case 0x2 :		/* Write enable for speech processor */
					if ( !enable )
						SpeechWrite ( SystemViaPortAPinLevel );
					break;

				case 0x3 :		/* Write enable for keyboard */
					if ( !enable )
					{
						SystemViaPortAPinLevel &= SystemVia [ DDRA ];
						SystemViaPortAPinLevel |=
							( KeyboardWrite ( SystemViaPortAPinLevel ) &
							~SystemVia [ DDRA ] );
					}
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
			break;
		}
		case ORA :
		case ORA_nh :
		{
			byteval			latch, enable;

			/*
			 * The values in ORA always change, but the pin levels only
			 * change as and when DDRA configures the pin as an output.
			 */

			SystemVia [ ORA ] = val;

			val &= SystemVia [ DDRA ];
			SystemViaPortAPinLevel &= ~SystemVia [ DDRA ];
			SystemViaPortAPinLevel |= val;

#ifdef	INFO
			if ( addr == ORA )
				printf ( "System VIA ORA = %02x, ", SystemVia [ ORA ] );
			else
				printf ( "System VIA ORA(nh) = %02x, ", SystemVia [ ORA ] );
			printf ( "Pin Levels = %02x\n", SystemViaPortAPinLevel );
#endif

			/*
			 * Clear the interrupt flags
			 */

			ClearPortAInterrupts();

			/*
			 * The OS *always* uses ORA_nh as it's output port, so
			 * if we're writing to ORA, barf here.  Otherwise, as I
			 * understand it, there's no need to do anything with the
			 * newly calculated values because the addressable latch
			 * and it's PB3 input will need to be tweaked before anything
			 * happens here.
			 *
			 * There's always one, though, isn't there ?  So we take
			 * account of the situation anyway...
			 */

			/*
			 * We only have to deal with devices which respond to
			 * writes on this port.
			 *
			 */

			latch = SystemViaPortBPinLevel & 0x7;
			enable = SystemViaPortBPinLevel & 0x8;

			if ( !enable )
			{
				switch ( latch )
				{
					case 0x0 :		/* Write enable for sound generator */
						SoundWrite ( SystemViaPortAPinLevel );
						break;

					case 0x2 :		/* Write enable for speech processor */
						SpeechWrite ( SystemViaPortAPinLevel );
						break;

					case 0x3 :		/* Write enable for keyboard */
						SystemViaPortAPinLevel &= SystemVia [ DDRA ];
						SystemViaPortAPinLevel |=
							( KeyboardWrite ( SystemViaPortAPinLevel ) &
							~SystemVia [ DDRA ] );
						break;
				}
			}
			break;
		}
		case DDRB :
			/*
			 * FIX ME
			 *
			 * DEFAULT_PORT_B should at some stage be replaced by whatever
			 * values are put on the input lines by the hardware we're
			 * emulating as attached to user VIA port B.
			 */

			SystemVia [ DDRB ] = val;
			SystemViaPortBPinLevel = ~val & DEFAULT_PORT_B;
			SystemViaPortBPinLevel |= ( val & SystemVia [ ORB ] );
#ifdef  INFO
			printf ( "System VIA DDRB = %02x\n", val );
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

			SystemVia [ DDRA ] = val;
			SystemViaPortAPinLevel = ~val & DEF_LOGIC;
			SystemViaPortAPinLevel |= ( val & SystemVia [ ORA ] );
#ifdef  INFO
			printf ( "System VIA DDRA = %02x\n", val );
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
			if ( SystemViaTimer1SetPB7 )
			{
				WriteSystemVia ( ORB, ReadSystemVia ( ORB ) & 0x7f );
				SystemViaTimer1SetPB7 = 1;
			}
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
			SystemViaTimer1SetPB7 = SystemVia [ ACR ] & 0x80;
			SystemViaTimer1Continuous = SystemVia [ ACR ] & 0x40;
			SystemViaTimer2PulseCount = SystemVia [ ACR ] & 0x20;
			PortBLatchEnable = SystemVia [ ACR ] & 0x02;
			PortALatchEnable = SystemVia [ ACR ] & 0x01;
#ifdef	INFO
			printf ( "System VIA ACR = %02x\n", SystemVia [ ACR ] );
#endif
			break;

		case PCR :
			SystemVia [ PCR ] = val;
			CA1Control = val & 0x01;
			CA2Control = ( val >> 1 ) & 0x07;
			CB1Control = ( val >> 4 ) & 0x01;
			CB2Control = ( val >> 5 ) & 0x07;

#ifdef	WARNINGS
			fprintf(stderr, "WARNING: System VIA PCR = %02x\n",SystemVia[PCR]);
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
			printf ( "System VIA IER = %02x\n", SystemVia [ IER ] );
#endif

			/*
			 * Now check to see if we need to cause an IRQ.
			 */

			SystemViaSetInterrupt ( SystemVia [ IFR ] );
			break;

		case IFR :
			SystemViaClearInterrupt ( val );
#ifdef	INFO
			printf ( "System VIA IFR = %02x\n", SystemVia [ IFR ] );
#endif
			break;

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
	printf ( "Interrupt requested by sys VIA, IFR flag 0x%02x\n", IFR_flag );
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


void
SystemViaSetPortBPinLevel ( byteval val )
{
	SystemViaPortBPinLevel &= SystemVia [ DDRB ];
	SystemViaPortBPinLevel |= ( val &= ~SystemVia [ DDRB ] );
	return;
}


void
SystemViaSetCA1 ( byteval level )
{
	if ( CA1Control == level )
		SystemViaSetInterrupt ( INT_CA1 );
	return;
}


void
SystemViaSetCA2 ( byteval level )
{
	if ( !level && ( CA2Control == HS2_NEGATIVE || CA2Control ==
														HS2_NEGATIVE_IND ))
		SystemViaSetInterrupt ( INT_CA2 );
	else
		if ( level && ( CA2Control == HS2_POSITIVE || CA2Control ==
														HS2_POSITIVE_IND ))
		SystemViaSetInterrupt ( INT_CA2 );

	/*
	 * We don't have to worry about the other values of CA2Control -- they're
	 * outputs.
	 */

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

	SystemViaClearInterrupt ( flags );
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

	SystemViaClearInterrupt ( flags );
	return;
}


int
SaveSystemVia ( int fd )
{
	byteval		via [ 32 ];

	SystemVia [ T1CL ] = SystemViaTimer1 & 0xff;
	SystemVia [ T1CH ] = SystemViaTimer1 >> 8;
	SystemVia [ T2CL ] = SystemViaTimer2 & 0xff;
	SystemVia [ T2CH ] = SystemViaTimer2 >> 8;

	memcpy ( via, SystemVia, 20 );
	via [ 24 ] = SystemViaTimer2InterruptEnable;
	via [ 25 ] = ScreenLengthIndex;
	via [ 26 ] = SystemViaPortAPinLevel;
	via [ 27 ] = SystemViaPortBPinLevel;

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

	memcpy ( SystemVia, via, 20 );

	SystemViaTimer1 = SystemVia [ T1CL ] + ( SystemVia [ T1CH ] << 8 );
	SystemViaTimer2 = SystemVia [ T2CL ] + ( SystemVia [ T2CH ] << 8 );

	SystemViaTimer2InterruptEnable = via [ 24 ];
	ScreenLengthIndex = via [ 25 ];
	SystemViaPortAPinLevel = via [ 26 ];
	SystemViaPortBPinLevel = via [ 27 ];

	SystemViaTimer1SetPB7 = SystemVia [ ACR ] & 0x80;
	SystemViaTimer1Continuous = SystemVia [ ACR ] & 0x40;
	SystemViaTimer2PulseCount = SystemVia [ ACR ] & 0x20;
	PortBLatchEnable = SystemVia [ ACR ] & 0x02;
	PortALatchEnable = SystemVia [ ACR ] & 0x01;

	return 0;
}
