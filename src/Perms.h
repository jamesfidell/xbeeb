/*
 * Copyright (c) James Fidell 1996.
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

#ifndef	PERMS_H
#define	PERMS_H

/*
 * This file exists only to fill in bits that appear to be missing
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

#endif	/* PERMS_H */
