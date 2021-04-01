/*
 * huffman_length_encoder.c
 *
 *  Created on: Mar 4, 2018
 *      Author: Zach
 */

#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "stdlib.h"

void writeCode(uint16_t codeWord, uint8_t length, uint16_t dummyCode, uint8_t dummyLength, int * filled, char * save, FILE * outFile, int end_flag);
void getDummy(int alphabetSize, uint8_t * lengths, uint16_t * codes, uint16_t * dummyCode, uint8_t * dummyLength);

void huffLength_enc(int alphabetSize, FILE * lengthFile, FILE * inFile, FILE * outFile)
{
	uint16_t i, j;
	uint16_t codeCount = 0;
	uint16_t * codes = (uint16_t *)malloc(alphabetSize * sizeof(uint16_t));
	uint8_t * codeLen = (uint8_t *)malloc(alphabetSize * sizeof(uint8_t));
	uint8_t * lenCpy = (uint8_t *)malloc(alphabetSize * sizeof(uint8_t));
	memset(codes, 0, alphabetSize * sizeof(uint16_t));
	memset(codeLen, 0, alphabetSize * sizeof(uint8_t));

	uint8_t minLen = 17;
	uint8_t minLoc = 0;

	char c;

	/* Obtain codeword lengths from file and place lengths as header in compressed file */
	fread(codeLen, sizeof(uint8_t), alphabetSize, lengthFile);
	fwrite(codeLen, sizeof(uint8_t), alphabetSize, outFile);

	memcpy(lenCpy, codeLen, alphabetSize * sizeof(uint8_t));

	/* Generate the canonical codewords */
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

		/* If the codeword is 16-bits or less */
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

	/* Determine a dummy codeword and length */
	uint16_t dummy = 0;
	uint8_t dummyLen = 0;
	getDummy(alphabetSize, codeLen, codes, &dummy, &dummyLen);

	int filled = 0;
	char save = 0;

	while( !feof(inFile) )
	{
		/* Read a character from the input file */
		c = getc(inFile);

		/* Write a character to the compressed file */
		writeCode(codes[(uint8_t)c], codeLen[(uint8_t)c], dummy, dummyLen, &filled, &save, outFile, 0);
	}

	/* Write an end-of-file tag to the end of the compressed file */
	writeCode(codes[(uint8_t)c], codeLen[(uint8_t)c], dummy, dummyLen, &filled, &save, outFile, 1);
}



/**************************************************************************************************
 * writeCode - generate binary stream by stuffing bits
 **************************************************************************************************/
void writeCode(uint16_t codeWord, uint8_t length, uint16_t dummyCode, uint8_t dummyLength, int * filled, char * save, FILE * outFile, int end_flag)
{
	int move, bytesize;
	bytesize = 8;

	/* Is the codeword longer than 16 bits? */
	if(length > 16)
	{
	   /* Encode a dummy codeword instead */
	   codeWord = dummyCode;
	   length = dummyLength;
	}

	/* Determine how far to shift the codeword for bit stuffing */
	move = bytesize - (* filled) - length;

	/* Will the entire codeword fit into the previously-stuffed byte? */
	if(move >= 0)
	{
		/* Tack the entire codeword onto the previously-stuffed byte and note how much is filled */
		(* save) = (* save) | (codeWord << move);
		(* filled) += length;

		/* Has a byte been stuffed? */
		if((* filled) == bytesize)
		{
			/* Write the stuffed byte to the compressed file */
			fputc((* save), outFile);
			(* save) = 0;
			(* filled) = 0;
		}
	}

	else
	{
		/* Stuff the amount of the codeword that will fit into the byte */
		(* save) = (* save) | (codeWord >> -move);

		/* Write the stuffed byte to the compressed file */
		fputc((* save), outFile);
		(* save) = 0;
		move = bytesize + move;

		/* Will the remainder of the codeword fit into a byte? */
		if( move >= 0)
		{
			/* Stuff the byte with the codeword remainder */
			(* save) = (* save) | (codeWord << move);
			(* filled) = bytesize - move;
		}

		else
		{
			/* Stuff the amount of the codeword that will fit into the byte */
			(* save) = (* save) | (codeWord >> -move);

			/* Write the stuffed byte to the compressed file */
			fputc((* save), outFile);
			(* save) = 0;
			move = bytesize + move;

			/* Stuff the remainder of the codeword into a byte */
			(* save) = (* save) | (codeWord << move);
			(* filled) = bytesize - move;
		}
	}

	/* Has the end of the file been reached */
	if(end_flag && filled != 0)
	{
		/* Write out whatever stuffed bits remain and write and EOF */
		fputc((* save), outFile);
		fputc(EOF, outFile);
	}
}

/*
 * getDummy - Search for the minimum-length codeword to insert when codeword length exceeds 16-bits
 */
void getDummy(int alphabetSize, uint8_t * lengths, uint16_t * codes, uint16_t * dummyCode, uint8_t * dummyLength)
{
	int i;
	int minLoc = 0;
	uint8_t min = lengths[0];

	for(i = 0; i < alphabetSize; ++i)
	{
		if(lengths[i] < min)
		{
			min = lengths[i];
			minLoc = i;
		}
	}

	*dummyLength = min;
	*dummyCode = codes[minLoc];
}
