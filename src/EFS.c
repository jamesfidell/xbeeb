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


#include	<stdio.h>
#include	<stdlib.h>
#include	<fcntl.h>
#include	<string.h>

#include	"Config.h"

#ifdef	EMUL_FS

#ifdef	NEED_STRCASECMP
#include	<ctype.h>

int			strcasecmp ( char*, char* );
#endif

#include	"EFS.h"
#include	"Memory.h"

static int			LoadFile ( char*, int, int, int* );
static int			SaveFile ( char*, unsigned int, unsigned int,
												unsigned int, unsigned int );
static void			CopyFilename ( unsigned int, char* );

typedef struct
{
	char			Filename [ 9 ];
	unsigned int	LoadAddress;
	unsigned int	ExeAddress;
	unsigned int	FileLength;
	unsigned int	StartSector;
} FileInfo;

static FileInfo		Catalog [ EFS_CATALOG_SIZE ];
static int			CatalogSize;


/*
 * These are global just to make passing the values around easier
 */

char			CurrentDirectory = '$';
char			DiskDirectory [ PATH_MAX ];
char			DiskDirSet = 0;


byteval
Emulate_OSFILE ( byteval A, byteval X, byteval Y )
{
	int						LoadAddress;
	unsigned int			ExeAddress, StartAddress, EndAddress;
	unsigned int			param_block, fname_address;
	char					fname [ 16 ];

	param_block = ( Y << 8 ) + X;
	fname_address = ReadWord ( param_block );

	switch ( A )
	{
		case 0xff :		/* Load a file */
		{
			CopyFilename ( fname_address, fname );
			if ( ReadByte ( param_block + 6 ) == 0 )
			{
				LoadAddress = ReadByte ( param_block + 2 ) +
								( ReadByte ( param_block + 3 ) << 8 ) +
								( ReadByte ( param_block + 4 ) << 16 ) +
								( ReadByte ( param_block + 5 ) << 24 );
				( void ) LoadFile ( fname, -1, LoadAddress, 0 );
			}
			else
				( void ) LoadFile ( fname, 0, 0, 0 );
			return A;
			break;
		}
		case 0x00 :		/* Save a file */
		{
			CopyFilename ( fname_address, fname );
			LoadAddress = ReadByte ( param_block + 2 ) +
							( ReadByte ( param_block + 3 ) << 8 ) +
							( ReadByte ( param_block + 4 ) << 16 ) +
							( ReadByte ( param_block + 5 ) << 24 );
			ExeAddress = ReadByte ( param_block + 6 ) +
							( ReadByte ( param_block + 7 ) << 8 ) +
							( ReadByte ( param_block + 8 ) << 16 ) +
							( ReadByte ( param_block + 9 ) << 24 );
			StartAddress = ReadByte ( param_block + 10 ) +
							( ReadByte ( param_block + 11 ) << 8 ) +
							( ReadByte ( param_block + 12 ) << 16 ) +
							( ReadByte ( param_block + 13 ) << 24 );
			EndAddress = ReadByte ( param_block + 14 ) +
							( ReadByte ( param_block + 15 ) << 8 ) +
							( ReadByte ( param_block + 16 ) << 16 ) +
							( ReadByte ( param_block + 17 ) << 24 );
			( void ) SaveFile ( fname, LoadAddress, ExeAddress, StartAddress,
																EndAddress );
			return A;
			break;
		}
		case 0x01 :
		case 0x02 :
		case 0x03 :
		case 0x04 :
		case 0x05 :
		case 0x06 :
		default :
			fprintf ( stderr, "Unimplemented EFS_OSFILE (A = 0x%02x)\n", A );
			FatalError();
			break;
	}
}


