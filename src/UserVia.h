/*
 *
 * $Id: UserVia.h,v 1.8 2002/01/15 15:46:43 james Exp $
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
 * $Log: UserVia.h,v $
 * Revision 1.8  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.7  2000/08/16 17:58:29  james
 * Update copyright message
 *
 * Revision 1.6  1996/10/01 00:33:06  james
 * Created separate hardware reset code for each emulated unit and called
 * these from the main initialisation section of the code to do all of the
 * setup necessary.
 *
 * Revision 1.5  1996/09/25 19:19:58  james
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
 * Revision 1.4  1996/09/24 23:05:45  james
 * Update copyright dates.
 *
 * Revision 1.3  1996/09/23 16:09:53  james
 * Initial implementation of bitmap MODEs -- including modification of
 * screen handling to use different windows for teletext and bitmapped
 * modes and corrections/improvements to colour- and cursor-handling
 * code.
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


#ifndef	USERVIA_H
#define	USERVIA_H

#include "6522Via.h"


extern  void			ResetUserVia();
extern  byteval			ReadUserVia ( int );
extern  void			WriteUserVia ( int, byteval );
extern	void			UserViaSetInterrupt ( byteval );
extern	void			UserViaClearInterrupt ( byteval );
extern	void			UserViaSetPortAPinLevel ( byteval );
extern	void			UserViaSetPortBPinLevel ( byteval );

extern	int				SaveUserVia ( int );
extern	int				RestoreUserVia ( int, unsigned int );

extern	int				UserViaTimer1;
extern	int				UserViaTimer2;
extern	unsigned char	UserViaTimer2InterruptEnable;
extern	unsigned char	UserViaTimer1Continuous;
extern	unsigned char	UserViaTimer1SetPB7;
extern	unsigned char	UserViaTimer2PulseCount;
extern	byteval			UserViaPortAPinLevel;
extern	byteval			UserViaPortBPinLevel;
extern	Via				UserVia;


#define	UserViaCheckInterrupt(x)	( UserVia [ IFR ] & x )

#endif	/* USERVIA_H */
