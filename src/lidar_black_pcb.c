/*
 ============================================================================
 Name        : lidar_black_pcb.c
 Author      : Denis
 Version     :
 Copyright   : (c) Denis Tsekh, 2020.
 Description : Read lidar data dump
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


int main(int argc, char *argv[])
{
    FILE *rfp; // read

    long int sz = 0;

    int c = 0;

    if( argc != 2 ){
    	printf("\nExapmle usage:\n");
    	printf( argv[0] );
    	printf(" InFile\t\t - file to read\n" );
    	return EXIT_FAILURE;
    }

    // read file in binary mode
    rfp = fopen( argv[1] , "rb");
    if (rfp == NULL) {
    	printf("Error opening input file!\n");
    	return EXIT_FAILURE;
    }

    fseek(rfp, 0L, SEEK_END);
    sz = ftell(rfp);
    rewind(rfp);

    printf("\nInFile size is %ld bytes\n\n", sz);

	while(!feof(rfp)){
		c = fgetc( rfp );

		if (c != EOF ){
			decoder( (uint8_t) c );
		}
	}

    /* close file */
    fclose(rfp);

    return EXIT_SUCCESS;
}