unsigned int
Emulate_OSFSC ( byteval A, byteval X, byteval Y, int *pPC )
{
	unsigned int		ExeAddress;

	*pPC = -1;

	switch ( A )
	{
		case 0x00 :		/* *OPT */
		{
			/*
			 * FIX ME
			 *
			 * These aren't meaningful to implement as things stand --
			 * virtual tapes and disks working directly with the hardware
			 * emulation will make them much more sensible.
			 * 
			 */
			printf ( "EFS_OSFSC ignored *OPT command\n" );
			break;
		}
		case 0x01 :		/* check EOF */
		{
			/*
			 * FIX ME
			 *
			 * Something do be done when data files are properly emulated.
			 */
			fprintf ( stderr, "Unimplemented EFS_OSFSC (A = 0x%02x)\n", A );
			FatalError();
			break;
		}
		case 0x02 :		/* a / command */
		case 0x03 :		/* unrecognised command */
		case 0x04 :		/* *RUN */
		{
			unsigned int	fname_address;
			char			fname[16];

			fname_address = ( Y << 8 ) + X;
			CopyFilename ( fname_address, fname );

			( void ) LoadFile ( fname, 0, 0, &ExeAddress );

			/*
			 * FIX ME
			 *
			 * Another addressing kludge...
			 * Fortunately not a major one, though, since there's
			 * no TUBE emulation, stripping of the high order bits that
			 * give the correct processor memory into which to load the
			 * stuff doesn't really make a lot of odds.
			 */

			*pPC = ExeAddress & 0xffff;
			break;
		}
		case 0x05 :		/* *CAT */
		{
			int			i = 0;

			printf ( "*CAT is not fully implemented -- \n\n" );

			while ( !DiskDirSet )
				ChangeDisk();

			while ( i < CatalogSize )
			{
				printf ( "%-10s load = %06X, exe = %06X, length =%06X\n",
					Catalog [ i ].Filename, Catalog [ i ].LoadAddress,
					Catalog [ i ].ExeAddress, Catalog [ i ].FileLength );
				i++;
			}
			putchar ( '\n' );
			break;
		}
		case 0x06 :		/* new FS starting */
		{
			/*
			 * FIX ME
			 *
			 * Until the FS emulation is better, there's really nothing
			 * much to do with this call.
			 */
#ifdef	INFO
			printf ( "EFS_OSFSC (A = 0x%02x) ignored\n", A );
#endif
			break;
		}
		case 0x07 :		/* filing system file handle range request */
		{
			/*
			 * FIX ME
			 *
			 * Another thing that's waiting for proper data file
			 * support.
			 */
			fprintf ( stderr, "Unimplemented EFS_OSFSC (A = 0x%02x)\n", A );
			FatalError();
			break;
		}
		case 0x08 :		/* processing an OS command */
		{
			/*
			 * The emulated FS completely ignores this.  If I was making
			 * a more disk-style emulation, I'd need to trap *ENABLE
			 * here.
			 */
			break;
		}
		default :
		{
			fprintf ( stderr, "Unimplemented EFS_OSFSC (A = 0x%02x)\n", A );
			FatalError();
			break;
		}
	}

	return ( A + ( X << 8 ) + ( Y << 16 ));
}


static int
LoadFile ( char *fname, int got_address, int address, int *pExe )
{
	char			fullname [ PATH_MAX ];
	unsigned char	filebyte;
	int				file_fd, i, found = -1;
	unsigned int	len;

	while ( !DiskDirSet )
	{
		printf ( "No disk directory is currently set\n" );
		printf ( "Set the directory now...\n" );
		ChangeDisk();
	}

	i = 0;
	while ( i < CatalogSize && found < 0 )
	{
		if ( strcasecmp ( Catalog [ i ].Filename, fname ) == 0 )
			found = i;
		i++;
	}

	if ( found < 0 )
	{
		printf ( "File %s not found in catalog\n", fname );
		printf ( "EFS cannot generate correct error\n" );
		return -1;
	}

	( void ) strcpy ( fullname, DiskDirectory );

	/*
	 * We copy the filename in the catalog to the file path, not the
	 * name we are called with, because that guarantees that we get
	 * the correct case.
	 */

	if (( strlen ( fullname ) + strlen ( fname ) + 2 ) > PATH_MAX )
	{
		fprintf ( stderr, "pathname is too long\n" );
		fprintf ( stderr, "EFS cannot generate correct error\n" );
		return -1;
	}
	( void ) strcat ( fullname, "/" );
	( void ) strcat ( fullname, Catalog [ found ].Filename );

	if (( file_fd = open ( fullname, O_RDONLY )) < 0 )
	{
		fprintf ( stderr, "File %s found in EFS catalog, ", fname );
		fprintf ( stderr, "but not in directory %s\n", DiskDirectory );
		FatalError();
	}

	/*
	 * FIX ME
	 *
	 * Another nasty hack to bring load addresses back into range and to
	 * avoid the mess caused by local/Tube addresses.  See the comment
	 * on the earlier one for details.
	 */


	if ( got_address )
		address &= 0xffff;
	else
		address = Catalog [ found ].LoadAddress & 0xffff;

	/*
	 * The file is read in one byte at a time so that the data can
	 * be written into memory using WriteByte, which will deal with
	 * any nasty little traps such as writing into ROM etc.
	 */

	for ( len = Catalog[ found ].FileLength; len > 0; len-- )
	{
		if ( read ( file_fd, &filebyte, 1 ) != 1 )
		{
			fprintf ( stderr, "Read of file %s failed\n", fname );
			FatalError();
		}

		WriteByte ( address++, filebyte );
	}

	( void ) close ( file_fd );

	if ( pExe )
		*pExe = Catalog[ found ].ExeAddress;
	return 0;
}


