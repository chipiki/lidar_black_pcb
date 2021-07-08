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
    FILE *wfp; // write

    long int sz = 0;
    long int rbs = 0; // readed bytes

    int c = 0;
    char hb2 = 0;
    int count = 0;

    if( argc != 3 ){
    	printf("\nExapmle usage:\n");
    	printf( argv[0] );
    	printf(" InFile OutFile\n" );
    	printf(" InFile\t\t - file to read\n" );
    	printf(" OutFile\t - file to write\n" );
    	return EXIT_FAILURE;
    }

    /* open a file to write */
    rfp = fopen( argv[1] , "rb");
    if (rfp == NULL) {
             printf("Error opening input file!\n");
             return EXIT_FAILURE;
    }

//    /* open a file to write */
//    wfp = fopen( argv[2] , "w");
//    if (wfp == NULL) {
//             printf("Error opening output file!\n");
//             fclose(rfp); // close read file first
//             return EXIT_FAILURE;
//    }


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
    //fclose(wfp);

    return EXIT_SUCCESS;
}

