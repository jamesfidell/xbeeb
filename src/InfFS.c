/*
 *
 * $Id: InfFS.c,v 1.3 2002/01/15 15:46:43 james Exp $
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
 * $Log: InfFS.c,v $
 * Revision 1.3  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.2  2002/01/14 23:20:44  james
 * Prevent rolling over 31 disk catalog entries
 *
 * Revision 1.1  2002/01/14 22:18:51  james
 * Added support for .inf files
 *
 *
 */


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<dirent.h>

#include	"Config.h"
#include	"Beeb.h"
#include	"Sheila.h"
#include	"Compat.h"


#ifdef	INF_FS

#include	"EmulFS.h"
#include	"Memory.h"

typedef struct
{
	char					Filename [ 10 ];
	char					RealFilename [ 32 ];	/* FIX ME */
	char					LockFlag;
	unsigned int			LoadAddress;
	unsigned int			ExeAddress;
	unsigned int			FileLength;
	unsigned int			StartSector;
	unsigned int			CRC;
} FileInfo;

struct FileInfoL_t
{
	FileInfo				info;
	struct FileInfoL_t		*prev;
	struct FileInfoL_t		*next;
};
typedef struct FileInfoL_t	FileInfoL;



static int			LoadFile ( char*, int, int, unsigned int* );
static int			SaveFile ( char*, unsigned int, unsigned int,
												unsigned int, unsigned int );
static FileInfoL	*FindFile ( char* );
static void			CopyFilename ( unsigned int, char* );
static int			WriteInfFile ( FileInfoL*, char* );

#ifdef	XDFS

static void			CopyFilenameLib ( unsigned int, char* );
static void			CopyDirname ( unsigned int, char* );

#endif	/* XDFS */


static FileInfoL	*FileInfoHead = 0;
static FileInfoL	*FileInfoTail = 0;

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
 * These are special memory locations that XDFS uses.  They could be
 * changed for a different DFS ROM, though.
 *
 * Change any of these to be more than 0xffff and expect things to
 * fail howwibly, though.
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
#define	XDFS_DATA		0x1001
#define	XDFS_CURR_DIR	0x10ca
#define	XDFS_CURR_LIB	0x10cc

char					DiskName [ MAX_DISKNAME + 1 ];
static int				CatalogWrites;
static int				CatalogSize = 0;

/*
 * For the Random Access File code.
 */

#define	RAF_BUFFERSIZE	0x10000
#define	RAF_READONLY	0x40
#define	RAF_WRITEONLY	0x80
#define	RAF_FILEHANDLE	0x11

static int				RAF_FilesOpen;
static int				RAF_FileType;
static int				RAF_FileLength;
static int				RAF_FilePointer;
char					RAF_Filename [ DFS_FILE_MAX ];
byteval					RAF_Buffer [ RAF_BUFFERSIZE ];

#endif	/* XDFS */


/*
 * These are global just to make passing the values around easier
 */

char			DiskDirectory [ PATH_MAX ];
char			DiskDirSet = 0;

#ifndef	XDFS
char			CurrentDirectory = '$';
#endif

/*
 * Just to make it tidier to increment an address value and make sure
 * it rolls over at 0xffff
 */

#define	Inc(x)	x++; x &= 0xffff


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
			{
				/*
				 * The gospel according to DRS...
				 *
				 * Acorn DFS stuffs the exe address of the loaded file
				 * back into the parameter block here and without it
				 * Imogen won't work...
				 */

				unsigned int		exe;

				( void ) LoadFile ( fname, 0, 0, &exe );
				WriteByte (( param_block + 6 ), ( exe & 0xff ));
				WriteByte (( param_block + 7 ), (( exe >> 8 ) & 0xff ));
				WriteByte (( param_block + 8 ), (( exe >> 16 ) & 0xff ));
				WriteByte (( param_block + 9 ), (( exe >> 24 ) & 0xff ));
			}
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
			FileInfoL			*FileInfoP;

			CopyFilename ( fname_address, fname );
			if (!( FileInfoP = FindFile ( fname )))
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

			FileInfoP->info.LoadAddress = LoadAddress;
			FileInfoP->info.ExeAddress = ExeAddress;
			( void ) WriteInfFile ( FileInfoP, 0 );

			return A;
			break;
		}
#endif	/* XDFS */

		case 0x02 :		/* Write the load address for a file */
#ifdef	XDFS
		{
			FileInfoL			*FileInfoP;

			CopyFilename ( fname_address, fname );
			if (!( FileInfoP = FindFile ( fname )))
			{
				fprintf ( stderr, "File %s not found in catalog\n", fname );
				FatalError();
			}

			LoadAddress = ReadByte ((( param_block + 2 ) & 0xffff )) +
						( ReadByte ((( param_block + 3 ) & 0xffff )) << 8 ) +
						( ReadByte ((( param_block + 4 ) & 0xffff )) << 16 ) +
						( ReadByte ((( param_block + 5 ) & 0xffff )) << 24 );

			FileInfoP->info.LoadAddress = LoadAddress;
			( void ) WriteInfFile ( FileInfoP, 0 );

			return A;
			break;
		}
