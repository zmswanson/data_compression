/*
 * arithmetic_encoder.c
 *
 *  Created on: Nov 06, 2018
 *      Author: Zachary M Swanson
 */

#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

#define MAX_COUNT     16384
#define MAX_LIMIT     65535
#define MSB_MASK      0x8000
#define MSB_CLEAR     0x7FFF
#define E3_MASK       0x4000   

extern void bit_feeder(FILE * source_file, char * symbol, uint8_t sym_size, uint16_t * bits_prev, uint16_t bits_now, char * store_char);
extern void bit_stuffer(FILE * destination_file, char bit_out, char * stuffed_byte, uint16_t * bits_stuffed);

void arithmetic_decoder(FILE * inFile, FILE * outFile)
{
    uint16_t i, j, k;     //variables for miscellaneous tasks    
    
    fseek(inFile, 0L, SEEK_END);                //go to the end of the file
    long total_bits = 8 * ftell(inFile);     //get the character count in the file
    fseek(inFile, 0L, SEEK_SET);                //return to the start of the file
    long bit_count = 0;                      //count variable for reading through file

    uint16_t * count = (uint16_t *)malloc(sizeof(uint16_t) * 16);     //allocate memory to a count array
    uint16_t cum_count0 = 0;         //a variable to hold cumulative count of x-1, initialize to zero
    uint16_t cum_count1 = 0;         //a variable to hold cumulative count of x, initialize to zero
    uint16_t total_count = 0;        //a variable to hold the total count of symbols
    uint16_t scale3 = 0;             //a variable to track E3 conditions
    uint16_t lower0 = 0;             //a variable to hold the previous lower bound
    uint16_t lower1 = 0;             //a variable to hold the new lower bound
    uint16_t upper0 = MAX_LIMIT;     //a variable to hold the old upper bound, initialize to max value of variable type
    uint16_t upper1 = MAX_LIMIT;     //a variable to hold the new upper bound

    char * in_symbol = (char *)malloc(sizeof(char));                 //a variable to hold incoming symbols
    char * read_storage = (char *)malloc(sizeof(char));              //a variable to hold a byte until all bits are read
    uint16_t * bits_read = (uint16_t *)malloc(sizeof(uint16_t));     //a variable to track how many bits have been read
    uint16_t bits_now = 4;                                           //a variable to hold how many bits to read next

    *in_symbol = 0;                    //no symbol loaded
    *read_storage = fgetc(inFile);     //load first character
    *bits_read = 0;                    //zero bits read

    char * write_storage = (char *)malloc(sizeof(char));              //a variable to hold a byte until it has been filled
    uint16_t * bits_wrote = (uint16_t *)malloc(sizeof(uint16_t));     //a variable to track how many bits have been written

    *write_storage = 0;     //nothing to write
    *bits_wrote = 0;        //zero bits written

    uint16_t * tag = (uint16_t *)malloc(sizeof(uint16_t));     //a variable to track the tag
   
   /* load the first 16 bits into the tag */
    bit_feeder(inFile, in_symbol, 8, bits_read, 8, read_storage);
    *tag = (((uint16_t)(*in_symbol) << 8) & 0xFF00);
    bit_feeder(inFile, in_symbol, 8, bits_read, 8, read_storage);
    *tag |= ((uint16_t)(*in_symbol) & 0x00FF);
    bit_count = 16;
    
    /* Initialize letter count array and total count */
    for(int i = 0; i < 16; i++)
    {
        count[i] = 1;
        total_count++;
    }     //for

    /* Scan through entire file */
    while( (*tag != lower1) || (bit_count < (total_bits - 16)) )
	{
        lower0 = lower1;     //update the previous lower bound
        upper0 = upper1;     //update the previous upper bound

        i = 0;
        j = 0;
        k = 0;

        /* Determine the decoding value to check against cum counts */
        i = ((*tag - lower0 + 1) * total_count - 1) / (upper0 - lower0 + 1);

        /* used i to decode the symbol */
        while(i >= j && k < 16)
        {
            j = j + count[k];     //increment the cumulative count
            k = k + 1;            //increment the symbol count
        }     //while

        /* based on k-1, use a switch statement to send the decoded symbol */
        switch( (k-1) )
        {
            case 0:
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                break;
            case 1:
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                break;
            case 2:
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                break;
            case 3:
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                break;
            case 4:
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                break;
            case 5:
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                break;
            case 6:
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                break;
            case 7:
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                break;
            case 8:
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                break;
            case 9:
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                break;
            case 10:
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                break;
            case 11:
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                break;
            case 12:
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                break;
            case 13:
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                break;
            case 14:
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 0, write_storage, bits_wrote);
                break;
            case 15:
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                break;
            default:
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                bit_stuffer(outFile, 1, write_storage, bits_wrote);
                break;
        }     //switch k

        cum_count0 = j - count[ (k - 1) ];     //calculate cum_count(x-1)
        cum_count1 = j;                        //calculate cum_count(x)

        lower1 = lower0 + (((upper0 - lower0 + 1) * cum_count0) / total_count);         //calculate the new lower bound
        upper1 = lower0 + (((upper0 - lower0 + 1) * cum_count1) / total_count) - 1;     //calculate the new upper bound

        /* while the MSB of u and l are equal to b or E3 condition holds */
        while( ((lower1 & MSB_MASK) == (upper1 & MSB_MASK)) || ((lower1 & E3_MASK) > (upper1 & E3_MASK)) )
        {
            /* if the MSB of lower and upper bound are equal */
            if( (lower1 & MSB_MASK) == (upper1 & MSB_MASK) )
            {
                lower1 <<= 1;     //shift l left 1 and shift a 0 into LSB
                upper1 <<= 1;     //shift u left 1
                upper1 += 1;      //shift a 1 into LSB of u

                *tag <<= 1;       //shift tag left 1 bit
                bit_feeder(inFile, in_symbol, 1, bits_read, 1, read_storage);     //read the next bit
                *tag |= (((uint16_t)(*in_symbol)) & 0x0001);                      //load next bit into tag's LSB
                bit_count++;
            }     //if E1 or E2

            /* if the E3 condition holds (2nd MSB of lower bound is greater than 2nd MSB of upper bound) */
            if( (lower1 & E3_MASK) > (upper1 & E3_MASK) )
            {
                lower1 <<= 1;    //shift lower bound left 1 bit and shift a 0 into LSB
                upper1 <<= 1;    //shift upper bound left 1 bit
                upper1 += 1;     //shift a 1 into LSB of uppper bound

                *tag <<= 1;       //shift tag left 1 bit
                bit_feeder(inFile, in_symbol, 1, bits_read, 1, read_storage);     //read the next bit
                *tag |= (((uint16_t)(*in_symbol)) & 0x0001);                      //load next bit into tag's LSB
                bit_count++;
                
                /* if the MSB of the lower bound is one */
                if( (lower1 & MSB_MASK) == MSB_MASK) lower1 &= MSB_CLEAR;     //set MSB to zero
                else lower1 |= MSB_MASK;     //set MSB to one
                
                /* if the MSB of the upper bound is a one */
                if( (upper1 & MSB_MASK) == MSB_MASK) upper1 &= MSB_CLEAR;     //set MSB to zero
                else upper1 |= MSB_MASK;     //set MSB to one
                
                /* if the MSB of the upper bound is a 1 */
                if( (*tag & MSB_MASK) == MSB_MASK) *tag &= MSB_CLEAR;     //compliment MSB of upper bound
                else *tag |= MSB_MASK;     //compliment MSB of upper bound
            }     //if E3
        }     //while E1, E2, or E3

        count[ (k - 1) ]++;     //increment symbol count
        total_count++;          //increment total count

        /* if the total symbol count reaches 2^(16-2), then rescale */
        if(total_count >= MAX_COUNT)
        {
            total_count = 0;

            for(i = 0; i < 16; i++)
            {
                count[i] = count[i] >> 1;     //rescale by half
                count[i]++;                   //add 1 to ensure no counts of 0

                total_count += count[i];
            }     //for length of count array
        }     //if total count > 2^(16-2)
    }     //while decoding
}     //arithmetic_decoder