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

int 
main(int argc, char *argv[])
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


	close (disk_fd); 

	return (0); 
}

