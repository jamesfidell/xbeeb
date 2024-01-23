/*
 *
 * $Id: Compat.h,v 1.5 2002/01/15 15:46:43 james Exp $
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
 * $Log: Compat.h,v $
 * Revision 1.5  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.4  2000/08/16 17:58:27  james
 * Update copyright message
 *
 * Revision 1.3  1996/10/13 17:12:47  james
 * Add local version of "strtoul" for those systems that don't have it.
 *
 * Revision 1.2  1996/10/13 17:07:08  james
 * Moved my own version of the strcasecmp function into Compat.c
 *
 * Revision 1.1  1996/10/12 15:15:02  james
 * Compat.h added to support non-UNIXisms, in particular Win32.
 *
 *
 */

#ifndef	COMPAT_H
#define	COMPAT_H


#ifdef	NEED_STRCASECMP
extern	int				strcasecmp ( char*, char* );
#endif

#ifdef	NEED_STRTOUL
extern	unsigned long	strtoul ( char*, char**, int );
#endif


/*
 * These exist only to fill in bits that appear to be missing
 * on some operating systems.  In particular in is derived from
 * patches that Robert Schmidt (robert@idt.unit.no) sent me to build
 * Xbeeb for eXceed and Win32
 */

/*
 * F_OK and R_OK are used in calls to "access" to check file permissions.
 * On a UNIX system, I'd expect to find them in <unistd.h>
 */

#ifndef	F_OK
#define	F_OK		0
#endif

#ifndef	R_OK
#define	R_OK		4
#endif


/*
 * O_BINARY is a nasty non-UNIX thing to tell the OS we're using binary
 * files. UNIX doesn't use/have this, so I just #define it as zero here
 * -- it's only ORed with lots of other file attributes...
 */

#ifndef	O_BINARY
#define	O_BINARY	0
#endif

#endif	/* COMPAT_H */