void
CopyFilename ( unsigned int src, char *tgt )
{
	int				quoted = 0, fstart = 0;
	unsigned int	i;
	char			c;

	if ( ReadByte ( src ) == '"' )
	{
		src++;
		quoted = 1;
	}

	if ( ReadByte ( src + 1 ) != '.' )
	{
		tgt [ 0 ] = CurrentDirectory;
		tgt [ 1 ] = '.';
		fstart = 2;
	}

	/*
	 * Read in the actual filename -- up to the carriage
	 * return or the first space.
	 */

	i = 0;
	while ((( c = ReadByte ( src + i )) != 0x0d )
								&& c != ' ' && ( !quoted || c != '"' ))
	{
		tgt [ fstart + i ] = c;
		i++;
	}
	tgt [ fstart + i ] = 0x0;

	return;
}


void
ChangeDisk()
{
	static char		*Home = 0;
	char			InputBuffer [ PATH_MAX ];
	char			done = 0;

	if ( !Home )
		Home = getenv ( "HOME" );

	printf ( "The current disk directory is " );
	if ( DiskDirSet )
		printf ( "'%s'\n", DiskDirectory );
	else
		printf ( "unset\n" );

	while ( !done )
	{
		printf ( "New directory : " );
		if ( fgets ( InputBuffer, PATH_MAX - 1, stdin ) == 0 )
		{
			perror ( "Input error\n : " );
			fprintf ( stderr, "Try again\n" );
			continue;
		}

		if ( *InputBuffer == '\n' )
		{
			printf ( "Directory unchanged\n" );
			done = 1;
			continue;
		}

		* ( strchr ( InputBuffer, '\n' )) = '\0';

		switch ( *InputBuffer )
		{
			case '+' :
			{
				/*
				 * New directory is relative to XBEEBDISKS
				 */

				char		tmp [ PATH_MAX ];

				if (( strlen(InputBuffer) + strlen(XBEEBDISKS)) > PATH_MAX )
				{
					fprintf ( stderr, "The pathname is too long\n" );
					fprintf ( stderr, "Try again\n" );
					continue;
				}

				( void ) strcpy ( tmp, & ( InputBuffer[1] ));
				( void ) strcpy ( InputBuffer, XBEEBDISKS );
				( void ) strcat ( InputBuffer, tmp );

				if ( ChangeDiskDirectory ( InputBuffer ) >= 0 )
					done = 1;
				break;
			}
			case '~' :
			{
				/*
				 * New directory is relative to $HOME
				 */

				char		tmp [ PATH_MAX ];

				if ((strlen ( InputBuffer ) + strlen ( Home ) - 1) > PATH_MAX)
				{
					fprintf ( stderr, "The pathname is too long\n" );
					fprintf ( stderr, "Try again\n" );
					continue;
				}

				( void ) strcpy ( tmp, & ( InputBuffer[1] ));
				( void ) strcpy ( InputBuffer, Home );
				( void ) strcat ( InputBuffer, tmp );
				if ( ChangeDiskDirectory ( InputBuffer ) >= 0 )
					done = 1;
				break;
			}
			default :		/* full path, relative path -- no change */
				if ( ChangeDiskDirectory ( InputBuffer ) >= 0 )
					done = 1;
				break;
		}

		if ( done )
		{
			printf ( "New directory set to : %s\n", DiskDirectory );
		}
	}
}


