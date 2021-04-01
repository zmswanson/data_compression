/*
 * main.c
 *
 *  Created on: Feb 25, 2018
 *      Author: Zach
 */

#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

#define WORK_LEN	256
#define TYPE_LEN	256

/* Create count, work and type tables for encoding */
uint16_t	count[WORK_LEN];
float		work[WORK_LEN];
uint8_t		type[TYPE_LEN];
uint8_t	codeLens[WORK_LEN];

extern void moffKataj( int alphabetSize, uint16_t * count, float * work, uint8_t * type, uint8_t * lengths, FILE * lengthFile);
extern void huffLength_enc(int alphabetSize, FILE * lengthFile, FILE * inFile, FILE * outFile);
extern void huffLength_dec(int alphabetSize, FILE * inFile, FILE * outFile);
void getUserFile(char * userPrompt, FILE ** userFile, char * openType);
void huffTable(int alphabetSize, uint8_t * codeLengths);
void compInOut(FILE * inFile, FILE * outFile);
void compressionCheck(FILE * inFile, FILE * outFile);

int main(int argc, char * argv[])
{
	FILE * original;
	FILE * codeLengths;
	FILE * compressed;
	FILE * decompressed;

	if(argc != 5)
	{
		fprintf(stderr, "Need four file names.\n\n");
		return 1;
	}

	if((original = fopen(argv[1], "rb")) == 0)
	{
		fprintf(stderr, "Can't open %s.\n\n", argv[1]);
		return 2;
	}

	if((codeLengths = fopen(argv[2], "wb+")) == 0)
	{
		fprintf(stderr, "Can't open %s.\n\n", argv[2]);
		return 3;
	}

	if((compressed = fopen(argv[3], "wb+")) == 0)
	{
		fprintf(stderr, "Can't open %s.\n\n", argv[3]);
		return 4;
	}

	if((decompressed = fopen(argv[4], "wb+")) == 0)
	{
		fprintf(stderr, "Can't open %s.\n\n", argv[4]);
	}

	/* Initialize work and type tables */
	memset(count, 0, WORK_LEN);
	memset(work, 0, WORK_LEN);
	memset(type, 0, TYPE_LEN);

	/* Scan through file and count symbol occurrences */
	static int c, i;
	uint32_t TC = 0;

	while(!feof(original))
	{
		c = getc(original);

		++count[c];

		++TC;
	}

	/* Calculate probabilities of all symbols */
	for(i = 0; i < WORK_LEN; i++)
	{
		work[i] = ( (float)(count[i]) / (float)(TC) );
	}

	/* Generate code word lengths using Moffat-Katajainen method */
	moffKataj(WORK_LEN, count, work, type, codeLens, codeLengths);

	/* Print a table of canonical Huffman codes and their lengths */
	huffTable(WORK_LEN, codeLens);

	rewind(original);
	rewind(codeLengths);

	/* Encode the original file */
	huffLength_enc(WORK_LEN, codeLengths, original, compressed);

	rewind(original);
	rewind(codeLengths);
	rewind(compressed);

	/* Decode the encoded file */
	huffLength_dec(WORK_LEN, compressed, decompressed);

	rewind(compressed);
	rewind(decompressed);

	/* Compare the original file and the decompressed file for error check */
	compInOut(original, decompressed);

	rewind(original);

	/* Compare the original file and the compressed file  for compression ratio */
	compressionCheck(original, compressed);

	fclose(original);
	fclose(codeLengths);
	fclose(compressed);
	fclose(decompressed);

	return 0;
}


/***********************************************************************************
 * huffTable
 * @brief:
 ***********************************************************************************/
void huffTable( int alphabetSize, uint8_t * codeLengths)
{
	uint16_t i, j;
	uint16_t codeCount = 0;
	uint16_t * codes = (uint16_t *)malloc(alphabetSize * sizeof(uint16_t));
	uint8_t * lenCpy = (uint8_t *)malloc(alphabetSize * sizeof(uint8_t));
	memset(codes, 0, alphabetSize*sizeof(uint16_t));
	memcpy(lenCpy, codeLengths, alphabetSize * sizeof(uint8_t));

	uint8_t minLen = 17;
	uint8_t minLoc = 0;

	for( i = 0; i < alphabetSize; i++ )
	{
		/* Find minimum codeword length */
		for( j = 0; j < alphabetSize; j++ )
		{
			if( lenCpy[j] < minLen )
			{
				minLen = lenCpy[j];
				minLoc = j;
			}
		}

		/* if the codeword is 16-bits or less */
		if( minLen < 17 )
		{
			/* Isolate desired bits of counter */
			codes[minLoc] = (codeCount >> (16 - lenCpy[minLoc]));

			/* Increment counter for next code word */
			while( (codeCount >> (16 - lenCpy[minLoc])) == codes[minLoc])
			{
				++codeCount;
			}
		} else {
			i = alphabetSize;	//break out of the for loop
		}

		/* Reset minimum length and set previous minimum length so it will be ignored */
		minLen = 17;
		lenCpy[minLoc] = 17;
	}

	printf("\n\nSymbol		Length		Code\n");

	for( i = 0; i < alphabetSize; i++ )
	{
		if( codeLengths[i] < 17 )
		{
			printf("%d		%d		%X\n", i, codeLengths[i], codes[i]);
		} else {
			printf("%d		%d		-\n", i, codeLengths[i]);
		}
	}
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
