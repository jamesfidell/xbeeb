/*
 *
 * $Id: EmulFS.h,v 1.2 2002/01/15 15:46:43 james Exp $
 *
 * Copyright (C) James Fidell 2002.
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
 * $Log: EmulFS.h,v $
 * Revision 1.2  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.1  2002/01/14 22:18:51  james
 * Added support for .inf files
 *
 *
 */


#ifndef	EMULFS_H
#define	EMULFS_H

extern  byteval			Emulate_OSFILE ( byteval, byteval, byteval );
extern  unsigned int	Emulate_OSFSC ( byteval, byteval, byteval, int* );
extern  int				ChangeDiskDirectory ( char * );
extern  void			ChangeDisk();

#endif	/* EMULFS_H */