#endif	/* XDFS */

		case 0x03 :		/* Write the execution address for a file */
#ifdef	XDFS
		{
			FileInfoL			*FileInfoP;

			CopyFilename ( fname_address, fname );
			if (!( FileInfoP = FindFile ( fname )))
			{
				fprintf ( stderr, "File %s not found in catalog\n", fname );
				FatalError();
			}

			ExeAddress = ReadByte ((( param_block + 6 ) & 0xffff )) +
						( ReadByte ((( param_block + 7 ) & 0xffff )) << 8 ) +
						( ReadByte ((( param_block + 8 ) & 0xffff )) << 16 ) +
						( ReadByte ((( param_block + 9 ) & 0xffff )) << 24 );

			FileInfoP->info.ExeAddress = ExeAddress;
			( void ) WriteInfFile ( FileInfoP, 0 );

			return A;
			break;
		}
#endif	/* XDFS */

		case 0x04 :		/* Set/clear the lock flag for a file */
#ifdef	XDFS
		{
			FileInfoL			*FileInfoP;
			unsigned char		attributes;

			CopyFilename ( fname_address, fname );
			attributes = ReadByte ((( param_block + 14 ) & 0xffff ));

			if (!( FileInfoP = FindFile ( fname )))
			{
				fprintf ( stderr, "File %s not found in catalog\n", fname );
				FatalError();
			}

			FileInfoP->info.LockFlag =
				(( attributes & 0x0a ) == 0x0a ) ? 'L' : ' ';
			( void ) WriteInfFile ( FileInfoP, 0 );

			return A;
			break;
		}
#endif	/* XDFS */

		case 0x05 :		/* Read a file's catalog information */
#ifdef	XDFS
		{
			FileInfoL			*FileInfoP;
			unsigned char		byte;
			unsigned int		p = ( param_block + 2 ) & 0xffff;

			CopyFilename ( fname_address, fname );
			if (!( FileInfoP = FindFile ( fname )))
			{
				fprintf ( stderr, "File %s not found in catalog\n", fname );
				FatalError();
			}

			WriteByte ( p, FileInfoP->info.LoadAddress & 0xff );
			Inc ( p );	/* p = param_block + 3 */
			WriteByte ( p, ( FileInfoP->info.LoadAddress >> 8 ) & 0xff );
			Inc ( p );	/* p = param_block + 4 */

			/*
			 * Take care of TUBE/local adddresses
			 */

			byte = ( FileInfoP->info.LoadAddress >> 16 ) & 0xff;
			WriteByte ( p, byte );
			Inc ( p );	/* p = param_block + 5 */
			if ( byte != 0xff )
				byte = 0;
			WriteByte ( p, byte );
			Inc ( p );	/* p = param_block + 6 */

			WriteByte ( p, FileInfoP->info.ExeAddress & 0xff );
			Inc ( p );	/* p = param_block + 7 */
			WriteByte ( p, ( FileInfoP->info.ExeAddress >> 8 ) & 0xff );
			Inc ( p );	/* p = param_block + 8 */

			/*
			 * Take care of TUBE/local adddresses
			 */

			byte = ( FileInfoP->info.ExeAddress >> 16 ) & 0xff;
			WriteByte ( p, byte );
			Inc ( p );	/* p = param_block + 9 */
			if ( byte != 0xff )
				byte = 0;
			WriteByte ( param_block + 9, byte );
			Inc ( p );	/* p = param_block + 10 */

			WriteByte ( p, FileInfoP->info.FileLength & 0xff );
			Inc ( p );	/* p = param_block + 11 */
			WriteByte ( p, ( FileInfoP->info.FileLength >> 8 ) & 0xff );
			Inc ( p );	/* p = param_block + 12 */
			WriteByte ( p, ( FileInfoP->info.FileLength >> 16 ) & 0xff );
			Inc ( p );	/* p = param_block + 13 */
			WriteByte ( p, FileInfoP->info.FileLength >> 24 );
			Inc ( p );	/* p = param_block + 14 */
			WriteByte ( p, ( FileInfoP->info.LockFlag == 'L' ) ? 0x0a : 0x0 );
			return A;
			break;
		}
#endif	/* XDFS */

		case 0x06 :		/* Delete a file */
