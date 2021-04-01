/*
 * main.c
 *
 *  Created on: Nov 06, 2018
 *      Author: Zachary M Swanson
 */

#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

void compInOut(FILE * inFile, FILE * outFile);
void compressionCheck(FILE * inFile, FILE * outFile);

extern void arithmetic_encoder(FILE * inFile, FILE * outFile);
extern void arithmetic_decoder(FILE * inFile, FILE * outFile);

int main(int argc, char * argv[])
{
	FILE * original;
	FILE * compressed;
	FILE * decompressed;

	if(argc != 4)
	{
		fprintf(stderr, "Need three file names.\n\n");
		return 1;
	}

	if((original = fopen(argv[1], "rb")) == 0)
	{
		fprintf(stderr, "Can't open %s.\n\n", argv[1]);
		return 2;
	}

	if((compressed = fopen(argv[2], "wb+")) == 0)
	{
		fprintf(stderr, "Can't open %s.\n\n", argv[3]);
		return 3;
	}

	if((decompressed = fopen(argv[3], "wb+")) == 0)
	{
		fprintf(stderr, "Can't open %s.\n\n", argv[4]);
	}

	arithmetic_encoder(original, compressed);
	
	if(fseek(original, 0L, SEEK_SET) != 0) printf("\nRewind error\n");
	if(fseek(compressed, 0L, SEEK_SET) != 0) printf("\nRewind error\n");

	arithmetic_decoder(compressed, decompressed);
	
	if(fseek(compressed, 0L, SEEK_SET) != 0) printf("\nRewind error\n");

	compressionCheck(original, compressed);	

	if(fseek(original, 0L, SEEK_SET) != 0) printf("\nRewind error\n");
	if(fseek(decompressed, 0L, SEEK_SET) != 0) printf("\nRewind error\n");

	compInOut(original, decompressed);

	fclose(original);
	fclose(compressed);
	fclose(decompressed);

	return 0;
}

/*
 * compInOut
 */

void compInOut(FILE * inFile, FILE * outFile)
{
	char cI = 0, cO = 0;
	uint32_t TC = 0, error = 0, line = 1, lineC = 0;

	while(!feof(inFile))
	{
		cI = getc(inFile);
		cO = getc(outFile);
		++TC;
		++lineC;

		if(cO == '\n')
		{
			++line;
			lineC = 0;
		}

		if(cI != cO)
		{
			++error;
			if(error < 15)
			{
				printf("\n\nError on line %u character %i... \n", line, lineC);
				printf("\tCharacter error:		0x%2X		%c\n", cI, cI);
			} else if ( error < 16 ) {
				printf("\nToo many errors; no more error reports.\n");
			}
		}
	}

	uint8_t PM = (uint8_t)((100 * (TC - error)) / TC);
	printf("\n\tFile is a %u%% match to original.\n", PM);
	printf("\n\t\tTotal number of errors = %u\n", error);
}

/*
 * compOrigComp
 */

void compressionCheck(FILE * inFile, FILE * outFile)
{
	fseek(inFile, 0, SEEK_END);
	fseek(outFile, 0, SEEK_END);

	long int size1 = ftell(inFile);
	long int size2 = ftell(outFile);

	printf("\n\nOriginal file size:\t%li bytes\n", size1);
	printf("\n\tCompressed file size:\t%li bytes\n", size2);
	printf("\n\t\tYour file was compressed %li%%\n\n", (100 - ((100*size2)/size1)));
}
