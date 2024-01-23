/*
 *
 * $Id: EFS.c,v 1.5 1996/10/08 23:05:31 james Exp $
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
 * $Log: EFS.c,v $
 * Revision 1.5  1996/10/08 23:05:31  james
 * Corrections to allow clean compilation under GCC 2.7.2 with -Wall -pedantic
 *
 * Revision 1.4  1996/10/07 22:06:33  james
 * Added XDFS ROM & support code from David Ralph Stacey.
 *
 * Revision 1.3  1996/09/24 23:05:37  james
 * Update copyright dates.
 *
 * Revision 1.2  1996/09/22 21:10:28  james
 * Include Beeb.h to get prototype for FatalError()
 *
 * Revision 1.1  1996/09/22 19:23:21  james
 * Add the emulated filing system code.
 *
 *
 */


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	"Config.h"
#include	"Beeb.h"
#include	"Sheila.h"
#include	"Perms.h"


#ifdef	EMUL_FS

#ifdef	NEED_STRCASECMP
#include	<ctype.h>

int			strcasecmp ( char*, char* );
#endif

#include	"EFS.h"
#include	"Memory.h"

static int			LoadFile ( char*, int, int, unsigned int* );
static int			SaveFile ( char*, unsigned int, unsigned int,
												unsigned int, unsigned int );
static short		FindFile ( char* );
static void			CopyFilename ( unsigned int, char* );
static int			WriteCatalog ( char* );

#ifdef	XDFS

static void			CopyFilenameLib ( unsigned int, char* );
static void			CopyDirname ( unsigned int, char* );

#endif	/* XDFS */


typedef struct
{
	char			Filename [ 10 ];
#ifdef	XDFS
	char			LockFlag;
#endif
	unsigned int	LoadAddress;
	unsigned int	ExeAddress;
	unsigned int	FileLength;
	unsigned int	StartSector;
} FileInfo;

static FileInfo		Catalog [ EFS_CATALOG_SIZE ];
static int			CatalogSize;

/*
 * Maximum length of a filename and library specification allowed in the
 * DFS
 */

#define	DFS_FILE_MAX	16

#ifdef	XDFS

/*
 * Maximum length of a disk name
 */

#define	MAX_DISKNAME	12

/*
 * These are special memory locations that XDFS uses.
 */

#define	XDFS_CATWS1		0x0e00
#define	XDFS_FNAMEBASE	0x0e08
#define	XDFS_DIRBASE	0x0e0f
#define	XDFS_CATWS2		0x0f00
#define	XDFS_CATWR		0x0f04
#define	XDFS_NOFILES	0x0f05
#define	XDFS_OPTION		0x0f06
#define	XDFS_NOSECTS	0x0f07
#define	XDFS_ADDRBASE	0x0f08
#define	XDFS_SECTBASE	0x0f0f
#define	XDFS_STATUS		0x1000
#define	XDFS_CURR_DIR	0x10ca
#define	XDFS_CURR_LIB	0x10cc

static int				BootOption;
static int				CatalogWrites;
char					DiskName [ MAX_DISKNAME + 1 ];

#endif	/* XDFS */


/*
 * These are global just to make passing the values around easier
 */

char			DiskDirectory [ PATH_MAX ];
char			DiskDirSet = 0;

#ifndef	XDFS
char			CurrentDirectory = '$';
#endif