#ifdef	XDFS
		{
			FileInfoL			*FileInfoP;
			char				delfile [ PATH_MAX ];

			CopyFilename ( fname_address, fname );
			if (!( FileInfoP = FindFile ( fname )))
			{
				fprintf ( stderr, "File %s not found in catalog\n", fname );
				FatalError();
			}

			( void ) strcpy ( delfile, DiskDirectory );
			( void ) strcat ( delfile, "/" );
			( void ) strcat ( delfile, ( FileInfoP->info.RealFilename ));

			unlink ( delfile );
			( void ) strcat ( delfile, ".inf" );
			unlink ( delfile );

			if ( FileInfoP->prev )
				FileInfoP->prev->next = FileInfoP->next;
			else
				FileInfoHead = FileInfoP->next;
			if ( FileInfoP->next )
				FileInfoP->next->prev = FileInfoP->prev;
			else
				FileInfoTail = FileInfoP->prev;
			free ( FileInfoP );

			if (( ++CatalogWrites & 0x0f ) > 9 ) {
				CatalogWrites &= 0xf0;
				CatalogWrites += 0x10;

				if ( CatalogWrites > 0x99 )
					CatalogWrites = 0;
			}
			CatalogSize--;

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
			char			fname [ DFS_FILE_MAX ], fname2 [ DFS_FILE_MAX ];
			FileInfoL		*FileInfoP;

			CopyFilename ( fname_address, fname );
			CopyFilename (( ReadWord ( param_block+2 )), fname2 );

			if ( FindFile ( fname2 ))
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

			if (!( FileInfoP = FindFile ( fname )))
			{
				/*
				 * Oh dear...
				 * The file to rename is not in the InfFS catalog. XDFS
				 * should have dealt with this.
				 */

				fprintf ( stderr,"File %s not in InfFS catalogue, ",fname);
				fprintf ( stderr,"even though XDFS thinks it is.\n" );
				FatalError();
			}

			/*
			 * Clear the XDFS status byte
			 */
		
			WriteByte ( XDFS_STATUS, 0x0 );
			( void ) strcpy ( FileInfoP->info.Filename, fname2 );
			( void ) WriteInfFile ( FileInfoP, 0 );
			return A;
			break;
		}

		case 0x0a :		/* OSFIND */
		{
			int					file_fd, i;
			char				fullname [ PATH_MAX ];
			unsigned char		filebyte;
			FileInfoL			*FileInfoP;

			/*
			 * Bits 6 and 7 of the Accumulator determine the action we are to
			 * take. XDFS stores the value of the accumulator in its status
			 * byte.
			 *
			 */

			RAF_FileType = (ReadByte ( XDFS_STATUS )) & 0xc0;

			/*
			 * XDFS looks for any error in the status byte, and any value is
			 * returned in the data byte. We must clear both of those now.
			 *
			 */

			WriteByte ( XDFS_STATUS, 0 );
			WriteByte ( XDFS_DATA, 0 );

			/*
			 * If the file type is zero, then we may need to close the file.
			 *
			 */

			if ( RAF_FileType == 0 )
			{
				/*
				 * FIX ME
				 *
				 * If the file is opened for writing, we should write the file
				 * out to disc now.
				 *
				 */

				/*
				 * Since we only have one channel at the moment, we can just
				 * flag no files open.
				 *
				 */

				RAF_FilesOpen = 0;
			}
			else
			{
				/*
				 * If we have a file open already, we cater for that here.
				 * Again, a "too many open" message is passed back to XDFS.
				 *
				 */


				/*
				 * FIX ME
				 *
				 * Tynesoft game loaders like to open random access files and
				 * then just not close them again. This is very bad programming,
				 * but we should attempt to support this. The proper code
				 * should be:
				 *
				 * if ( RAF_FilesOpen )
				 * {
				 *     WriteByte ( XDFS_STATUS, 0x22 );
				 *     return A;
				 * }
				 *
				 * but I'm just going to leave it for now. The old random
				 * access file will be closed and the new one opened. Things
				 * will be better when XDFS supports more than one random
				 * access file.
				 */

				/*
				 * If a directory is not currently set then sort that out now.
				 *
				 */

				while ( !DiskDirSet )
				{
					printf ( "No disk directory is currently set\n" );
					printf ( "Set the directory now...\n" );
					ChangeDisk();
				}

				/*
				 * X and Y point to the filename. We must check that the file
				 * is in the catalog, and if not return 0.
				 *
				 */

				fname_address = ( Y << 8 ) + X;
				CopyFilename ( fname_address, fname );

				if ( RAF_FileType & RAF_WRITEONLY )
				{
					/*
					 * This is unsupported, so die now.
					 *
					 */

					fprintf ( stderr, "Cannot open files to write yet.\n" );
					return A;
				}

				if ( RAF_FileType & RAF_READONLY )
				{
					if (!( FileInfoP = FindFile ( fname )))
						return 0;

					/*
					 * We copy the filename in the catalog to the file path,
					 * not the name we are called with, because this gurantees
					 * that we get the correct case.
					 *
					 */

					( void ) strcpy ( fullname, DiskDirectory );

					if (( strlen ( fullname ) + strlen ( fname ) + 2 )
																> PATH_MAX )
					{
						fprintf ( stderr, "pathname is too long\n" );
						fprintf (stderr, "InfFS can't generate correct "
							"error\n");
						return 0;
					}

					( void ) strcat ( fullname, "/" );
					( void ) strcat ( fullname, FileInfoP->info.RealFilename );

					/*
					 * If we can't find the file in the Unix directory, again
					 * all we have to do is return 0 - not found.
					 *
					 */

					if (( file_fd = open ( fullname, O_RDONLY | O_BINARY ))
																		< 0 )
						return 0;

					/*
					 * Now to set up the file length and pointer.
					 *
					 */

					RAF_FileLength = FileInfoP->info.FileLength;
					RAF_FilePointer = 0;

					/*
					 * If the file we are trying to open is longer than our
					 * buffer, then we're in trouble...
					 *
					 */

					if ( RAF_FileLength >= RAF_BUFFERSIZE )
					{
						RAF_FileLength = RAF_BUFFERSIZE - 1;
						fprintf ( stderr, "File is larger than buffer size.");
						fprintf ( stderr, "\nYou can only read the first 0x%x",
															RAF_FileLength );
						fprintf ( stderr, " bytes.\n" );
					}

					/*
					 * If we are required to read the file, we should do that
					 * now. I'll read the data into the buffer one byte at a
					 * time so that the buffer does not overflow.
					 *
					 */

					for ( i = 0; i < RAF_FileLength; i++ )
					{
						if ( read ( file_fd, &filebyte, 1 ) != 1 )
						{
							fprintf ( stderr, "Read of file %s failed\n",
																	fname );
							FatalError();
						}

						RAF_Buffer [ i ] = filebyte;
					}

					/*
					 * All to do now is to pass back the file handle in the
					 * data byte and flag that we have a file open.
					 *
					 */

					WriteByte ( XDFS_DATA, RAF_FILEHANDLE );
					RAF_FilesOpen = 1;
				}
			}

			return A;
			break;
		}

		case 0x0c :		/* BGET */
		{
			/*
			 * If Y does not equal the file handle, then we return a "Channel"
			 * error via the XDFS status byte. Also need to generate an error
			 * if we have no files open.
			 *
			 */

			if (( Y != RAF_FILEHANDLE ) || ( !RAF_FilesOpen ))
			{
				WriteByte ( XDFS_STATUS, 0x33 );
				return A;
			}

			/*
			 * We need to see if we have gone passed the end of the file. If
			 * so we set a flag in the status byte. The value we pass back
			 * is invalid.
			 *
			 */

			if ( RAF_FilePointer >= RAF_FileLength )
				WriteByte ( XDFS_STATUS, 0xff );
			else
			{
				/*
				 * Write the data to the data byte, clear the status byte and
				 * increment the pointer.
				 *
				 */

				WriteByte ( XDFS_DATA, RAF_Buffer [ RAF_FilePointer ] );
				WriteByte ( XDFS_STATUS, 0 );
				RAF_FilePointer ++;
			}

			return A;
			break;
		}

		case 0x0d :		/* OSARGS */
		{

			byteval		file_action;

			/*
			 * If Y is 0 then we have a problem, since XDFS should
			 * have dealt with this.
			 *
			 */

			if ( Y == 0 )
			{
				fprintf ( stderr, "OSARGS call with Y = 0\n");
				FatalError ();
			}

			/*
			 * If Y does not contain the file handle, then issue  a "Channel"
			 * error via the XDFS status byte. Also generate an error if we
			 * have no file open.
			 *
			 */

			if (( Y != RAF_FILEHANDLE ) || ( !RAF_FilesOpen ))
			{
				WriteByte ( XDFS_STATUS, 0x33 );
				return A;
			}

			/*
			 * XDFS passes the accumulator value in the status byte.
			 * This value determines the action we take
			 *
			 */

			file_action = ReadByte ( XDFS_STATUS );
			WriteByte ( XDFS_STATUS, 0 );

			if ( file_action == 0 ) 	/* read pointer */
			{
				WriteByte ( X, RAF_FilePointer & 0xff );
				WriteByte (( X + 1 ), ( RAF_FilePointer & 0xff00 ) >> 8 );
				WriteByte (( X + 2 ), ( RAF_FilePointer & 0xff0000 ) >> 16 );
				WriteByte (( X + 3 ), ( RAF_FilePointer & 0xff000000 ) >> 24 );
				return A;
			}

			if ( file_action == 1 )		/* write pointer */
			{
				RAF_FilePointer  = ReadByte ( X + 3 ); RAF_FilePointer <<= 8;
				RAF_FilePointer |= ReadByte ( X + 2 ); RAF_FilePointer <<= 8;
				RAF_FilePointer |= ReadByte ( X + 1 ); RAF_FilePointer <<= 8;
				RAF_FilePointer |= ReadByte ( X );

				/*
				 * If we are only reading the file and the new value of the
				 * pointer lies outside the file, we inform XDFS
				 *
				 */

				if (( RAF_FileType == RAF_READONLY ) && ( RAF_FilePointer >= RAF_FileLength ))
					WriteByte ( XDFS_STATUS, 0x55 );

				return A;
			}

			if ( file_action == 2 )		/* read file length */
			{
				WriteByte ( X, RAF_FileLength & 0xff );
				WriteByte (( X + 1 ), ( RAF_FileLength & 0xff00 ) >> 8 );
				WriteByte (( X + 2 ), ( RAF_FileLength & 0xff0000 ) >> 16 );
				WriteByte (( X + 3 ), ( RAF_FileLength & 0xff000000 ) >> 24 );
				return A;
			}

			fprintf ( stderr, "Unimplemented OSARGS A = %x\n", file_action );
			FatalError ();
			break;
		}