int
ChangeDiskDirectory ( char *new )
{
	char			catalog [ PATH_MAX ], buff [ 80 ];
	FILE			*cat_fp;
	FileInfo		*p;

	if (( strlen ( new ) + strlen ( CAT_NAME ) + 1) > PATH_MAX )
	{
		fprintf ( stderr, "catalogue pathname is too long\n" );
		return -1;
	}

	( void ) strcpy ( catalog, new );
	( void ) strcat ( catalog, "/" );
	( void ) strcat ( catalog, CAT_NAME );

	( void ) strcpy ( DiskDirectory, new );
	DiskDirSet = 1;
	CatalogSize = 0;

	if (( cat_fp = fopen ( catalog, "r" )) == 0 )
	{
		/*
		 * If we can't open the disk catalog, let's just assume that
		 * at some stage we're going to write one and pretend there
		 * are currently no files listed.
		 */

		return 0;
	}

	p = Catalog;
	while ( fgets ( buff, 79, cat_fp ) > 0 && CatalogSize < EFS_CATALOG_SIZE )
	{

		/*
		 * Filenames are right-padded with spaces initially.
		 */

		( void ) strncpy ( p -> Filename, buff, 9 );
		p -> Filename [ 9 ] = '\0';

		sscanf ( buff, "%s %X %X %X %X", p -> Filename, &( p -> LoadAddress ),
			&( p -> ExeAddress), &( p -> FileLength ), &( p -> StartSector ));
		CatalogSize++;
		p++;
	}

	( void ) fclose ( cat_fp );
	return 0;
}


