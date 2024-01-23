/*
 *
 * $Id: 6522Via.c,v 1.10 1996/10/10 21:44:01 james Exp $
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
 * $Log: 6522Via.c,v $
 * Revision 1.10  1996/10/10 21:44:01  james
 * Fixes from David Ralph Stacey for scan-line updates.
 *
 * Revision 1.9  1996/10/08 23:05:28  james
 * Corrections to allow clean compilation under GCC 2.7.2 with -Wall -pedantic
 *
 * Revision 1.8  1996/10/07 23:10:54  james
 * Tidied up the VIA register dump display.
 *
 * Revision 1.7  1996/10/07 23:09:35  james
 * Changed screen update handling so that rather than immediately generating
 * a CA1 interrupt, the clock-ticking code pulls the system VIA CA1 low and
 * lets the system VIA code decide what should happen from there.
 *
 * Revision 1.6  1996/10/07 23:05:02  james
 * Modified clock tick updates to avoid dropping half-ticks.
 *
 * Revision 1.5  1996/09/25 19:19:56  james
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
 * Revision 1.4  1996/09/24 23:05:33  james
 * Update copyright dates.
 *
 * Revision 1.3  1996/09/23 16:09:49  james
 * Initial implementation of bitmap MODEs -- including modification of
 * screen handling to use different windows for teletext and bitmapped
 * modes and corrections/improvements to colour- and cursor-handling
 * code.
 *
 * Revision 1.2  1996/09/21 22:13:45  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.1  1996/09/21 17:20:34  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#include <stdio.h>

#include "Config.h"
#include "6522Via.h"
#include "SystemVia.h"
#include "UserVia.h"
#include "Screen.h"
#include "Crtc.h"
#include "Bitmap.h"
#include "Teletext.h"
#include "Modes.h"


void
ViaDump ( Via via )
{
	fprintf ( stderr, "ORB\t= %x\n", via [ ORB ] );
	fprintf ( stderr, "ORA\t= %x\n", via [ ORA ] );
	fprintf ( stderr, "IRB\t= %x\n", via [ IRB ] );
	fprintf ( stderr, "IRA\t= %x\n", via [ IRA ] );
	fprintf ( stderr, "DDRB\t= %x\n", via [ DDRB ] );
	fprintf ( stderr, "DDRA\t= %x\n", via [ DDRA ] );
	fprintf ( stderr, "T1CL\t= %x\n", via [ T1CL ] );
	fprintf ( stderr, "T1CH\t= %x\n", via [ T1CH ] );
	fprintf ( stderr, "T1LL\t= %x\n", via [ T1LL ] );
	fprintf ( stderr, "T1LH\t= %x\n", via [ T1LH ] );
	fprintf ( stderr, "T2CL\t= %x\n", via [ T2CL ] );
	fprintf ( stderr, "T2CH\t= %x\n", via [ T2CH ] );
	fprintf ( stderr, "SR\t= %x\n", via [ SR ] );
	fprintf ( stderr, "ACR\t= %x\n", via [ ACR ] );
	fprintf ( stderr, "PCR\t= %x\n", via [ PCR ] );
	fprintf ( stderr, "IFR\t= %x\n", via [ IFR ] );
	fprintf ( stderr, "IER\t= %x\n", via [ IER ] );
	fprintf ( stderr, "T2LL	= %x\n", via [ T2LL ] );
	return;
}


void
ViaClockUpdate ( byteval val )
{
	static unsigned int			video_sync_ticks = 0;
	static unsigned int			screen_frame_ticks = 0;
	static unsigned int			extra_tick = 0;

	/*
	 * FIX ME
	 *
	 * The CPU is running at 2Mhz, but the 6522's run at 1Mhz, so
	 * the clocks should decrement by half the CPU cycles.
	 * Clock-stretching will affect this process, but I'm not exactly
	 * sure how.
	 */

    /*
     * Need to catch any odd ticks here so that we don't lose them when
	 * val is halved.
	 */

	val += extra_tick;
    extra_tick = val & 0x1;

	val >>= 1;				/* Convert 2Mhz CPU ticks to 1Mhz VIA ticks */
	video_sync_ticks += val;
	screen_frame_ticks += val;

	/*
	 * System VIA CA1 needs to cause an interrupt every 20ms (50Hz) to
	 * coincide with the vertical sync. from the CRTC. (used for
	 * flashing colour changes).
	 */

	if ( video_sync_ticks >= 20000 )
	{
		/*
		 * The CRTC pulses CA1 at this point.  I believe that it uses
		 * negative logic.
		 */

		video_sync_ticks -= 20000;
		SystemViaSetCA1 ( 0 );
	}

	if ( screen_frame_ticks >= 40000 )
	{
		if ( CurrentScreenMode == MODE_TELETEXT )
			TeletextScreenUpdate();
		else
			BitmapScreenUpdate();

		screen_frame_ticks -= 40000;
	}

	if ( CurrentScreenMode == MODE_BITMAP )
	{
		/*
		 * TV does 1 line per 64uS, so update the appropriate line
		 * for the current setting of the clock...
		 */

		/*
		 * FIX ME
		 *
		 * There are lots of magic numbers in this.  I don't understand
		 * their derivation fully.  Here's Dave Stacey's comments :
		 *
		 * The 1900 is being pulled out of a geniune piece of extremely thin
		 * air. It is a universal constant not calculable from Crtc / System
		 * VIA registers and is the time between v-sync flyback and the start
		 * of the screen being redrawn. Since all floating line methods use
		 * v-sync to calculate the timing, we must put this in.
		 *
		 * I #defined VSYNC_TIME to be 1900 in Crtc.c
		 */

		if ( screen_frame_ticks > ( CrtcMagicNumber + VSYNC_TIME ))
			BitmapScanlineUpdate (( screen_frame_ticks - VSYNC_TIME -
												CrtcMagicNumber ) / 64 );
	}

	/*
	 * System VIA Timer 1
	 *
	 * This can be in one of two modes -- free running or one shot, depending
	 * on the setting of bit 6 of the ACR.
	 *
	 */

	SystemViaTimer1 -= val;
	if ( SystemViaTimer1 <= 0 )
	{
		if ( SystemViaTimer1SetPB7 == 1 )
		{
			/*
			 * Need to set/invert the logic level of PB7, depending on
			 * whether we're in one-shot or free-running mode.
			 */

			if ( SystemViaTimer1Continuous )
				SystemViaSetPortBPinLevel ( SystemViaPortBPinLevel ^ 0x80 );
			else
			{
				SystemViaSetPortBPinLevel ( SystemViaPortBPinLevel | 0x80 );
				/*
				 * Prevent further writes until the timer is reset.
				 */
				SystemViaTimer1SetPB7 = 2;
			}
		}

		/*
		 * In continuous mode, we also need to re-load the counter
		 * latches, otherwise just roll the counter over
		 */

		if ( SystemViaTimer1Continuous )
			SystemViaTimer1 += (SystemVia[T1LL] + 256 * SystemVia[T1LH]);
		else
			SystemViaTimer1 &= 0xffff;

		/*
		 * And generate the interrupt if we can.
		 */

		if ( !SystemViaCheckInterrupt ( INT_T1 ))
			SystemViaSetInterrupt ( INT_T1 );
	}

	/*
	 * System VIA Timer 2
	 *
	 * This is either a single timed interrupt, or a countdown on
	 * pulses on PB6, depending on bit 5 of the ACR.  If the timer is
	 * in pulse-counting mode, then that should be handled elsewhere.
	 *
	 */

	if ( !SystemViaTimer2PulseCount )
	{
		SystemViaTimer2 -= val;
		if ( SystemViaTimer2 <= 0 )
		{
			if ( SystemViaTimer2InterruptEnable )
			{
				/*
				 * FIX ME
				 *
				 * David Stacey commented this next line out to get
				 * Firetrack working.  I really need to look into that
				 * a little more.
				 */

				SystemViaSetInterrupt ( INT_T2 );
				SystemViaTimer2InterruptEnable = 0;
			}

			/*
			 * roll the counter over
			 */

			SystemViaTimer2 &= 0xffff;
		}
	}

	/*
	 * User VIA Timer 1
	 *
	 * This can be in one of two modes -- free running or one shot, depending
	 * on the setting of bit 6 of the ACR.
	 *
	 */

	UserViaTimer1 -= val;
	if ( UserViaTimer1 <= 0 )
	{
		if ( UserViaTimer1SetPB7 == 1 )
		{
			/*
			 * Need to set/invert the logic level of PB7, depending on
			 * whether we're in one-shot or free-running mode.
			 */

			if ( UserViaTimer1Continuous )
				UserViaSetPortBPinLevel ( UserViaPortBPinLevel ^ 0x80 );
			else
			{
				UserViaSetPortBPinLevel ( UserViaPortBPinLevel | 0x80 );
				/*
				 * Prevent further writes until the timer is reset.
				 */
				UserViaTimer1SetPB7 = 2;
			}
		}

		/*
		 * In continuous mode, we also need to re-load the counter
		 * latches, otherwise just roll the counter over.
		 */

		if ( UserViaTimer1Continuous )
			UserViaTimer1 += (UserVia[T1LL] + 256 * UserVia[T1LH]);
		else
			UserViaTimer1 &= 0xffff;

		/*
		 * And generate the interrupt
		 */

		if ( !UserViaCheckInterrupt ( INT_T1 ))
			UserViaSetInterrupt ( INT_T1 );
	}

	/*
	 * User VIA Timer 2
	 *
	 * This is either a single timed interrupt, or a countdown on
	 * pulses on PB6, depending on bit 5 of the ACR.  If the timer is
	 * in pulse-counting mode, then that should be handled elsewhere.
	 *
	 */

	if ( !UserViaTimer2PulseCount )
	{
		UserViaTimer2 -= val;
		if ( UserViaTimer2 <= 0 )
		{
			if ( UserViaTimer2InterruptEnable )
			{
				UserViaSetInterrupt ( INT_T2 );
				UserViaTimer2InterruptEnable = 0;
			}

			/*
			 * Now roll over the counter.
			 */

			UserViaTimer2 &= 0xffff;
		}
	}

	return;
}
