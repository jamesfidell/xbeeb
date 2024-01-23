/*
 *
 * $Id: Sound.c,v 1.5 1996/10/01 22:30:33 james Exp $
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
 * $Log: Sound.c,v $
 * Revision 1.5  1996/10/01 22:30:33  james
 * Added VoxWare sound code from James Murray <jsm@jsm-net.demon.co.uk>.
 *
 * Revision 1.4  1996/09/25 19:19:57  james
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
 * Revision 1.3  1996/09/24 23:05:44  james
 * Update copyright dates.
 *
 * Revision 1.2  1996/09/21 22:13:50  james
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

#include "Config.h"
#include "Sound.h"

#ifdef	VOXWARE_SOUND
#include "VoxWare.h"
#endif

#if defined(VOXWARE_SOUND)
#define	SOUND_IMPLEMENTED
#endif


void
SoundWrite ( byteval ora )
{
#ifdef	SOUND_IMPLEMENTED

#ifdef	VOXWARE_SOUND
	VoxWareWrite ( ora );
#endif

#else	/* SOUND_IMPLEMENTED */

	/*
	 * Not really sure what to do here -- let's just ignore the whole
	 * thing...
	 */

#endif	/* SOUND_IMPLEMENTED */

	return;
}


byteval
SpeechRead()
{
	/*
	 * FIX ME
	 */

#ifdef	TODO
	fprintf ( stderr, "Speech read not yet implemented\n" );
#endif
	return 0x00;
}


void
SpeechWrite ( byteval data )
{
	/*
	 * FIX ME
	 */

#ifdef	TODO
	fprintf ( stderr, "Speech write not yet implemented\n" );
#endif
	return;
}


void
InitialiseSound()
{
#ifdef	SOUND_IMPLEMENTED

#ifdef	VOXWARE_SOUND
	InitialiseVoxWare();
#endif

#else	/* SOUND_IMPLEMENTED */

	fprintf ( stderr, "warning: sound is not implemented\n" );

#endif	/* SOUND_IMPLEMENTED */

	return;
}
