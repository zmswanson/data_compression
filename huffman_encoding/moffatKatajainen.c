/*
 * moffatKatajainen.c
 *
 *  Created on: Feb 26, 2018
 *      Author: Zach
 */

#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

#define LEAF		0
#define INTERNAL	1
#define PARENT		2
#define DEPTH		3
#define LENGTH		4
#define EMPTY		5

volatile static int i, j, k;


/*
 * moffKataj
 */
void moffKataj( int alphabetSize, uint16_t * count, float * work, uint8_t * type, uint8_t * lengths, FILE * lengthFile)
{
	uint8_t * lexiOrder = (uint8_t *)malloc(alphabetSize * sizeof(uint8_t));

	memset(type, 0, alphabetSize);
	memset(lengths, 0, alphabetSize);

	for(i = 0; i < alphabetSize; i++)
	{
		lexiOrder[i] = i;
	}

	/* Sort probabilities in increasing order */
	float temp = 0;

	for(i = 0; i < (alphabetSize - 1); ++i)
	{
		for(j = (i + 1); j < alphabetSize; ++j)
		{
			if(work[i] > work[j])
			{
				/* Sort probabilities */
				temp = work[i];
				work[i] = work[j];
				work[j] = temp;

				/* Sort lexicographical order to match probabilities */
				temp = lexiOrder[i];
				lexiOrder[i] = lexiOrder[j];
				lexiOrder[j] = temp;
			}

		}
	}

	/***********************************************************************
	 *****************************   PHASE 1   *****************************
	 ***********************************************************************/

	/* Phase 1: Combine weights to create internal nodes and find root of Huffman binary tree */
	printf("PHASE 1...\n");
	float min1 = 1;
	float min2 = 1;
	uint8_t minPtr1 = 0;
	uint8_t minPtr2 = 0;

	/* Total no. of leaf/internal combinations possible  = No. of symbols - 1 */
	for(i = 0; i < alphabetSize - 1; i++)
	{
		min1 = 1;
		min2 = 1;
		minPtr1 = 0;
		minPtr2 = 0;

		/* Loop through symbols to find the 2 left-most  */
		for(j = 0; j < alphabetSize; j++)
		{
			/* Is the current element a leaf or an internal node */
			if( (type[j] == LEAF) || (type[j] == INTERNAL) )
			{
				/* Is the current element less than the minimum values? */
				if((work[j] < min1) && (work[j] < min2))
				{
					min1 = work[j];		//set new minimum value
				}//if

				/* Is the current element less than the second minimum value? */
				else if( work[j] < min2 )
				{
					min2 = work[j];		//set new minimum value
				}//else if

				/* This is a redundant check */
				else if ( work[j] < min1 )
				{
					min1 = work[j];		//set new minimum value
				}//else if
			}//if
		}//for

		/* Loop through symbols, again. Index in reverse order */
		for(j = 0; j < alphabetSize; j++)
		{
			/* Is the current element a leaf or a node? */
			if( (type[alphabetSize - 1 - j] == LEAF) || (type[alphabetSize - 1 - j] == INTERNAL) )
			{
				/* Is the current element equal to both minimum values? */
				if((work[alphabetSize - 1 - j] == min2) && (work[alphabetSize - 1 - j] == min1))
				{
					minPtr2 = alphabetSize - 1 - j;	//set second left-most position of minimum element
					min2 = 1;
				}

				/* Is the current element equal to the second left-most minimum element? */
				else if ((work[alphabetSize - 1 - j] == min2)) {
					minPtr2 = alphabetSize - 1 - j;	//set second left-most position of minimum element
					min2 = 1;
				}

				/* Is the current element equal to  the left-most minimum element? */
				else if (work[alphabetSize - 1 - j] == min1) {
					minPtr1 = alphabetSize - 1 - j;	//set the left-most position of minimum element
					min1 = 1;
				}//if
			}//if
		}//for

		/* Are the left-most minimum elements both internal nodes? */
		if( (type[minPtr1] == INTERNAL) && (type[minPtr2] == INTERNAL) )
		{
			/* Find first empty cell */
			k = 0;
			while((type[k] != EMPTY) && (k < alphabetSize))
			{
				++k;
			}

			/* Combine the internal node weights at cell k and change type to INTERNAL */
			work[k] = work[minPtr1] + work[minPtr2];
			type[k] = INTERNAL;

			/* Set first internal node to PARENT type and point to cell k */
			type[minPtr1] = PARENT;
			work[minPtr1] = k;

			/* Set second internal node to PARENT type and point to cell k */
			type[minPtr2] = PARENT;
			work[minPtr2] = k;
		}

		/* Is the left-most minimum element an internal node? */
		else if(type[minPtr1] == INTERNAL)
		{
			/* Find the first empty cell */
			k = 0;
			while((type[k] != EMPTY) && (k < alphabetSize))
			{
				++k;
			}

			/* Is the first empty cell before the leaf cell? */
			if(k < minPtr2)
			{
				/* Combine the leaf and internal cells at cell k and set k to INTERNAL */
				work[k] = work[minPtr1] + work[minPtr2];
				type[k] = INTERNAL;

				/* Clear the leaf cell and change to EMPTY */
				work[minPtr2] = 0;
				type[minPtr2] = EMPTY;

				/* Change internal cell to PARENT and point to cell k */
				work[minPtr1] = k;
				type[minPtr1] = PARENT;
			}

			else
			{
				/* Combine the leaf and internal cells in the leaf cell and change leaf to internal */
				work[minPtr2] = work[minPtr1] + work[minPtr2];
				type[minPtr2] = INTERNAL;

				/* Change internal cell to PARENT and point to new internal cell */
				work[minPtr1] = minPtr2;
				type[minPtr1] = PARENT;
			}
		}

		/* Is the second left-most minimum element an internal node? */
		else if(type[minPtr2] == INTERNAL) {

			/* Find the first empty cell */
			k = 0;
			while((type[k] != EMPTY) && (k < alphabetSize))
			{
				++k;
			}

			/* Is the first empty cell before the leaf cell? */
			if(k < minPtr1)
			{
				/* Combine the leaf and internal cells at cell k and set k to INTERNAL */
				work[k] = work[minPtr1] + work[minPtr2];
				type[k] = INTERNAL;

				/* Clear the leaf cell and change to EMPTY */
				work[minPtr1] = 0;
				type[minPtr1] = EMPTY;

				/* Change internal cell to PARENT and point to cell k */
				work[minPtr2] = k;
				type[minPtr2] = PARENT;
			}

			else
			{
				/* Combine the leaf and internal cells in the leaf cell and change leaf to internal */
				work[minPtr1] = work[minPtr1] + work[minPtr2];
				type[minPtr1] = INTERNAL;

				/* Change internal cell to PARENT and point to new internal cell */
				work[minPtr2] = minPtr1;
				type[minPtr2] = PARENT;
			}
		}

		/* Are both left-most elements leafs? */
		else {

			/* Find first empty cell */
			k = 0;
			while((type[k] != EMPTY) && (k < alphabetSize))
			{
				++k;
			}

			/* Is the first empty cell before the first leaf cell? */
			if(k < minPtr1)
			{
				/* Combine the leaf cells at cell k and set k to INTERNAL */
				work[k] = work[minPtr1] + work[minPtr2];
				type[k] = INTERNAL;

				/* Clear both leaf cell and set to EMPTY */
				work[minPtr1] = 0;
				type[minPtr1] = EMPTY;

				work[minPtr2] = 0;
				type[minPtr2] = EMPTY;
			}

			else
			{
				/* Combine the leaf cells in left-most leaf cell and set to INTERNAL */
				work[minPtr1] = work[minPtr1] + work[minPtr2];
				type[minPtr1] = INTERNAL;

				/* Clear the second leaf cell */
				work[minPtr2] = 0;
				type[minPtr2] = EMPTY;
			}
		}
	}

	/***********************************************************************
	 *****************************   PHASE 2   *****************************
	 ***********************************************************************/

	/* Phase 2: Assign depths to to each internal node A[i] <- A[A[i]] + 1 */
	printf("PHASE 2...\n");

	/* Find the root node */
	i = alphabetSize;
	while((work[--i] != 1) && (type[i] != INTERNAL));

	/* Assign the root node a depth of zero */
	work[i] = 0;
	type[i] = DEPTH;

	/* Check for any parent nodes */
	int prtChk = 0;
	for(i = 0; i < alphabetSize; i++)
	{
		if(type[i] == PARENT)
		{
			prtChk = 1;
		}
	}

	/* While parent nodes exist in the array */
	i = 0;
	while(prtChk)
	{
		/* Is the current element a parent node? */
		if(type[i] == PARENT)
		{
			/* Is the type where this parent points DEPTH? */
			if(type[(int)work[i]] == DEPTH)
			{
				/* Set this element to DEPTH equal to the depth where this parent points plus 1 */
				work[i] = work[(int)work[i]] + 1;
				type[i] = DEPTH;
				++i;
			}

			/* Is the type where this parent points PARENT? */
			else if (type[(int)work[i]] == PARENT)
			{
				/* Go to the pointed element */
				i = (int)(work[i]);
			}
		}

		else
		{
			++i;
		}

		if(i > alphabetSize - 1)
		{
			i = 0;
		}

		/* Check for more parent nodes */
		prtChk = 0;

		for(j = 0; j < alphabetSize; j++)
		{
			if(type[j] == PARENT)
			{
				prtChk = 1;
			}
		}
	}

	/***********************************************************************
	 *****************************   PHASE 3   *****************************
	 ***********************************************************************/

	/* Phase 3: Assign codeword lengths */
	printf("PHASE 3...\n");

	uint8_t depthCnt[256];
	memset(depthCnt, 0, 256);

	/* Determine the total number of each depth */
	for(i = 0; i < alphabetSize; i++)
	{
		if(type[i] == DEPTH)
		{
			depthCnt[(int)work[i]]++;
		}
	}

	i = 0;
	int nL = 0, nN = 0;		//nL: no. of leaves at depth || nN: no. of nodes at depth
	int workPtr = alphabetSize - 1;

	/* Scan depth counts left-to-right and work/type array right-to-left */
	while(depthCnt[i])
	{
		nN = 2 * depthCnt[i++];	//determine number of nodes possible
		nL = nN - depthCnt[i];	//determine number of leaves

		/* Insert the length equivalent to the current depth into nL elements */
		for(j = 0; j < nL; j++)
		{
			work[workPtr] = i;
			type[workPtr] = LENGTH;
			--workPtr;
		}
	}

	printf("Codeword lengths generated...\n");

	/* Put the codeword lengths back in lexicographical order */
	for(i = 0; i < alphabetSize; i++)
	{
		lengths[lexiOrder[i]] = work[i];
	}

	/* Write the codeword lengths to a file */
	fwrite(lengths, sizeof(uint8_t), alphabetSize, lengthFile);
	fputc(EOF, lengthFile);
}
