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


#ifndef	SYSTEMVIA_H
#define	SYSTEMVIA_H

#include "6522Via.h"

extern  unsigned char       ReadSystemVia ( int );
extern  void				WriteSystemVia ( int, unsigned char );
extern	void				SystemViaSetInterrupt ( unsigned char );
extern	void				SystemViaClearInterrupt ( unsigned char );

extern	int					SaveSystemVia ( int );
extern	int					RestoreSystemVia ( int, unsigned int );

extern	int					SystemViaTimer1;
extern	int					SystemViaTimer2;
extern	unsigned char		SystemViaTimer2InterruptEnable;
extern	unsigned char		SystemViaTimer1Continuous;
extern	unsigned char		SystemViaTimer2PulseCount;
extern	Via					SystemVia;

#endif	/* SYSTEMVIA_H */
