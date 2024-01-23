/*
 *
 * $Id: SystemVia.h,v 1.7 2002/01/15 15:46:43 james Exp $
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
 * $Log: SystemVia.h,v $
 * Revision 1.7  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.6  2000/08/16 17:58:28  james
 * Update copyright message
 *
 * Revision 1.5  1996/10/01 00:33:04  james
 * Created separate hardware reset code for each emulated unit and called
 * these from the main initialisation section of the code to do all of the
 * setup necessary.
 *
 * Revision 1.4  1996/09/25 19:19:58  james
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
 * Revision 1.2  1996/09/21 22:13:52  james
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


#ifndef	SYSTEMVIA_H
#define	SYSTEMVIA_H

#include "6522Via.h"


extern  void			ResetSystemVia();
extern  byteval			ReadSystemVia ( int );
extern  void			WriteSystemVia ( int, byteval );
extern	void			SystemViaSetPortAPinLevel ( byteval );
extern	void			SystemViaSetPortBPinLevel ( byteval );
extern	void			SystemViaSetCA1 ( byteval );
extern	void			SystemViaSetCA2 ( byteval );
extern	void			SystemViaSetInterrupt ( byteval );
extern	void			SystemViaClearInterrupt ( byteval );

extern	int				SaveSystemVia ( int );
extern	int				RestoreSystemVia ( int, unsigned int );

extern	int				SystemViaTimer1;
extern	int				SystemViaTimer2;
extern	unsigned char	SystemViaTimer2InterruptEnable;
extern	unsigned char	SystemViaTimer1Continuous;
extern	unsigned char	SystemViaTimer1SetPB7;
extern	unsigned char	SystemViaTimer2PulseCount;
extern	byteval			SystemViaPortAPinLevel;
extern	byteval			SystemViaPortBPinLevel;
extern	Via				SystemVia;


#define	SystemViaCheckInterrupt(x)	( SystemVia [ IFR ] & x )

#endif	/* SYSTEMVIA_H */
