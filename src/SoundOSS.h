/*
 *
 * $Id: SoundOSS.h,v 1.3 2002/01/15 15:46:43 james Exp $
 *
 * Copyright (C) James Fidell 1995-2002.
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
 * $Log: SoundOSS.h,v $
 * Revision 1.3  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/06 11:41:13  james
 * First cut at "proper" sound code
 *
 * Revision 1.1  2000/09/02 18:48:26  james
 * Changed all VoxWare references to OSS
 *
 *
 *
 */


#ifndef	SOUNDOSS_H
#define	SOUNDOSS_H

extern void		InitialiseOSS();
extern void		OSSWrite ( byteval );
extern void		OSSSoundRefresh();

#endif	/* SOUNDOSS_H */