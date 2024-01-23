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


extern  byteval		ReadAcia ( int );
extern  void		WriteAcia ( int, byteval );
extern	void		AciaSRClear ( byteval );

extern	int			SaveACIA ( int );
extern	int			RestoreACIA ( int, unsigned int );

#endif	/* ACIA_H */