#endif	/* XDFS */

		case 0x07 :		/* Prepare an area of disc space -- Master only */
		default :
			fprintf ( stderr, "Unimplemented InfFS_OSFILE (A = 0x%02x)\n", A );
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
			/*
			 * FIX ME
			 *
			 * These aren't meaningful to implement as things stand --
			 * virtual tapes and disks working directly with the hardware
			 * emulation will make them much more sensible.
			 * 
			 */

			fprintf ( stderr, "InfFS_OSFSC ignored *OPT %d,%d\n", X, Y );
			break;

		case 0x01 :		/* check EOF */
		{
			/*
			 * FIX ME
			 *
			 * Something do be done when data files are properly emulated.
			 */
			fprintf ( stderr, "Unimplemented InfFS_OSFSC (A = 0x%02x)\n", A );
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
			FileInfoL		*FileInfoP;

			fname_address = ( Y << 8 ) + X;
			CopyFilename ( fname_address, fname );

			/*
			 * The file could be in the currently selected directory
			 * or it could be in the library directory. We check the
			 * currently selected directory first.
			 */

			if (!( FileInfoP = FindFile ( fname )))
			{

				/*
				 * The file wasn't in the currently selected
				 * directory so we look in the library.
				 */

				CopyFilenameLib ( fname_address, fname );
				if (!( FileInfoP = FindFile ( fname )))
				{

					/*
					 * If we get here then something has gone horribly
					 * wrong. XDFS should be checking that the file
					 * exists in the directory, so the next couple of
					 * lines should never be exectued.
					 */

					fprintf ( stderr, "File %s not found in InfFS directory\n",
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
			int					j, off, fptr;
			char				byte;
			FileInfoL			*FileInfoP;
			unsigned int		BootOption;

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
				WriteByte (( XDFS_CATWS1 + i ) & 0xffff, ' ' );
				WriteByte (( XDFS_CATWS2 + i ) & 0xffff, 0x00 );
			}

			/*
			 * Copy the catalog (inc. addresses) into pages &E and &F.
			 */

			FileInfoP = FileInfoHead;
			i = 0;
			while ( FileInfoP )
			{
				off = i * 8;
				fptr = XDFS_FNAMEBASE + off;
				for ( j = 2; j < 10; j++ )
				{
					if (!( byte = FileInfoP->info.Filename [ j ] ))
						break;
					WriteByte ( fptr, byte );
					Inc ( fptr );
				}

				byte = *(FileInfoP->info.Filename);		/* file directory */
				byte |= ( FileInfoP->info.LockFlag == 'L' ) ? 0x80 : 0x0;
				WriteByte (( XDFS_DIRBASE + off ) & 0xffff, byte );

				fptr = ( XDFS_ADDRBASE + off ) & 0xffff;
				WriteByte ( fptr, FileInfoP->info.LoadAddress & 0xff );
				Inc ( fptr );
				WriteByte ( fptr, ( FileInfoP->info.LoadAddress >> 8 ) & 0xff );
				Inc ( fptr );

				WriteByte ( fptr, FileInfoP->info.ExeAddress );
				Inc ( fptr );
				WriteByte ( fptr, ( FileInfoP->info.ExeAddress >> 8 ) & 0xff );
				Inc ( fptr );

				WriteByte ( fptr, FileInfoP->info.FileLength );
				Inc ( fptr );
				WriteByte ( fptr, ( FileInfoP->info.FileLength >> 8 ) & 0xff );
				Inc ( fptr );

				WriteByte (( XDFS_SECTBASE + off ) & 0xffff,
							FileInfoP->info.StartSector & 0xff );
				Inc ( fptr );

				byte = ( FileInfoP->info.ExeAddress & 0x30000 ) >> 10;
				byte &= ( FileInfoP->info.FileLength & 0x30000 ) >> 12;
				byte &= ( FileInfoP->info.LoadAddress & 0x30000 ) >> 14;
				byte &= ( FileInfoP->info.StartSector & 0x300 ) >> 8;

				WriteByte ( fptr, byte );

				FileInfoP = FileInfoP->next;
				i++;
			}
			CatalogSize = i;
			BootOption = 0;

			/*
			 * Clear title memory.  Don't have to clear what is in page 0xf00
			 * because we did that earlier on.
			 */

			for ( i = 0; i < 8; i++)
				WriteByte (( XDFS_CATWS1 + i ) & 0xffff, 0x0 );

			/* Put new disc title in memory */

			for ( i = 0; i < 8; i++ )
				WriteByte (( XDFS_CATWS1 + i ) & 0xffff, DiskName [ i ] );

			for ( i = 8; i < 12; i++ )
				WriteByte (( XDFS_CATWS2 + i - 8 ) & 0xffff, DiskName [ i ] );

			WriteByte ( XDFS_CATWR, CatalogWrites );	/* times written */
			WriteByte ( XDFS_NOFILES, CatalogSize * 8 );	/* no. of files */
			WriteByte ( XDFS_OPTION, ( BootOption & 0x3 ) << 4 ); /* option */
			WriteByte ( XDFS_NOSECTS, 0);	/* sectors have no meaning */

#else	/* XDFS */

			printf ( "*CAT is not fully implemented -- \n\n" );

			while ( !DiskDirSet )
				ChangeDisk();

			FileInfoP = FileInfoHead;
			while ( FileInfoP )
			{
				printf ( "%-10s load = %06X, exe = %06X, length =%06X\n",
					FileInfoP->info.Filename, FileInfoP->info.LoadAddress,
					FileInfoP->info.ExeAddress, FileInfoP->info.FileLength );
				i++;
				FileInfoP = FileInfoP->next;
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
			printf ( "InfFS_OSFSC (A = 0x%02x) ignored\n", A );
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

			/*
			 * For XDFS there's nothing to do here as the ROM handles it all
			 */

#ifdef	XDFS
#ifdef	INFO
			printf ( "InfFS_OSFSC (A = 0x%02x)\n", A );
#endif
#else
			fprintf ( stderr, "Unimplemented InfFS_OSFSC (A = 0x%02x)\n", A );
#endif
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
			fprintf ( stderr, "Unimplemented InfFS_OSFSC (A = 0x%02x)\n", A );
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
	unsigned char	filebyte;
	int				file_fd;
	unsigned int	len;
	FileInfoL		*FileInfoP;

	while ( !DiskDirSet )
	{
		printf ( "No disk directory is currently set\n" );
		printf ( "Set the directory now...\n" );
		ChangeDisk();
	}

	if (!( FileInfoP = FindFile ( fname )))
	{
		printf ( "File %s not found in catalog\n", fname );
		printf ( "InfFS cannot generate correct error\n" );
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
		fprintf ( stderr, "InfFS cannot generate correct error\n" );
		return -1;
	}
	( void ) strcat ( fullname, "/" );
	( void ) strcat ( fullname, FileInfoP->info.RealFilename );

	if (( file_fd = open ( fullname, O_RDONLY | O_BINARY )) < 0 )
	{
		fprintf ( stderr, "File %s found in InfFS catalog, ", fname );
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
		address = FileInfoP->info.LoadAddress & 0xffff;

	/*
	 * The file is read in one byte at a time so that the data can
	 * be written into memory using WriteByte, which will deal with
	 * any nasty little traps such as writing into ROM etc.
	 */

	for ( len = FileInfoP->info.FileLength; len > 0; len-- )
	{
		if ( read ( file_fd, &filebyte, 1 ) != 1 )
		{
			fprintf ( stderr, "Read of file %s failed\n", fname );
			FatalError();
		}

		WriteByte ( address, filebyte );
		Inc ( address );
	}

	( void ) close ( file_fd );

	if ( pExe )
		*pExe = FileInfoP->info.ExeAddress;
	return 0;
}


static FileInfoL*
FindFile ( char *file )
{
	FileInfoL		*FileInfoP;
	char			ff[10];

	while ( !DiskDirSet )
	{
		printf ( "No disk directory is currently set\n" );
		printf ( "Set the directory now...\n" );
		ChangeDisk();
	}

	memset ( ff, ' ', 9 );
	ff[9] = 0;
	strncpy ( ff, file, strlen ( file ));
	FileInfoP = FileInfoHead;
	while ( FileInfoP )
	{
		if (!strcasecmp ( FileInfoP->info.Filename, ff ))
			break;
		FileInfoP = FileInfoP->next;
	}
	return FileInfoP;
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
	char			buff [ 80 ], beebfile[32];	/* FIX ME */
	char			cwd [ PATH_MAX+1];
	FILE			*cat_fp;
	FileInfoL		*p, *q;
	char			*lock, *n;
	DIR				*dirp;
	struct dirent	*entry;
	unsigned int	l;
	struct stat		sb;

	if (!( dirp = opendir ( new ))) {
		fprintf ( stderr, "can't open directory\n" );
		return -1;
	}
	( void ) getcwd ( cwd, PATH_MAX );
	chdir ( new );

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
	if (( p = FileInfoHead )) {
		while ( p ) {
			q = p->next;
			free ( p );
			p = q;
		}
	}
	FileInfoHead = FileInfoTail = 0;
	CatalogSize = 0;

	while (( entry = readdir ( dirp )) && CatalogSize < CATALOG_SIZE ) {
		l = strlen ( entry->d_name );
		if ( l > 4 && !strcmp ( entry->d_name + l - 4, ".inf" )) {
			( void ) strcpy ( beebfile, entry->d_name );
			beebfile[l-4] = 0;
			/*
			 * We're just going to ignore any errors here -- we'll assume
			 * it means that the .inf file doesn't have a corresponding
			 * BBC file
			 */
			cat_fp = 0;
			if ( !access ( beebfile, R_OK ) &&
						( cat_fp = fopen ( entry->d_name, "r" ))) {
				if ( fgets ( buff, 79, cat_fp ) > 0 ) {
					/*
					 * FIX ME
					 * Check for errors from malloc and sscanf
					 */
					p = malloc ( sizeof ( FileInfoL ));
					sscanf ( buff, "%*s %X %X %c",
						&(p->info.LoadAddress), &(p->info.ExeAddress), &lock );
					p->info.StartSector = 0;
					stat ( beebfile, &sb );
					p->info.FileLength = sb.st_size;
					if ( lock ) {
#ifdef	XDFS
						if ( !strncmp ( lock, "Locked", 6 )) {
							p->info.LockFlag = 'L';
						}
#endif
						free ( lock );
					}
					/*
					 * Filenames are right-padded with spaces initially.
					 */
					( void ) strncpy ( p->info.Filename, buff, 9 );
					p->info.Filename [ 9 ] = '\0';
					/*
					 * FIX ME
					 * check for overflow?
					 */
					( void ) strcpy ( p->info.RealFilename, beebfile );
					p->next = 0;
					if ( FileInfoTail ) {
						FileInfoTail->next = p;
						p->prev = FileInfoTail;
						FileInfoTail = p;
					} else {
						FileInfoHead = FileInfoTail = p;
						p->prev = 0;
					}
				} else {
					fprintf ( stderr, "can't read from %s\n", entry->d_name );
				}
				if ( cat_fp )
					( void ) fclose ( cat_fp );
			}
		}
	}
	chdir ( cwd );
	return 0;
}


static int
SaveFile ( char *fname, unsigned int load, unsigned int exe,
									unsigned int start, unsigned int end )
{
	int				fd,	size, q;
	int				l, l1, l2, overwrite;
	char			tempfile [ PATH_MAX ], tempinf [ PATH_MAX ];
	char			newfile [ PATH_MAX ];
	char			c, realfname [ 20 ];
	FileInfoL		*p;

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
		fprintf ( stderr, "InfFS cannot generate correct error\n" );
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
		fprintf ( stderr, "InfFS cannot generate correct error\n" );
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
		fprintf ( stderr, "InfFS cannot generate correct error\n" );
		return -1;
	}

	/*
	 * Write the data out as a temporary file
	 */

	q = start;
	while ( --size >= 0 )
	{
		c = ReadByte ( q );
		if ( write ( fd, &c, 1 ) < 1 )
		{
			fprintf ( stderr, "write error on %s\n", tempfile );
			fprintf ( stderr, "InfFS cannot generate correct error\n" );
			close ( fd );
			unlink ( tempfile );
			return -1;
		}
		q++;
		q &= 0xffff;
	}
	close ( fd );

	/*
	 * If the file appears in the catalog, we'll need to overwrite the
	 * entry.  Otherwise we create a new one.
	 */

	if (( p = FileInfoHead )) {
		while ( p ) {
			if ( !strcasecmp ( p->info.Filename, fname ))
				break;
			p = p->next;
		}
	}

	/*
	 * If we haven't found one, add a new catalog entry unless the catalog
	 * is full, in which case return an error.  If we have found one, write
	 * over the old catalog entry.
	 */

	if ( !p ) {
		if ( CatalogSize == CATALOG_SIZE ) {
			fprintf ( stderr, "InfFS catalog is full\n" );
			fprintf ( stderr, "InfFS cannot return correct error\n" );
			return -1;
		}
		/*
		 * FIX ME
		 * check for errors from malloc
		 */
		p = malloc ( sizeof ( FileInfoL ));
		overwrite = 0;
	}
	else {
		( void ) strcpy ( realfname, p->info.Filename );
		overwrite = 1;
	}

	/*
	 * Set start sector to junk, because we don't have anything useful
	 * there.
	 */

	( void ) strcpy ( p->info.Filename, fname );
#ifdef	XDFS
	p->info.LockFlag = ' ';
#endif
	p->info.LoadAddress = load;
	p->info.ExeAddress = exe;
	p->info.FileLength = end - start;
	p->info.StartSector = 0x3ff;
	( void ) strcpy ( p->info.RealFilename, p->info.Filename + 2 );

	/*
	 * Now write the new catalog.  If an error occurs, we abort.
	 */

	/*
	 * FIX ME
	 *
	 * Should check for filename overflow here.
	 */

	( void ) strcpy ( tempinf, DiskDirectory );
	( void ) strcat ( tempinf, "/" );
	( void ) strcat ( tempinf, TMP_INF );

	if ( WriteInfFile ( p, tempinf ) < 0 )
	{
		fprintf (stderr, "Error in writing .inf file. Unlinking %s\n",
																tempfile );
		unlink ( tempfile );
		unlink ( tempinf );
		return -1;
	}

	if ( overwrite )
	{
		unlink ( realfname );
		( void ) strcat ( realfname, ".inf" );
		unlink ( realfname );
	}

	/*
	 * FIX ME
	 *
	 * Since we've just delete the old file, there should be no problems
	 * renaming the others, so I won't check for errors again.
	 *
	 * It is however possible that errors could occur at this stage if
	 * someone's messing around with things behind our backs.
	 */

	( void ) strcpy ( newfile, DiskDirectory );
	( void ) strcat ( newfile, "/" );
	( void ) strcat ( newfile, p->info.RealFilename );
	rename ( tempfile, newfile );
	( void ) strcat ( newfile, ".inf" );
	rename ( tempinf, newfile );
	p->next = 0;
	if ( FileInfoTail ) {
		FileInfoTail->next = p;
		p->prev = FileInfoTail;
		FileInfoTail = p;
	} else {
		FileInfoHead = FileInfoTail = p;
		p->prev = 0;
	}
	return 0;
}


static int
WriteInfFile ( FileInfoL *FileInfoP, char* tempinf )
{
	char		inf [ PATH_MAX ];
	FILE		*fp;

	/*
	 * Update the number of times the catalogue has been written,
	 * wrapping around at 99.  XDFS uses BCD, so we need to fix that
	 * up here, too.
	 */

	if (( ++CatalogWrites & 0x0f ) > 9 )
	{
		CatalogWrites &= 0xf0;
		CatalogWrites += 0x10;

		if ( CatalogWrites > 0x99 )
			CatalogWrites = 0;
	}

	if ( tempinf )
		/*
		 * FIX ME
		 *
		 * Should check for filename overflow here.
		 */

		( void ) strcpy ( inf, tempinf );
	else
	{
		/*
		 * FIX ME
		 *
		 * Should check for filename overflow here.
		 */

		( void ) strcpy ( inf, DiskDirectory );
		( void ) strcat ( inf, "/" );
		( void ) strcat ( inf, TMP_INF );
	}

	if (( fp = fopen ( inf, "w" )) == 0 )
	{
		fprintf ( stderr, "can't open catalog file %s for writing\n", inf );
		fprintf ( stderr, "InfFS cannot generate correct error\n" );
		return -1;
	}

#ifdef	XDFS
	fprintf ( fp, "%-9s %6.6X %6.6X %s\n", FileInfoP->info.Filename,
		FileInfoP->info.LoadAddress, FileInfoP->info.ExeAddress,
		FileInfoP->info.LockFlag == 'L' ? " Locked" : "" );
#else
	fprintf ( fp, "%-9s %6.6X %6.6X\n", FileInfoP->info.Filename,
		FileInfoP->info.LoadAddress, FileInfoP->info.ExeAddress );
#endif	/* XDFS */

	fclose ( fp );

	if ( !tempinf )
	{
		char	newfile [ PATH_MAX ];
		/*
		 * FIX ME
		 *
		 * Should check for overflow here.
		 */

		( void ) strcpy ( newfile, DiskDirectory );
		( void ) strcat ( newfile, "/" );
		( void ) strcat ( newfile, FileInfoP->info.RealFilename );
		( void ) strcat ( newfile, ".inf" );
		rename ( inf, newfile );
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

#endif	/* INF_FS */
