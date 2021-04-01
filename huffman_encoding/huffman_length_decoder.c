/*
 * huffman_lengths_decoder.c
 *
 *  Created on: Mar 5, 2018
 *      Author: Zach
 */

#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "stdlib.h"

void huffLength_dec(int alphabetSize, FILE * inFile, FILE * outFile)
{
	uint16_t i, j;
	uint16_t codeCount = 0;
	uint16_t * codes	= (uint16_t *)malloc(alphabetSize * sizeof(uint16_t));
	uint8_t  * codeLen	= (uint8_t *)malloc(alphabetSize * sizeof(uint8_t));
	uint8_t  * lenCpy	= (uint8_t *)malloc(alphabetSize * sizeof(uint8_t));
	memset(codes, 0, alphabetSize * sizeof(uint16_t));
	memset(codeLen, 0, alphabetSize * sizeof(uint8_t));

	uint8_t minLen = 17;
	uint8_t minLoc = 0;

	uint16_t eBits;

	/* Obtain codeword lengths from file and place lengths as header in compressed file */
	fread(codeLen, sizeof(uint8_t), alphabetSize, inFile);

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


	int move = 16, moveSum = 0, cleared = 0;
	uint16_t codeCheck = 0, tempCode = 0;
	char c1 = getc(inFile), c2 = getc(inFile);

	/* Loop until entire encoded file has been read */
	while(!feof(inFile))
	{
		eBits = (uint8_t)c1;
		eBits = eBits << 8;
		eBits |= (uint8_t)c2;
		cleared = 0;

		/* Loop until all bits have been decoded or stored */
		while(cleared == 0)
		{
			if(move >= (16 - moveSum))
			{
				tempCode = tempCode << (16 - moveSum);
				tempCode |= eBits >> (moveSum);
				eBits = eBits << (16 - moveSum);

				move -= (16 - moveSum);
				moveSum = 0;
				cleared = 1;
			} else
			{
				tempCode = tempCode << move;
				tempCode |= eBits >> (16 - move);
				eBits = eBits << move;

				moveSum += move;
				move = 0;

				/* Loop through codes to find a match */
				for(i = 0; i < alphabetSize; i++)
				{
					if(codeLen[i] < 17)
					{
						codeCheck = tempCode >> (16 - codeLen[i]);

						if(codes[i] == codeCheck)
						{
							fputc((char)i, outFile);
							move += (int)codeLen[i];
							i = alphabetSize;
						}
					}
				}
			}
		}

		c1 = getc(inFile);
		c2 = getc(inFile);
	}

	fputc(EOF, outFile);
}
