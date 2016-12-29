/*
 ============================================================================
 Name        : main.c
 Author      : Dogukan Yigit Polat & Gulsum Gudukbay
 Version     :
 Copyright   : Ours and only ours.
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include <linux/msdos_fs.h>

#define SECTORSIZE 512   //bytes
#define BLOCKSIZE  4096  // bytes - do not change this value

char diskname[48];
int  disk_fd;

unsigned char volumesector[SECTORSIZE];
unsigned char cur_block[BLOCKSIZE];

int get_sector (unsigned char *buf, int snum)
{
	off_t offset;
	int n;
	offset = snum * SECTORSIZE;
	lseek (disk_fd, offset, SEEK_SET);
	n  = read (disk_fd, buf, SECTORSIZE);
	if (n == SECTORSIZE)
		return (0);
	else {
		printf ("sector number %d invalid or read error.\n", snum);
		exit (1);
	}
}

int get_block (unsigned char *buf, int snum)
{
	off_t offset;
	int n;
	offset = snum * BLOCKSIZE;
	lseek (disk_fd, offset, SEEK_SET);
	n  = read (disk_fd, buf, BLOCKSIZE);
	if (n == BLOCKSIZE)
		return (0);
	else {
		printf ("sector number %d invalid or read error.\n", snum);
		exit (1);
	}
}

void print_sector (unsigned char *s)
{
	int i;

	for (i = 0; i < SECTORSIZE; ++i) {
		printf ("%02x ", (unsigned char) s[i]);
		if ((i+1) % 16 == 0)
			printf ("\n");
	}

	printf ("\n");
}

void print_volume_details()
{
	int i, j;

	for (i = 0; i < SECTORSIZE; ++i) {
		printf ("%03x: %02x %c \n", i, volumesector[i], volumesector[i]);

	}

	i = 0;
	printf( "jump instr: %02x\n", volumesector[i]);

	i = 0x3;
	printf( "OEM name: ");
	for( j = i; j < 8 + i; j++)
		printf( "%c", volumesector[j]);
	printf( "\n");

	printf( "\n");
	i = 0xb;
	printf( "BOOT PARAMETER BLOCK \n");

	j = 0xb;
	printf( "Bytes per logical sector: ");
	printf( "%d \n", volumesector[j] + (volumesector[j + 1] << 8));

	j = 0xd;
	printf( "Logical sectors per cluster: ");
	printf( "%d \n", volumesector[j]);

	j = 0xe;
	printf( "Count of reserved logical sectors: ");
	printf( "%d \n", volumesector[j] + (volumesector[j + 1] << 8));

	j = 0x10;
	printf( "# of FATs: ");
	printf( "%d \n", volumesector[j]);

	j = 0x11;
	printf( "max # of FAT12/16 root dir entries: ");
	printf( "%d \n", volumesector[j] + (volumesector[j + 1] << 8));

	j = 0x13;
	printf( "Total logical sectors: ");
	printf( "%d \n", volumesector[j] + (volumesector[j + 1] << 8));

	j = 0x15;
	printf( "Media Descriptor: ");
	printf( "%02x \n", volumesector[j]);

	j = 0x24;
	printf( "Logical sectors per FAT: ");
	printf( "%d \n", *((int*)(&volumesector[j])));

	j = 0x28;
	printf( "Drive description: ");
	printf( "%x \n", volumesector[j] + (volumesector[j + 1] << 8));

	j = 0x2a;
	printf( "Version: ");
	printf( "%d.%d \n", volumesector[j + 1], volumesector[j]);

	j = 0x2c;
	printf( "Cluster number of root dir. starts: ");
	printf( "%d \n", *((int*)(&volumesector[j])));

	j = 0x30;
	printf( "Logical sector number of FS Information Sector: ");
	printf( "%d \n", volumesector[j] + (volumesector[j + 1] << 8));


	j = 0x32;
	printf( "First logical sector number of a copy of the three FAT32 boot sectors: ");
	printf( "%d \n", volumesector[j] + (volumesector[j + 1] << 8));


	printf( "\n");

	i = 0x1fd;
	printf( "Physical drive number : %x\n",  volumesector[j]);

	i = 0x1fe;
	printf( "Boot sector signature: %02x%02x\n", volumesector[i+1],volumesector[i]);


	printf ("\n");
}


void print_block (unsigned char *s)
{
	int i;

	for (i = 0; i < BLOCKSIZE; ++i) {
		printf ("%c ", (unsigned char) s[i]);
		if ((i+1) % 16 == 0)
			printf ("\n");
	}

	printf ("\n");
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf ("wrong usage\n");
		exit (1);
	}

	strcpy (diskname, argv[1]);

        disk_fd = open (diskname, O_RDWR);
	if (disk_fd < 0) {
		printf ("could not open the disk image\n");
		exit (1);
	}

	get_sector (volumesector, 0);
	printf ("volume sector retrieved\n");

	print_sector(volumesector);
	print_volume_details();


	get_block( cur_block,6);
	print_block( cur_block);

	close (disk_fd);
	return (0);
}