byteval
Emulate_OSFILE ( byteval A, byteval X, byteval Y )
{
	int						LoadAddress;
	unsigned int			ExeAddress, StartAddress, EndAddress;
	unsigned int			param_block, fname_address;
	char					fname [ DFS_FILE_MAX ];

	param_block = ( Y << 8 ) + X;
	fname_address = ReadWord ( param_block );

	switch ( A )
	{
		case 0xff :		/* Load a file */
		{
			CopyFilename ( fname_address, fname );
			if ( ReadByte ((( param_block + 6 ) & 0xffff )) == 0 )
			{
				LoadAddress = ReadByte ((( param_block + 2 ) & 0xffff )) +
						( ReadByte ((( param_block + 3 ) & 0xffff )) << 8 ) +
						( ReadByte ((( param_block + 4 ) & 0xffff )) << 16 ) +
						( ReadByte ((( param_block + 5 ) & 0xffff )) << 24 );
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
			LoadAddress = ReadByte ((( param_block + 2 ) & 0xffff )) +
						( ReadByte ((( param_block + 3 ) & 0xffff )) << 8 ) +
						( ReadByte ((( param_block + 4 ) & 0xffff )) << 16 ) +
						( ReadByte ((( param_block + 5 ) & 0xffff )) << 24 );
			ExeAddress = ReadByte ((( param_block + 6 ) & 0xffff )) +
						( ReadByte ((( param_block + 7 ) & 0xffff )) << 8 ) +
						( ReadByte ((( param_block + 8 ) & 0xffff )) << 16 ) +
						( ReadByte ((( param_block + 9 ) & 0xffff )) << 24 );
			StartAddress = ReadByte ((( param_block + 10 ) & 0xffff )) +
						( ReadByte ((( param_block + 11 ) & 0xffff )) << 8 ) +
						( ReadByte ((( param_block + 12 ) & 0xffff )) << 16 ) +
						( ReadByte ((( param_block + 13 ) & 0xffff )) << 24 );
			EndAddress = ReadByte (( param_block + 14 )) +
						( ReadByte ((( param_block + 15 ) & 0xffff )) << 8 ) +
						( ReadByte ((( param_block + 16 ) & 0xffff )) << 16 ) +
						( ReadByte ((( param_block + 17 ) & 0xffff )) << 24 );
			( void ) SaveFile ( fname, LoadAddress, ExeAddress, StartAddress,
																EndAddress );
			return A;
			break;
		}
		case 0x01 :		/* Write the load and exe addresses for a file */
#ifdef	XDFS
		{
			short				idx;

			CopyFilename ( fname_address, fname );
			if (( idx = FindFile ( fname )) < 0 )
			{
				fprintf ( stderr, "File %s not found in catalog\n", fname );
				FatalError();
			}

			LoadAddress = ReadByte ((( param_block + 2 ) & 0xffff )) +
						( ReadByte ((( param_block + 3 ) & 0xffff )) << 8 ) +
						( ReadByte ((( param_block + 4 ) & 0xffff )) << 16 ) +
						( ReadByte ((( param_block + 5 ) & 0xffff )) << 24 );
			ExeAddress = ReadByte ((( param_block + 6 ) & 0xffff )) +
						( ReadByte ((( param_block + 7 ) & 0xffff )) << 8 ) +
						( ReadByte ((( param_block + 8 ) & 0xffff )) << 16 ) +
						( ReadByte ((( param_block + 9 ) & 0xffff )) << 24 );

			Catalog [ idx ].LoadAddress = LoadAddress;
			Catalog [ idx ].ExeAddress = ExeAddress;
			( void ) WriteCatalog ( 0 );

			return A;
			break;
		}
#endif	/* XDFS */

		case 0x02 :		/* Write the load address for a file */
#ifdef	XDFS
		{
			short				idx;

			CopyFilename ( fname_address, fname );
			if (( idx = FindFile ( fname )) < 0 )
			{
				fprintf ( stderr, "File %s not found in catalog\n", fname );
				FatalError();
			}

			LoadAddress = ReadByte ((( param_block + 2 ) & 0xffff )) +
						( ReadByte ((( param_block + 3 ) & 0xffff )) << 8 ) +
						( ReadByte ((( param_block + 4 ) & 0xffff )) << 16 ) +
						( ReadByte ((( param_block + 5 ) & 0xffff )) << 24 );

			Catalog [ idx ].LoadAddress = LoadAddress;
			( void ) WriteCatalog ( 0 );

			return A;
			break;
		}
#endif	/* XDFS */

		case 0x03 :		/* Write the execution address for a file */
#ifdef	XDFS
		{
			short				idx;

			CopyFilename ( fname_address, fname );
			if (( idx = FindFile ( fname )) < 0 )
			{
				fprintf ( stderr, "File %s not found in catalog\n", fname );
				FatalError();
			}

			ExeAddress = ReadByte ((( param_block + 6 ) & 0xffff )) +
						( ReadByte ((( param_block + 7 ) & 0xffff )) << 8 ) +
						( ReadByte ((( param_block + 8 ) & 0xffff )) << 16 ) +
						( ReadByte ((( param_block + 9 ) & 0xffff )) << 24 );

			Catalog [ idx ].ExeAddress = ExeAddress;
			( void ) WriteCatalog ( 0 );

			return A;
			break;
		}
#endif	/* XDFS */

		case 0x04 :		/* Set/clear the lock flag for a file */
#ifdef	XDFS
		{
			short				idx;
			unsigned char		attributes;

			CopyFilename ( fname_address, fname );
			attributes = ReadByte ((( param_block + 14 ) & 0xffff ));

			if (( idx = FindFile ( fname )) < 0 )
			{
				fprintf ( stderr, "File %s not found in catalog\n", fname );
				FatalError();
			}

			Catalog [ idx ].LockFlag =
								(( attributes & 0x0a ) == 0x0a ) ? 'L' : ' ';

			( void ) WriteCatalog ( 0 );

			return A;
			break;
		}
#endif	/* XDFS */

		case 0x05 :		/* Read a file's catalog information */
#ifdef	XDFS
		{
			short				idx;
			unsigned char		byte;

			CopyFilename ( fname_address, fname );
			if (( idx = FindFile ( fname )) < 0 )
			{
				fprintf ( stderr, "File %s not found in catalog\n", fname );
				FatalError();
			}

			WriteByte ( param_block + 2, Catalog [ idx ].LoadAddress & 0xff );
			WriteByte ( param_block + 3,
						( Catalog [ idx ].LoadAddress >> 8 ) & 0xff );

			/*
			 * Take care of TUBE/local adddresses
			 */

			byte = ( Catalog [ idx ].LoadAddress >> 16 ) & 0xff;
			WriteByte ( param_block + 4, byte );
			if ( byte != 0xff )
				byte = 0;
			WriteByte ( param_block + 5, byte );

			WriteByte ( param_block + 6, Catalog [ idx ].ExeAddress & 0xff );
			WriteByte ( param_block + 7,
						( Catalog [ idx ].ExeAddress >> 8 ) & 0xff );
			/*
			 * Take care of TUBE/local adddresses
			 */

			byte = ( Catalog [ idx ].ExeAddress >> 16 ) & 0xff;
			WriteByte ( param_block + 8, byte );
			if ( byte != 0xff )
				byte = 0;
			WriteByte ( param_block + 9, byte );

			WriteByte ( param_block + 10, Catalog [ idx ].FileLength & 0xff );
			WriteByte ( param_block + 11,
						( Catalog [ idx ].FileLength >> 8 ) & 0xff );
			WriteByte ( param_block + 12,
						( Catalog [ idx ].FileLength >> 16 ) & 0xff );
			WriteByte ( param_block + 13, Catalog [ idx ].FileLength >> 24 );
			WriteByte ( param_block + 14,
							( Catalog [idx].LockFlag == 'L' ) ? 0x0a : 0x0 );
			return A;
			break;
		}
#endif	/* XDFS */

		case 0x06 :		/* Delete a file */
#ifdef	XDFS
		{
			short		idx;
			char		delfile [ PATH_MAX ];

			CopyFilename ( fname_address, fname );
			if (( idx = FindFile ( fname )) < 0 )
			{
				fprintf ( stderr, "File %s not found in catalog\n", fname );
				FatalError();
			}

			( void ) strcpy ( delfile, DiskDirectory );
			( void ) strcat ( delfile, "/" );
			( void ) strcat ( delfile, ( Catalog [ idx ].Filename ));

			/*
			 * FIX ME
			 *
			 * This doesn't account for case (in)sensitivity.
			 */

			unlink ( delfile );

			CatalogSize --;
			while ( idx < CatalogSize )
			{
				Catalog [ idx ].LoadAddress = Catalog [ idx + 1 ].LoadAddress;
				Catalog [ idx ].ExeAddress = Catalog [ idx + 1 ].ExeAddress;
				Catalog [ idx ].FileLength = Catalog [ idx + 1 ].FileLength;
				Catalog [ idx ].StartSector = Catalog [ idx + 1 ].StartSector;
				Catalog [ idx ].LockFlag = Catalog [ idx + 1 ].LockFlag;
				( void ) strcpy ( Catalog [ idx ].Filename,
										Catalog [ idx + 1 ].Filename );
				idx++;
			}

			( void ) WriteCatalog ( 0 );
			return A;
			break;
		}
#endif	/* XDFS */

#ifdef	XDFS

		/*
		 * These two cases are specially added to support XDFS
		 */

		case 0x08 :		/* NEWDISC */
		{
			char			tmp [ PATH_MAX ], fname [ PATH_MAX ];

			CopyDirname ( fname_address, fname );

			if (( strlen ( fname ) + strlen ( XBEEBDISKS )) > PATH_MAX )
			{
				fprintf ( stderr, "The pathname is too long\n" );
				fprintf ( stderr, "Directory unchanged - try again\n" );
			}
			else
			{
				( void ) strcpy ( tmp, fname );
				( void ) strcpy ( fname, XBEEBDISKS );
				( void ) strcat ( fname, tmp );

				if ( ChangeDiskDirectory ( fname ))
					WriteByte ( XDFS_STATUS, 0xff );
				else
					WriteByte ( XDFS_STATUS, 0x0 );
			}
			return A;
			break;
		}

		case 0x09 : 		/* *RENAME */
		{
			char			fail;
			char			fname [ DFS_FILE_MAX ], fname2 [ DFS_FILE_MAX ];
			char			thefile [ PATH_MAX ], thefile2 [ PATH_MAX ];
			short			idx;

			CopyFilename ( fname_address, fname );
			CopyFilename (( ReadWord ( param_block+2 )), fname2 );

			if ( FindFile ( fname2 ) >= 0 )
			{
				/*
				 * Serious error:
				 * XDFS says that the new filename is not in the
				 * directory, but we've just found it. Time for
				 * a FatalError methinks...
				 */

				fprintf ( stderr, "File %s is in the catalogue, but", fname2 );
				fprintf ( stderr, " XDFS didn't find it there\n" );
				FatalError();
			}

			if (( idx = FindFile ( fname )) < 0 )
			{
				/*
				 * Oh dear...
				 * The file to rename is not in the EFS catalog. XDFS
				 * should have dealt with this.
				 */

				fprintf ( stderr,"File %s not in EFS catalogue, ",fname);
				fprintf ( stderr,"even though XDFS thinks it is.\n" );
				FatalError();
			}

			/*
			 * Clear the XDFS status byte
			 */
		
			WriteByte ( XDFS_STATUS, 0x0 );
			fail = 0;

			( void ) strcpy ( thefile, DiskDirectory );
			if (( strlen ( thefile ) + strlen ( Catalog [ idx ].Filename )
															+ 2 ) > PATH_MAX )
			{               
				fprintf ( stderr, "pathname is too long\n" );
				fprintf ( stderr, "EFS cannot generate correct error\n" );
				FatalError();
			}
			( void ) strcat ( thefile, "/" );
			( void ) strcat ( thefile, Catalog [ idx ].Filename );

			if ( access ( thefile, R_OK ))
			{
				/*
				 * The file does not exist on the directory but
				 * is in the EFS catalog. We tell XDFS about this by
				 * setting a flag at &1000.
				 */

				WriteByte ( XDFS_STATUS, 0xff );
				fail = -1;
			}

			( void ) strcpy ( thefile2, DiskDirectory );
			if (( strlen ( thefile2 ) + strlen ( fname2 ) + 2 ) > PATH_MAX )
			{
				fprintf ( stderr, "pathname is too long\n" );
				fprintf ( stderr, "EFS cannot generate correct error\n" );
				FatalError();
			}
			( void ) strcat ( thefile2, "/" );
			( void ) strcat ( thefile2, fname2 );

			if ( ! access ( thefile2, F_OK ))
			{
				/*
				 * The file to rename to already exists in the Unix
				 * directory, but is not in the EFS catalog.
				 */

				WriteByte ( XDFS_STATUS, 0xcc );
				fail = -1;
			}

			if ( !fail )
			{
				/*
				 * FIX ME
				 *
				 * Not all systems have rename, do they ?
				 */

				rename ( thefile, thefile2 );
				idx = FindFile ( fname );
				( void ) strcpy ( Catalog [ idx ].Filename, fname2 );
			}
			
			( void ) WriteCatalog ( 0 );
			return A;
			break;
		}

#endif	/* XDFS */

		case 0x07 :		/* Prepare an area of disc space -- Master only */
		default :
			fprintf ( stderr, "Unimplemented EFS_OSFILE (A = 0x%02x)\n", A );
			FatalError();
			break;
	}

	/* NOTREACHED */

	/*
	 * FIX ME
	 *
	 * Should give a fatal error here.
	 */

	return 0xff;
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
#ifdef	XDFS
			/*
			 * *OPT 1 is handled by the DFS ROM.  Here we deal with *OPT 4
			 * because it updates the catalog information.  All other values
			 * for *OPT are ignored and just generate a warning message.
			 */

			if ( X == 4 )
			{
				BootOption = Y & 0x3;
				( void ) WriteCatalog ( 0 );
			}
			else

#endif	/* XDFS */

				/*
				 * FIX ME
				 *
				 * These aren't meaningful to implement as things stand --
				 * virtual tapes and disks working directly with the hardware
				 * emulation will make them much more sensible.
				 * 
				 */

				fprintf ( stderr, "EFS_OSFSC ignored *OPT command\n" );

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
#ifndef	XDFS
		case 0x03 :		/* unrecognised *COMMAND */
#endif
		case 0x04 :		/* *RUN */
		{
			unsigned int	fname_address;
			char			fname[16];

			/*
			 * FIX ME
			 *
			 * Should check to see if the filename is in the disk catalogue
			 * before loading it.
			 */

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

#ifdef	XDFS

		case 3 : 		/* Unrecognised star command */
        {
			unsigned int	fname_address;
			char			fname [ DFS_FILE_MAX ];
			int				idx;

			fname_address = ( Y << 8 ) + X;
			CopyFilename ( fname_address, fname );

			/*
			 * The file could be in the currently selected directory
			 * or it could be in the library directory. We check the
			 * currently selected directory first.
			 */

			if (( idx = FindFile ( fname )) < 0 )
			{

				/*
				 * The file wasn't in the currently selected
				 * directory so we look in the library.
				 */

				CopyFilenameLib ( fname_address, fname );
				if (( idx = FindFile ( fname )) < 0 )
				{

					/*
					 * If we get here then something has gone horribly
					 * wrong. XDFS should be checking that the file
					 * exists in the directory, so the next couple of
					 * lines should never be exectued.
					 */

					fprintf ( stderr, "File %s not found in EFS catalog\n",
																		fname);
					FatalError();
				}
			}

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
#endif	/* XDFS */

		case 0x05 :		/* *CAT */
		{
			int			i;

#ifdef	XDFS
			int			j, off, fptr;
			char		byte;

			/*
			 * If a disc hasn't been selected previously then select
			 * one now.
			 */

			while ( !DiskDirSet )
				ChangeDisk();

			/*
			 * Clear the catalog workspace in main memory. If this were
			 * not done then a blank disc could have the same catalog
			 * as the previous disc - unlikely, but just making sure.
			 *
			 * This also means that we don't have to pad the ends of
			 * fields when we're copying information into them.
			 */

			for ( i = 0; i < 256; i++ )
			{
				WriteByte ( XDFS_CATWS1 + i, ' ' );
				WriteByte ( XDFS_CATWS2 + i, 0x00 );
			}

			/*
			 * Copy the catalog (inc. addresses) into pages &E and &F.
			 */

			for ( i = 0; i < CatalogSize; i++ )
			{
				off = i * 8;
				fptr = XDFS_FNAMEBASE + off;
				for ( j = 2; j < 10; j++ )
				{
					if (!( byte = Catalog [ i ].Filename [ j ] ))
						break;
					WriteByte ( fptr++, byte );
				}

				byte = *Catalog [ i ].Filename; 		/* file directory */
				byte |= ( Catalog [ i ].LockFlag == 'L' ) ? 0x80 : 0x0;
				WriteByte ( XDFS_DIRBASE + off, byte );

				fptr = XDFS_ADDRBASE + off;
				WriteByte( fptr++, Catalog[ i ].LoadAddress & 0xff );
				WriteByte( fptr++, ( Catalog[ i ].LoadAddress >> 8 ) & 0xff );

				WriteByte( fptr++, Catalog[ i ].ExeAddress );
				WriteByte( fptr++, ( Catalog[ i ].ExeAddress >> 8 ) & 0xff );

				WriteByte( fptr++, Catalog[ i ].FileLength );
				WriteByte( fptr++, ( Catalog[ i ].FileLength >> 8 ) & 0xff );

				WriteByte( XDFS_SECTBASE + off,
							Catalog [ i ].StartSector & 0xff );

				byte = ( Catalog [ i ].ExeAddress & 0x30000 ) >> 10;
				byte &= ( Catalog [ i ].FileLength & 0x30000 ) >> 12;
				byte &= ( Catalog [ i ].LoadAddress & 0x30000 ) >> 14;
				byte &= ( Catalog [ i ].StartSector & 0x300 ) >> 8;

				WriteByte ( fptr, byte );
			}

			/*
			 * Clear title memory.  Don't have to clear what is in page 0xf00
			 * because we did that earlier on.
			 */

			for ( i = 0; i < 8; i++)
				WriteByte ( XDFS_CATWS1 + i, 0x0 );

			/* Put new disc title in memory */

			for ( i = 0; i < 8; i++ )
				WriteByte ( XDFS_CATWS1 + i, DiskName [ i ] );

			for ( i = 8; i < 12; i++ )
				WriteByte ( XDFS_CATWS2 + i - 8, DiskName [ i ] );

			WriteByte ( XDFS_CATWR, CatalogWrites );	/* times written */
			WriteByte ( XDFS_NOFILES, CatalogSize * 8 );	/* no. of files */
			WriteByte ( XDFS_OPTION, ( BootOption & 0x3 ) << 4 ); /* option */
			WriteByte ( XDFS_NOSECTS, 0);	/* sectors have no meaning */

#else	/* XDFS */

			printf ( "*CAT is not fully implemented -- \n\n" );

			while ( !DiskDirSet )
				ChangeDisk();

			i = 0;
			while ( i < CatalogSize )
			{
				printf ( "%-10s load = %06X, exe = %06X, length =%06X\n",
					Catalog [ i ].Filename, Catalog [ i ].LoadAddress,
					Catalog [ i ].ExeAddress, Catalog [ i ].FileLength );
				i++;
			}
			putchar ( '\n' );

#endif	/* XDFS */

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
LoadFile ( char *fname, int got_address, int address, unsigned int *pExe )
{
	char			fullname [ PATH_MAX ];
	short			found = -1;
	unsigned char	filebyte;
	int				file_fd;
	unsigned int	len;

	while ( !DiskDirSet )
	{
		printf ( "No disk directory is currently set\n" );
		printf ( "Set the directory now...\n" );
		ChangeDisk();
	}

	if (( found = FindFile ( fname )) < 0 )
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

	if (( file_fd = open ( fullname, O_RDONLY | O_BINARY )) < 0 )
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


static short
FindFile ( char *file )
{
	short		idx = -1, i = 0;

	while ( !DiskDirSet )
	{
		printf ( "No disk directory is currently set\n" );
		printf ( "Set the directory now...\n" );
		ChangeDisk();
	}

	while ( i < CatalogSize && idx < 0 )
	{
		if ( strcasecmp ( Catalog [ i ].Filename, file ) == 0 )
			idx = i;
		i++;
	}

	return idx;
}


static void
CopyFilename ( unsigned int src, char *tgt )
{
	int				quoted = 0, fstart = 0;
	unsigned int	i;
	char			c;

	if ( ReadByte ( src ) == '"' )
	{
		src++;
		src &= 0xffff;
		quoted = 1;
	}

	if ( ReadByte ((( src + 1 ) & 0xffff )) != '.' )
	{
#ifdef	XDFS
		tgt [ 0 ] = ReadByte ( XDFS_CURR_DIR );
#else
		tgt [ 0 ] = CurrentDirectory;
#endif	/* XDFS */
		tgt [ 1 ] = '.';
		fstart = 2;
	}

	/*
	 * Read in the actual filename -- up to the carriage
	 * return or the first space.
	 */

	i = 0;
	while ((( c = ReadByte ((( src + i ) & 0xffff ))) != 0x0d )
							&& c != ' ' && ( !quoted || c != '"' )
							&& (( fstart + i ) < DFS_FILE_MAX ))
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
#ifdef	XDFS
	char			*n;
	int				l;
#endif

	if (( strlen ( new ) + strlen ( CAT_NAME ) + 1) > PATH_MAX )
	{
		fprintf ( stderr, "catalogue pathname is too long\n" );
		return -1;
	}

	( void ) strcpy ( catalog, new );
	( void ) strcat ( catalog, "/" );
	( void ) strcat ( catalog, CAT_NAME );

#ifdef	XDFS

	( void ) memset ( DiskName, 0, MAX_DISKNAME );
	if (( n = strrchr ( new, '/' )))
		n++;
	else
		n = new;
	if (( l = strlen ( n )) > MAX_DISKNAME )
		l = MAX_DISKNAME;
	( void ) strncpy ( DiskName, n, l );

#endif	/* XDFS */

	( void ) strcpy ( DiskDirectory, new );
	DiskDirSet = 1;
	CatalogSize = 0;

	if (( cat_fp = fopen ( catalog, "r" )) == 0 )
	{
#ifdef	XDFS
		/*
		 * We can't open the catalog, so we need to generate an error.
		 * If the user is using *NEWDISC then this will be done by
		 * the XDFS rom.
		 */

		return -1;

#else	/* XDFS */

		/*
		 * If we can't open the disk catalog, let's just assume that
		 * at some stage we're going to write one and pretend there
		 * are currently no files listed.
		 */

		return 0;

#endif	/* XDFS */
	}

	/*
	 * FIX ME
	 *
	 * Really should handle errors here...
	 */

	if ( fgets ( buff, 6, cat_fp ) != 0 )
		sscanf ( buff, "%2d %1d", &CatalogWrites, &BootOption );

	p = Catalog;
	while ( fgets ( buff, 79, cat_fp ) != 0 && CatalogSize < EFS_CATALOG_SIZE )
	{

		/*
		 * Filenames are right-padded with spaces initially.
		 */

		( void ) strncpy ( p -> Filename, buff, 9 );
		p -> Filename [ 9 ] = '\0';

#ifdef	XDFS
		sscanf ( buff, "%s ", p -> Filename );
		p -> LockFlag = buff [ 11 ];
		sscanf ( buff + 12, "%X %X %X %X", &( p -> LoadAddress ),
#else
		sscanf ( buff, "%s %X %X %X %X", p -> Filename, &( p -> LoadAddress ),
#endif	/* XDFS */
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
	char			c, realfname [ 20 ];

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
		p &= 0xffff;
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
#ifdef	XDFS
	Catalog [ found ].LockFlag = ' ';
#endif
	Catalog [ found ].LoadAddress = load;
	Catalog [ found ].ExeAddress = exe;
	Catalog [ found ].FileLength = end - start;
	Catalog [ found ].StartSector = 0x3ff;

	/*
	 * Now write the new catalog.  If an error occurs, we abort.
	 */

	/*
	 * FIX ME
	 *
	 * Should check for filename overflow here.
	 */

	( void ) strcpy ( tempcat, DiskDirectory );
	( void ) strcat ( tempcat, "/" );
	( void ) strcat ( tempcat, TMP_CAT );

	if ( WriteCatalog ( tempcat ) < 0 )
	{
			unlink ( tempfile );
			return -1;
	}

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


static int
WriteCatalog ( char* tempcat )
{
	int			i;
	char		cat [ PATH_MAX ],  catbuf [ 80 ];
	FILE		*fp;

	if ( tempcat )
		/*
		 * FIX ME
		 *
		 * Should check for filename overflow here.
		 */

		( void ) strcpy ( cat, tempcat );
	else
	{
		/*
		 * FIX ME
		 *
		 * Should check for filename overflow here.
		 */

		( void ) strcpy ( cat, DiskDirectory );
		( void ) strcat ( cat, "/" );
		( void ) strcat ( cat, TMP_CAT );
	}

	if (( fp = fopen ( cat, "w" )) == 0 )
	{
		fprintf ( stderr, "can't open catalog file %s for writing\n", cat );
		fprintf ( stderr, "EFS cannot generate correct error\n" );
		return -1;
	}

#ifdef	XDFS
	/*
	 * Update the number of times the catalogue has been written,
	 * wrapping around at 99.
	 */

	if ( ++CatalogWrites == 100 )
		CatalogWrites = 0;

	/*
	 * Write the new catalogue
	 */

	sprintf ( catbuf, "%2.2d %1.1d\n", CatalogWrites, BootOption );
	if ( fputs ( catbuf, fp ) < 0 )
	{
		fprintf ( stderr, "error writing %s\n", cat );
		fprintf ( stderr, "EFS cannot generate correct error\n" );
		fclose ( fp );
		unlink ( cat );
		return -1;
	}

#endif	/* XDFS */

	for ( i = 0; i < CatalogSize; i++ )
	{
#ifdef	XDFS
		sprintf ( catbuf, "%-9s  %c  %6.6X %6.6X %6.6X %3.3X\n",
						Catalog [ i ].Filename, Catalog [ i ].LockFlag,
						Catalog [ i ].LoadAddress, Catalog [ i ].ExeAddress,
						Catalog [ i ].FileLength, Catalog [ i ].StartSector );
#else	/* XDFS */
		sprintf ( catbuf, "%-13s %6.6X %6.6X %6.6X %3.3X\n",
						Catalog [ i ].Filename, Catalog [ i ].LoadAddress,
						Catalog [ i ].ExeAddress, Catalog [ i ].FileLength,
						Catalog [ i ].StartSector );
#endif	/* XDFS */

		if ( fputs ( catbuf, fp ) < 0 )
		{
			fprintf ( stderr, "error writing %s\n", cat );
			fprintf ( stderr, "EFS cannot generate correct error\n" );
			fclose ( fp );
			unlink ( cat );
			return -1;
		}
	}
	fclose ( fp );

	if ( !tempcat )
	{
		char	newfile [ PATH_MAX ], savefile [ PATH_MAX ];
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
		rename ( cat, newfile );

		unlink ( savefile );
	}

	return 0;
}


static void
CopyFilenameLib ( unsigned int src, char *tgt )
{               
	int				quoted = 0, fstart = 0;  
	unsigned int	i;
	char			c;

	if ( ReadByte ( src ) == '"' )
	{
		src++;
		src &= 0xffff;
		quoted = 1;
	}

	if ( ReadByte ((( src + 1 ) & 0xffff )) != '.' )
	{
		tgt [ 0 ] = ReadByte ( XDFS_CURR_LIB );
		tgt [ 1 ] = '.';
		fstart = 2;
	}

	/*
	 * Read in the actual filename -- up to the carriage
	 * return or the first space.
	 */

	i = 0;
	while ((( c = ReadByte ((( src + i ) & 0xffff ))) != 0x0d )
							&& c != ' ' && ( !quoted || c != '"' )
							&& (( fstart + i ) < DFS_FILE_MAX ))
	{
		tgt [ fstart + i ] = c;
		i++;
	}
	tgt [ fstart + i ] = 0x0;

	return;
}


static void
CopyDirname ( unsigned int src, char *tgt )
{
	int				quoted = 0, fstart = 0;
	unsigned int	i;
	char			c;

	if ( ReadByte ( src ) == '"' )
	{  
		src++;
		src &= 0xffff;
		quoted = 1;
	}

	/*
	 * Read in the actual directory name -- up to the carriage
	 * return or the first space.
	 */

	i = 0;   
	while ((( c = ReadByte ((( src + i ) & 0xffff ))) != 0x0d )
							&& c != ' ' && ( !quoted || c != '"' )
							&& (( fstart + i ) < PATH_MAX ))
	{
		tgt [ fstart + i ] = c;
		i++;
	}
	tgt [ fstart + i ] = 0x0;

	return;
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
