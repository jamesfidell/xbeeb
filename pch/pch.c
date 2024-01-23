#include <stdio.h>
#include <fcntl.h>

main()
{
	unsigned char	x [ 16384 ];
	int				fd;

	fd = open ( "OS1.2p1.rom", O_RDWR );
	read ( fd, x, 16384 );

	/*
	 * The OSFILE trap
	 */

	x [ 0x327d ] = 0xd4;

	/*
	 * The OSFSC trap
	 */

	x [ 0x31b1 ] = 0xd7;

	lseek ( fd, 0, SEEK_SET );
	write ( fd, x, 16384 );
	close ( fd );
}