static int
SaveFile ( char *fname, unsigned int load, unsigned int exe,
									unsigned int start, unsigned int end )
{
	int				fd,	size, p;
	int				l, l1, l2, i, found;
	char			tempfile [ PATH_MAX ], tempcat [ PATH_MAX ];
	char			newfile [ PATH_MAX ], savefile [ PATH_MAX ];
	char			c, catbuf [ 80 ], realfname [ 20 ];
	FILE			*fp;

	while ( !DiskDirSet )
	{
		printf ( "No disk directory is currently set\n" );
		printf ( "Set the directory now...\n" );
		ChangeDisk();
	}

	l1 = strlen ( TMP_FILE );
	l2 = strlen ( fname );
	l = l2 > l1 ? l2 : l1;

	( void ) strcpy ( tempfile, DiskDirectory );
	if (( strlen ( tempfile ) + l + 2 ) > PATH_MAX )
	{
		fprintf ( stderr, "pathname is too long\n" );
		fprintf ( stderr, "EFS cannot generate correct error\n" );
		return -1;
	}
	( void ) strcat ( tempfile, "/" );
	( void ) strcat ( tempfile, TMP_FILE );

	/*
	 * FIX ME
	 *
	 * Yes, yes, it's that ugly address hacking again...
	 */

	start &= 0xffff;
	end &= 0xffff;

	if ( start > end )
	{
		fprintf ( stderr, "start address is greater than end address\n" );
		fprintf ( stderr, "EFS cannot generate correct error\n" );
		return -1;
	}

	size = end - start;

	/*
	 * POSSIBLE ENHANCEMENT ?
	 *
	 * To the best of my knowledge, load and execution addresses are
	 * only given to 6-bytes accuracy, so I'm going to round these off...
	 */

	load &= 0xffffff;
	exe &= 0xffffff;

	if (( fd = creat ( tempfile, 0644 )) < 0 )
	{
		fprintf ( stderr, "error opening %s\n", tempfile );
		perror ( "error is " );
		fprintf ( stderr, "EFS cannot generate correct error\n" );
		return -1;
	}

	/*
	 * Write the data out as a temporary file
	 */

	p = start;
	while ( --size >= 0 )
	{
		c = ReadByte ( p );
		if ( write ( fd, &c, 1 ) < 1 )
		{
			fprintf ( stderr, "write error on %s\n", tempfile );
			fprintf ( stderr, "EFS cannot generate correct error\n" );
			close ( fd );
			unlink ( tempfile );
			return -1;
		}
		p++;
	}
	close ( fd );

	/*
	 * If the file appears in the catalog, we'll need to overwrite the
	 * entry.  Otherwise we create a new one.
	 */

	i = 0;
	found = -1;
	while ( i < CatalogSize && found < 0 )
	{
		if ( strcasecmp ( Catalog [ i ].Filename, fname ) == 0 )
			found = i;
		i++;
	}

	/*
	 * If we haven't found one, add a new catalog entry unless the catalog
	 * is full, in which case return an error.  If we have found one, write
	 * over the old catalog entry.
	 */

	if ( found < 0 && CatalogSize == EFS_CATALOG_SIZE )
	{
		fprintf ( stderr, "EFS catalog is full\n" );
		fprintf ( stderr, "EFS cannot return correct error\n" );
		return -1;
	}

	if ( found < 0 )
	{
		found = CatalogSize;
		CatalogSize++;
	}
	else
		/*
		 * We'll use this later to remove the current file (because it's
		 * only the same name as the file we're writing if you do a
		 * caseless comparison.
		 */

		( void ) strcpy ( realfname, Catalog [ found ].Filename );

	/*
	 * Set start sector to junk, because we don't have anything useful
	 * there.
	 */

	( void ) strcpy ( Catalog [ found ].Filename, fname );
	Catalog [ found ].LoadAddress = load;
	Catalog [ found ].ExeAddress = exe;
	Catalog [ found ].FileLength = end - start;
	Catalog [ found ].StartSector = 0xfff;

	/*
	 * Now write the new catalog
	 */

	/*
	 * FIX ME
	 *
	 * Should check for filename overflow here.
	 */

	( void ) strcpy ( tempcat, DiskDirectory );
	( void ) strcat ( tempcat, "/" );
	( void ) strcat ( tempcat, TMP_CAT );

	fp = fopen ( tempcat, "w" );

	for ( i = 0; i < CatalogSize; i++ )
	{
		sprintf ( catbuf, "%-13s %6.6X %6.6X %6.6X %3.3X\n",
						Catalog [ i ].Filename, Catalog [ i ].LoadAddress,
						Catalog [ i ].ExeAddress, Catalog [ i ].FileLength,
						Catalog [ i ].StartSector );
		if ( fputs ( catbuf, fp ) < 0 )
		{
			fprintf ( stderr, "error writing %s\n", tempcat );
			fprintf ( stderr, "EFS cannot generate correct error\n" );
			fclose ( fp );
			unlink ( tempcat );
			unlink ( tempfile );
			return -1;
		}
	}
	fclose ( fp );

	( void ) strcpy ( newfile, DiskDirectory );
	( void ) strcat ( newfile, "/" );
	( void ) strcat ( newfile, realfname );

	if ( found )
	{
		/*
	 	* FIX ME
		 *
		 * Should check for filename overflow here.
		 */

		( void ) strcpy ( savefile, DiskDirectory );
		( void ) strcat ( savefile, "/" );
		( void ) strcat ( savefile, BAK_FILE );

		if ( rename ( newfile, savefile ) , 0 )
		{
			fprintf ( stderr, "can't create backup of old file\n" );
			fprintf ( stderr, "EFS can't return correct error\n" );
			return -1;
		}
	}

	/*
	 * FIX ME
	 *
	 * Since we've just renamed one file, there should be no problems
	 * renaming the others, so I won't check for errors again.
	 *
	 * It is however possible that errors could occur at this stage if
	 * someone's messing around with things behind our backs.
	 */

	( void ) strcpy ( newfile, DiskDirectory );
	( void ) strcat ( newfile, "/" );
	( void ) strcat ( newfile, fname );

	rename ( tempfile, newfile );
	unlink ( savefile );

	/*
	 * FIX ME
	 *
	 * Should check for overflow here.
	 */

	( void ) strcpy ( newfile, DiskDirectory );
	( void ) strcat ( newfile, "/" );
	( void ) strcat ( newfile, CAT_NAME );

	( void ) strcpy ( savefile, DiskDirectory );
	( void ) strcat ( savefile, "/" );
	( void ) strcat ( savefile, BAK_CAT );

	rename ( newfile, savefile );
	rename ( tempcat, newfile );

	unlink ( savefile );

	return 0;
}


#ifdef	NEED_STRCASECMP

int
strcasecmp ( char *s, char *t )
{
	char		m, n;

	/*
	 * Degenerative cases first
	 */

	if ( !s && !t )
		return 0;

	if ( !s )
		return 1;

	if ( !t )
		return -1;

	while ((( m = toupper ( *s++ )) == ( n = toupper ( *t++ ))) && m != '\0' );

	return m - n;
}

#endif	/* NEED_STRCASECMP */

#endif	/* EMUL_FS */
