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


#ifndef	_6522VIA_H
#define	_6522VIA_H

extern	void	ViaClockUpdate ( byteval );

#define		ORB		0x00
#define		IRB		0x00
#define		ORA		0x01
#define		IRA		0x01
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
 * These two are kind of awkward, because they sort of map onto ORA/IRA
 */

#define		ORA_nh	0x0f
#define		IRA_nh	0x0f

/*
 * This one's a bit odd too -- it's the write-only half of T2CL
 */

#define		T2LL	0x10

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

typedef byteval			Via[17];

extern	void			ViaDump ( Via );

#endif	/* _6522VIA_H */
