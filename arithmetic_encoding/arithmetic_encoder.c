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
#define E3_MASK       0x4000   

extern void bit_feeder(FILE * source_file, char * symbol, uint8_t sym_size, uint16_t * bits_prev, uint16_t bits_now, char * store_char);
extern void bit_stuffer(FILE * destination_file, char bit_out, char * stuffed_byte, uint16_t * bits_stuffed);

void arithmetic_encoder(FILE * inFile, FILE * outFile)
{
    uint16_t i, j, k;     //variables for miscellaneous tasks

    fseek(inFile, 0L, SEEK_END);                //go to the end of the file
    long total_symbols = 2 * ftell(inFile);     //get the character count in the file
    fseek(inFile, 0L, SEEK_SET);                //return to the start of the file
    long symbol_count = 0;                      //count variable for reading through file

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
    *read_storage = fgetc(inFile);     //load the first character
    *bits_read = 0;                    //no bits have been read yet

    char * write_storage = (char *)malloc(sizeof(char));              //a variable to hold a byte until it has been filled
    uint16_t * bits_wrote = (uint16_t *)malloc(sizeof(uint16_t));     //a variable to track how many bits have been written

    *write_storage = 0;     //nothing written
    *bits_wrote = 0;        //not bits have been written

    /* Initialize letter count array and total count */
    for(int i = 0; i < 16; i++)
    {
        count[i] = 1;
        total_count++;
    }     //for
    
    /* Scan through entire file */
    for(symbol_count = 0; symbol_count < total_symbols; symbol_count++)
	{
        lower0 = lower1;     //update the previous lower bound
        upper0 = upper1;     //update the previous upper bound

        cum_count0 = 0;     //clear cumulative counts
        cum_count1 = 0;     //clear cumulative counts
        
        //read the next 4-bit symbol
        bit_feeder(inFile, in_symbol, 4, bits_read, bits_now, read_storage);

        //if the symbol is 0000
        if(*in_symbol == 0) cum_count0 = 0;     //set cum_count(x-1) to zero
        else for(i = 0; i < (*in_symbol); i++) cum_count0 += count[i];     //calculate cum_count(x-1)

        cum_count1 = cum_count0 + count[(*in_symbol)];     //calculate cum_count(x)

        lower1 = lower0 + (((upper0 - lower0 + 1) * cum_count0) / total_count);         //calculate the new lower bound
        upper1 = lower0 + (((upper0 - lower0 + 1) * cum_count1) / total_count) - 1;     //calculate the new upper bound

        /* while the MSB of u and l are equal to b or E3 condition holds */
        while( ((lower1 & MSB_MASK) == (upper1 & MSB_MASK)) || ((lower1 & E3_MASK) > (upper1 & E3_MASK)) )
        {
            /* if the MSB of lower and upper bound are equal */
            if( (lower1 & MSB_MASK) == (upper1 & MSB_MASK) )
            {
                //if the MSB of l and u are 1
                if( (lower1 & MSB_MASK) == MSB_MASK)  bit_stuffer(outFile, (char)(1), write_storage, bits_wrote);     //send a one
                else bit_stuffer(outFile, (char)(0), write_storage, bits_wrote);     //send a zero

                //while rescaling is required
                while(scale3 > 0)
                {
                    //if the MSB of l and u were 1
                    if( (lower1 & MSB_MASK) == MSB_MASK) bit_stuffer(outFile, (char)(0), write_storage, bits_wrote);     //send a zero
                    else bit_stuffer(outFile, (char)(1), write_storage, bits_wrote);     //send a one
                    
                    scale3--;     //decriment scale3
                }     //while scale3

                lower1 <<= 1;     //shift l left 1 and shift a 0 into LSB
                upper1 <<= 1;     //shift u left 1
                upper1 += 1;      //shift a 1 into LSB of u
            }     //if E1 or E2

            /* if the E3 condition holds (2nd MSB of lower bound is greater than 2nd MSB of upper bound) */
            if( (lower1 & E3_MASK) > (upper1 & E3_MASK) )
            {
                lower1 <<= 1;    //shift lower bound left 1 bit and shift a 0 into LSB
                upper1 <<= 1;    //shift upper bound left 1 bit
                upper1 += 1;     //shift a 1 into LSB of uppper bound
                
                /* if the MSB of the lower bound is one */
                if( (lower1 & MSB_MASK) == MSB_MASK) lower1 &= ~(MSB_MASK);     //set MSB to zero
                else lower1 |= MSB_MASK;     //set MSB to one
                
                /* if the MSB of the upper bound is a 1 */
                if( (upper1 & MSB_MASK) == MSB_MASK) upper1 &= ~(MSB_MASK);     //set MSB to zero
                else  upper1 |= MSB_MASK;     //set MSB to 1
                
                scale3++;     //increment scale3 count
            }     //if E3
        }     //while E1, E2, or E3

        count[ (*in_symbol) ]++;     //increment symbol count
        total_count++;           //increment total count

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
    }     //while reading through file

    /* Transmit the lower bound to complete encoding */
    char trsmt_low = (char)((lower1 & MSB_MASK) >> 15);             //determine MSB of l
    bit_stuffer(outFile, trsmt_low, write_storage, bits_wrote);     //send the MSB of l

    /* check scale3 */
    while(scale3 > 0)
    {
        //if the MSB of l was one
        if( (lower1 & MSB_MASK) == MSB_MASK) bit_stuffer(outFile, (char)(0), write_storage, bits_wrote);     //send zero
        else bit_stuffer(outFile, (char)(1), write_storage, bits_wrote);     //send one
        
        scale3--;     //decriment scale3
    }     //while scale3

    /* transmit remain bits of l */
    for(i = 0; i < 15; i++)
    {
        trsmt_low = (char)((lower1 & (MSB_MASK >> (1 + i))) >> (14 - i));     //determine next bit of l
        bit_stuffer(outFile, trsmt_low, write_storage, bits_wrote);           //send next bit of l
    }     //for 15 least significant bits

    /* pad output with zeros if a character is not full */
    for(i = (8 - (*bits_wrote)); i > 0; i--) bit_stuffer(outFile, (char)(0), write_storage, bits_wrote);
}     //arithmetic_encoder