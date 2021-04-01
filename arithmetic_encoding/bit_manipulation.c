/*
 * bit_manipulation.c
 *
 *  Created on: Nov 15, 2018
 *      Author: Zachary M Swanson
 */

#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

/*
 * Function:      bit_feeder
 * Description:   This function manages a pseudo-continuous bit stream from a source file.
 *                The user can load x number of bits into the x least significant bits of a 
 *                char variable and pads the remaining most significant bits with zeros.
 * 
 * Inputs:
 *      - source _file: The file that data is being read from in bytes. 
 *      - symbol: A pointer to a char containing previous bits and where new bits will be loaded.
 *      - bits_prev: A pointer to track how many bits have been used from the current 
 *            byte of data from the source file.
 *      - bits_now: The user specifies how many bits to load into "symbol".
 *      - store_char: A pointer to a char that holds the latest byte read in from the source file.
 *                
 * Notes: This function is currently set up for a word size of 4 bits. To edit the word 
 *        size change the mask value used to zero unwanted bits.
 */

void bit_feeder(FILE * source_file, char * symbol, uint8_t sym_size, uint16_t * bits_prev, uint16_t bits_now, char * store_char)
{
    if(bits_now > 8) bits_now = 8;     //safeguard against greater than 8-bit word lengths

    char temp = *store_char << *bits_prev;     //load unused bits into a temporary variable

    //if the required bits extends beyond current bit stream
    if((*bits_prev + bits_now) > 8)
    {
        *store_char = fgetc(source_file);               //get the next 8 bits from the source file

        char hold = *store_char >> (8 - *bits_prev);    //hold the bits that will fit into temp

        //safeguard against right shifting in ones
        switch(*bits_prev)
        {
            case 0: hold &= 0x00; break;
            case 1: hold &= 0x01; break;
            case 2: hold &= 0x03; break;
            case 3: hold &= 0x07; break;
            case 4: hold &= 0x0F; break;
            case 5: hold &= 0x1F; break;
            case 6: hold &= 0x3F; break;
            case 7: hold &= 0x7F; break;
            default: hold &= 0xFF; break;
        }     //switch

        temp |= hold;                                   //tack new character bits onto the end of temp
        *bits_prev = ((*bits_prev + bits_now) - 8);     //update the bit_prev count
    }     //if

    //else the bit stream contains a sufficient amount of bits
    else
    {
        *bits_prev += bits_now;     //update the bit_prev count
    }     //else

    temp = temp >> (8 - bits_now);     //shift the desired bits into the desired position

    switch(bits_now)
    {
        case 1: temp &= 0x01; break;
        case 2: temp &= 0x03; break;
        case 3: temp &= 0x07; break;
        case 4: temp &= 0x0F; break;
        case 5: temp &= 0x1F; break;
        case 6: temp &= 0x3F; break;
        case 7: temp &= 0x7F; break;
        default: temp &= 0xFF; break;
    }     //switch

    *symbol = *symbol << bits_now;     //shift out old bits
    *symbol |= temp;     //load in new bits

    //mask off old bits
    switch(sym_size)
    {
        case 1:  *symbol &= 0x01; break;
        case 2:  *symbol &= 0x03; break;
        case 3:  *symbol &= 0x07; break;
        case 4:  *symbol &= 0x0F; break;
        case 5:  *symbol &= 0x1F; break;
        case 6:  *symbol &= 0x3F; break;
        case 7:  *symbol &= 0x7F; break;
        case 8:  *symbol &= 0xFF; break;
        default: *symbol &= 0xFF; break;
    }     //switch
}     //bit_feeder










/*
 * Function:        bit_stuffer
 * Description:     
 */
void bit_stuffer(FILE * destination_file, char bit_out, char * stuffed_byte, uint16_t * bits_stuffed)
{
    (*bits_stuffed)++;
    *stuffed_byte = *stuffed_byte << 1;     //shift to make room for new bits
    *stuffed_byte |= bit_out;               //stuff the bits

    //if the bits to be stuffed will fill a byte
    if((*bits_stuffed) == 8)
    {
        *bits_stuffed = 0;                            //update the bits_stuffed count
        fputc((*stuffed_byte), destination_file);     //write the stuffed byte to the destination file
        *stuffed_byte = 0;                        //clear the stuffed byte
    }     //if
}     //bit_stuffer