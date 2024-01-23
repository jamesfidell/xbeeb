/*
 *
 * $Id: Acia.h,v 1.7 2002/01/15 15:46:43 james Exp $
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
 * $Log: Acia.h,v $
 * Revision 1.7  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.6  2000/08/16 17:58:26  james
 * Update copyright message
 *
 * Revision 1.5  1996/10/01 00:32:58  james
 * Created separate hardware reset code for each emulated unit and called
 * these from the main initialisation section of the code to do all of the
 * setup necessary.
 *
 * Revision 1.4  1996/09/24 23:05:34  james
 * Update copyright dates.
 *
 * Revision 1.3  1996/09/22 21:35:05  james
 * New implementation of the (partial) ACIA emulation.
 *
 * Revision 1.2  1996/09/21 22:13:46  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.1  1996/09/21 17:20:35  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#ifndef	ACIA_H
#define	ACIA_H

/*
 * Flags for the AciaSRClear function
 */

#define	SR_RDRF		0x01
#define	SR_TDRE		0x02
#define	SR_DCD		0x04
#define	SR_CTS		0x08
#define	SR_FE		0x10
#define	SR_OVRN		0x20
#define	SR_PE		0x40
#define	SR_IRQ		0x80


extern  void		ResetAcia();
extern  byteval		ReadAcia ( int );
extern  void		WriteAcia ( int, byteval );
extern	void		AciaSRClear ( byteval );
extern	void		AciaSRSet ( byteval );

extern	int			SaveACIA ( int );
extern	int			RestoreACIA ( int, unsigned int );

#endif	/* ACIA_H */
