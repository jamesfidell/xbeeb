/*
 *
 * $Id: EFS.h,v 1.2 1996/09/24 23:05:37 james Exp $
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
 * $Log: EFS.h,v $
 * Revision 1.2  1996/09/24 23:05:37  james
 * Update copyright dates.
 *
 * Revision 1.1  1996/09/22 19:23:21  james
 * Add the emulated filing system code.
 *
 *
 */


#ifndef	EFS_H
#define	EFS_H

extern  byteval			Emulate_OSFILE ( byteval, byteval, byteval );
extern  unsigned int	Emulate_OSFSC ( byteval, byteval, byteval, int* );
extern  int				ChangeDiskDirectory ( char * );
extern  void			ChangeDisk();

#endif	/* EFS_H */
