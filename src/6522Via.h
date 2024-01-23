/*
 *
 * $Id: 6522Via.h,v 1.7 2002/01/15 15:46:43 james Exp $
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
 * $Log: 6522Via.h,v $
 * Revision 1.7  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.6  2000/08/16 17:58:26  james
 * Update copyright message
 *
 * Revision 1.5  1996/11/24 22:13:24  james
 * Timer values need to be updated before they are read or over-written
 * in the 6522 User and System VIA code.  From a fix by David Ralph Stacey.
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
 * Revision 1.3  1996/09/24 23:05:33  james
 * Update copyright dates.
 *
 * Revision 1.2  1996/09/21 22:13:46  james
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


#ifndef	_6522VIA_H
#define	_6522VIA_H

extern	void	ViaClockUpdate ( byteval );

/*
 * Register names...
 */

#define		ORB		0x00
#define		ORA		0x01
#define		DDRB	0x02
#define		DDRA	0x03
#define		T1CL	0x04
#define		T1CH	0x05
#define		T1LL	0x06
#define		T1LH	0x07
#define		T2CL	0x08
#define		T2CH	0x09
#define		SR		0x0a
#define		ACR		0x0b
#define		PCR		0x0c
#define		IFR		0x0d
#define		IER		0x0e

/*
 * These ones are just for convenience.  They're duplicates of existing
 * registers that it helps to identify in a different way or to store
 * a separate copy of
 */

#define		ORA_nh	0x0f
#define		IRA_nh	0x0f
#define		T2LL	0x10
#define		IRB		0x11
#define		IRA		0x12

/*
 * Interrupt flag names
 */

#define		INT_CA2		0x01
#define		INT_CA1		0x02
#define		INT_SR		0x04
#define		INT_CB2		0x08
#define		INT_CB1		0x10
#define		INT_T2		0x20
#define		INT_T1		0x40
#define		INT_ANY		0x80

/*
 * Handshake modes for C[AB][12]
 */

#define		HS1_NEGATIVE_ACTIVE	0x00
#define		HS2_POSITIVE_ACTIVE	0x01

#define		HS2_NEGATIVE		0x00
#define		HS2_NEGATIVE_IND	0x01
#define		HS2_POSITIVE		0x02
#define		HS2_POSITIVE_IND	0x03
#define		HS2_OUTPUT			0x04
#define		HS2_PULSE			0x05
#define		HS2_LOW				0x06
#define		HS2_HIGH			0x07

/*
 * Default pin levels for ports A and B
 */

#define		DEF_LOGIC	0xff


typedef byteval			Via[20];

extern	void			ViaDump ( Via );
extern	unsigned char	ClockCyclesSoFar;

#endif	/* _6522VIA_H */
