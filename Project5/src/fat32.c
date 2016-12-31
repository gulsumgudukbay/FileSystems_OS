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
		printf ("block number %d invalid or read error.\n", snum);
		exit (1);
	}
}

int write_block (unsigned char *buf, int snum)
{
	off_t offset;
	int n;
	offset = snum * BLOCKSIZE;
	lseek (disk_fd, offset, SEEK_SET);
	n  = write (disk_fd, buf, BLOCKSIZE);
	if (n == BLOCKSIZE)
		return (0);
	else {
		printf ("block number %d invalid or read error.\n", snum);
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

	get_sector (volumesector, 0);
	printf ("volume sector retrieved\n");

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

void print_root_dir()
{
	int i, j;
	int size;
	int blocks;
	int start;
	unsigned char c;

	get_block( cur_block,6);

	i = 0;
	while( c = cur_block[i] )
	{
		if( c == 0xe5)
		{
			i += 32;
			continue;
		}

		printf("filename: ");
		for( j = i; j < 11 + i; j++)
		{
			printf("%c", cur_block[j]);
		}
		size =  *((int*)(&cur_block[i+0x1c]));
		blocks = *((int*)(&cur_block[i+0x1c])) / 4096 + (*((int*)(&cur_block[i+0x1c])) % 4096 ? 1 : 0);
		start = cur_block[i+0x1a] + (cur_block[i+0x14] << 16);
		printf("\t size: %6d (%2d blocks)", size, blocks);
		printf("\t starts at block: %d", start);
		printf("\n");
		i += 32;
	}

}

void print_blocks_of( char* filename)
{
	int i, j, k;
	int size;
	int blocks;
	int start;
	int filefound;
	char c;
	char carr[10];

	get_block( cur_block,6);

	i = 0;
	filefound = 0;
	while( c = cur_block[i] )
	{
		k = 0;
		for( j = i; cur_block[j] != 0x20 && cur_block[j]; j++)
		{
			carr[k++] = cur_block[j];
		}
		carr[k++] = '.';
		for( j = i + 8; j < 11 + i; j++)
		{
			carr[k++] = cur_block[j];
		}
		carr[k] = 0;

		if( strcmp( carr, filename) == 0)
		{
			filefound = ~filefound;
			printf( "\nmatched file:\n");
			printf( "%s", carr);
			size =  *((int*)(&cur_block[i+0x1c]));
			blocks = size / 4096 + (size % 4096 ? 1 : 0);
			start = cur_block[i+0x1a] + (cur_block[i+0x14] << 16);
			printf("\t size: %6d (%2d blocks)", size, blocks);
			printf("\t starts at block: %d", start);
			printf("\n");
		}
		i += 32;
	}

	if(!filefound)
	{
		printf("\nfile not found \n");
	}
	else
	{
		int next;

		get_block(cur_block, 4);
		printf( "blocks: \n");

		next = start;
		i = 0;
		while( next != 0x0fffffff)
		{
			if( i >= size)
			{
				if( !i)
				{
					printf("empty file\n");
					return;
				}

				printf("corrupt file\n");
				return;
			}

			printf("%d: %d\n", i++, next);
			next = *((int*)(&cur_block[next << 2]));
		}
	}
}
///////////////////////////////////////////////////////////////////////

void delete_block(char *filename){
	int i, j, k;
	int size;
	int blocks;
	int start;
	int filefound;
	char c;
	char carr[10];

	get_block( cur_block,6);

	i = 0;
	filefound = 0;
	while( c = cur_block[i] )
	{
		k = 0;
		for( j = i; cur_block[j] != 0x20 && cur_block[j]; j++)
		{
			carr[k++] = cur_block[j];
		}
		carr[k++] = '.';
		for( j = i + 8; j < 11 + i; j++)
		{
			carr[k++] = cur_block[j];
		}
		carr[k] = 0;

		if( strcmp( carr, filename) == 0)
		{
			filefound = ~filefound;
			printf( "\nfile to be deleted:\n");
			printf( "%s", carr);
			size =  *((int*)(&cur_block[i+0x1c]));
			blocks = size / 4096 + (size % 4096 ? 1 : 0);
			start = cur_block[i+0x1a] + (cur_block[i+0x14] << 16);
			printf("\t size: %6d (%2d blocks)", size, blocks);
			printf("\t starts at block: %d", start);
			printf("\n");
			cur_block[i] = 0xE5;
			write_block( cur_block, 6);
		}
		i += 32;
	}

	if(!filefound)
	{
		printf("\nfile not found \n");
	}
	else
	{
		int next, prev;

		get_block(cur_block, 4);
		printf( "blocks to be deleted: \n");

		next = start;
		i = 0;
		while( next != 0x0fffffff)
		{
			if( i >= blocks)
			{
				if( !i)
				{
					*((int*)(&cur_block[start << 2])) = 0;
					return;
				}
				printf("corrupt file\n");
				return;
			}

			printf("%d: %d\n", i++, next);
			prev = next;
			next = *((int*)(&cur_block[next << 2]));
			*((int*)(&cur_block[prev << 2])) = 0;
		}

		write_block( cur_block, 4);
	}
}

void print_block (unsigned char *s)
{
	int i;


	for( i = 0; i < 32; i++)
	{
		printf ("%02d ", i);
	}
	printf("\n");
	for( i = 0; i < 32; i++)
	{
		if( i % 4  == 3)
			printf ("  *");
		else
			printf ("  |");
	}
	printf("\n");

	for (i = 0; i < BLOCKSIZE; ++i) {
		printf ("%02x ", (unsigned char) s[i]);
		if ((i+1) % 32 == 0)
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

	if( strcmp(argv[2], "-p") == 0)
	{
		if( strcmp(argv[3], "volumeinfo") == 0)
		{
			print_volume_details();
		}

		if( strcmp(argv[3], "rootdir") == 0)
		{
			print_root_dir();
		}

		if( strcmp(argv[3], "blocks") == 0)
		{
			print_blocks_of( argv[4]);
			// filename = argv[4]
		}
	}

	if( strcmp(argv[2], "-d") == 0)
	{

		delete_block( argv[3]);
		// filename = argv[3]
	}

	close (disk_fd);
	return (0);
}
