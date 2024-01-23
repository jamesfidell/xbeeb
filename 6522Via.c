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


#include <stdio.h>

#include "Config.h"
#include "6522Via.h"
#include "SystemVia.h"
#include "UserVia.h"
#include "Screen.h"


void
ViaDump ( Via via )
{
	fprintf ( stderr, "ORB/IRB	= %x\n", via [ ORB ] );
	fprintf ( stderr, "ORA/IRA	= %x\n", via [ ORA ] );
	fprintf ( stderr, "DDRB	= %x\n", via [ DDRB ] );
	fprintf ( stderr, "DDRA	= %x\n", via [ DDRA ] );
	fprintf ( stderr, "T1CL	= %x\n", via [ T1CL ] );
	fprintf ( stderr, "T1CH	= %x\n", via [ T1CH ] );
	fprintf ( stderr, "T1LL	= %x\n", via [ T1LL ] );
	fprintf ( stderr, "T1LH	= %x\n", via [ T1LH ] );
	fprintf ( stderr, "T2CL	= %x\n", via [ T2CL ] );
	fprintf ( stderr, "T2CH	= %x\n", via [ T2CH ] );
	fprintf ( stderr, "SR	= %x\n", via [ SR ] );
	fprintf ( stderr, "ACR	= %x\n", via [ ACR ] );
	fprintf ( stderr, "PCR	= %x\n", via [ PCR ] );
	fprintf ( stderr, "IFR	= %x\n", via [ IFR ] );
	fprintf ( stderr, "IER	= %x\n", via [ IER ] );
	fprintf ( stderr, "ORA_nh	= %x\n", via [ ORA_nh ] );
	fprintf ( stderr, "T2LL	= %x\n", via [ T2LL ] );
	return;
}


void
ViaClockUpdate ( unsigned char val )
{
	static unsigned int			clock_ticks = 0;

	/*
	 * FIX ME
	 *
	 * The CPU is running at 2Mhz, but the 6522's run at 1Mhz, so
	 * the clocks should decrement by half the CPU cycles.
	 * If so, what happens about 3-byte instructions ?
	 */

	val >>= 1;
	clock_ticks += val;

	/*
	 * System VIA CA1 needs to cause an interrupt every 20ms (50Hz) to
	 * coincide with the vertical sync. from the CRTC. (used for
	 * flashing colour changes).
	 */

	if ( clock_ticks >= 20000 )
	{
		/*
		 * Now we need to cause an interrupt request.
		 */

		clock_ticks -= 20000;
		SystemViaSetInterrupt ( INT_CA1 );

		/*
		 * Also at this point we'll take the opportunity to update
		 * the screen image to the window
		 */

		UpdateScreenImage();
	}

	/*
	 * System VIA Timer 1
	 *
	 * This can be in one of two modes -- free running or one shot, depending
	 * on the setting of bit 6 of the ACR.
	 *
	 */

	if ( SystemViaTimer1Continuous )
	{
		SystemViaTimer1 -= val;
		if ( SystemViaTimer1 <= 0 )
		{
			SystemViaSetInterrupt ( INT_T1 );
			SystemViaTimer1 += (SystemVia [ T1LL ] + 256 * SystemVia [ T1LH ]);
		}
	}
	else
	{											/* one shot mode */
		/*
		 * FIX ME
		 *
		 * run T1 in one-shot mode.
		 */
	}

	/*
	 * System VIA Timer 2
	 *
	 * This is either a single timed interrupt, or a countdown on
	 * pulses on PB6, depending on bit 5 of the ACR.
	 *
	 */

	if ( SystemViaTimer2PulseCount )
	{
		/*
		 * FIX ME
		 *
		 * run T2 counting pulses on PB6
		 */
	}
	else
	{
		SystemViaTimer2 -= val;
		if ( SystemViaTimer2InterruptEnable && SystemViaTimer2 <= 0 )
		{
			SystemViaSetInterrupt ( INT_T2 );
			SystemViaTimer2InterruptEnable = 0;
		}
	}

	/*
	 * User VIA Timer 1
	 *
	 * This can be in one of two modes -- free running or one shot, depending
	 * on the setting of bit 6 of the ACR.
	 *
	 */

	if ( UserViaTimer1Continuous )
	{
		UserViaTimer1 -= val;
		if ( UserViaTimer1 <= 0 )
		{
			UserViaSetInterrupt ( INT_T1 );
			UserViaTimer1 += (UserVia [ T1LL ] + 256 * UserVia [ T1LH ]);
		}
	}
	else
	{											/* one shot mode */
		/*
		 * FIX ME
		 *
		 * run T1 in one-shot mode.
		 */
	}

	/*
	 * User VIA Timer 2
	 *
	 * This is either a single timed interrupt, or a countdown on
	 * pulses on PB6, depending on bit 5 of the ACR.
	 *
	 */

	if ( UserViaTimer2PulseCount )
	{
		/*
		 * FIX ME
		 *
		 * run T2 counting pulses on PB6
		 */
	}
	else
	{
		UserViaTimer2 -= val;
		if ( UserViaTimer2InterruptEnable && UserViaTimer2 <= 0 )
		{
			UserViaSetInterrupt ( INT_T2 );
			UserViaTimer2InterruptEnable = 0;
		}
	}

	return;
}
