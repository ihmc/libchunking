/*------------------------------------------------------------------------
 *
 * Copyright (c) 1997-1998 by Cornell University.
 * 
 * See the file "license.txt" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 *------------------------------------------------------------------------
 */
#include "visionInt.h"

int
BitErode8 (srcBuf, destBuf)
    BitImage *srcBuf, *destBuf;
{
    unsigned char *currSrc, *currDest;
    unsigned char *firstSrc, *firstDest;
    int h, bw;
    int srcParW, destParW;

    unsigned char *row0, *row1, *row2, *firstRow0, *firstRow2;
    unsigned char *nextSrcRow;
    unsigned int bytesIn, bytesOut;
    unsigned char scratchBuf[SCRATCHSIZE];
    register int i;

    if ((srcBuf->height != destBuf->height) || (srcBuf->byteWidth != destBuf->byteWidth))
        return DVM_DIFFERENT_SIZES;
    
    if (srcBuf->height < 2)
        return DVM_BAD_HEIGHT;

    if (srcBuf->firstBit || destBuf->firstBit || srcBuf->lastBit || destBuf->lastBit)
        return DVM_NOT_BYTE_ALLIGNED;

    if (3*srcBuf->byteWidth > SCRATCHSIZE)
        return DVM_SCRATCH_SIZE_TOO_SMALL;

    h = srcBuf->height;
    currSrc = srcBuf->firstByte;
    currDest = destBuf->firstByte;
    bw = srcBuf->byteWidth;
    srcParW = srcBuf->parentWidth;
    destParW = destBuf->parentWidth;

    firstRow0 = scratchBuf;
    row1 = firstRow0 + bw;
    firstRow2 = row1 + bw;

    firstSrc = srcBuf->firstByte;
    firstDest = destBuf->firstByte;

    memcpy(firstRow0, firstSrc, bw);


    /* 
     * First row 
     */
    currSrc = firstSrc;
    currDest = firstDest;
    nextSrcRow = currSrc + srcParW;
    row2 = row1;

    /* 
    * First byte 
    */
    bytesIn = *currSrc;
    bytesIn <<= 8;
    bytesIn |= *(currSrc+1);
    bytesOut = bytesIn<<1 & bytesIn & (bytesIn>>1 | 0x80000000);
    *row2 = *nextSrcRow;
    bytesOut &= ((unsigned int)(*row2))<<8;
    *currDest = (bytesOut>>8)&0xff;

    currDest++;
    nextSrcRow++;
    row2++;

    /*
    * Middle bytes
    */
    if (bw > 3) {
        DO_N_TIMES(bw/2-1,
            bytesIn = *currSrc;
            bytesIn <<= 8;
            bytesIn |= *(currSrc+1);
            bytesIn <<= 8;
            bytesIn |= *(currSrc+2);
            bytesIn <<= 8;
            bytesIn |= *(currSrc+3);

            bytesOut = bytesIn<<1 & bytesIn & bytesIn>>1;
            *row2 = *nextSrcRow;
            bytesOut &= ((unsigned int)(*row2))<<16 | 0xff00ffff;

            row2++;
            nextSrcRow++;
            *row2 = *nextSrcRow;
            bytesOut &= ((unsigned int)(*row2))<<8 | 0xffff00ff;

            *currDest = (bytesOut>>16)&0xff;
            currDest++;
            *currDest = (bytesOut>>8)&0xff;
            currSrc += 2;
            currDest++;
            nextSrcRow++;
            row2++;
            );
    }

    if (bw % 2 == 0) {
        /* 
         * Last byte if even number of bytes
         */
        bytesIn = *currSrc;
        bytesIn <<= 8;
        bytesIn |= *(currSrc+1);
        bytesOut = (bytesIn<<1 | 1) & bytesIn & bytesIn>>1;
        *row2 = *nextSrcRow;
        bytesOut &= (unsigned int)(*row2);
        *currDest = bytesOut&0xff;
    } else {
        /* 
         * Last two bytes if odd number of bytes 
         */
        bytesIn = *currSrc;
        bytesIn <<= 8;
        bytesIn |= *(currSrc+1);
        bytesIn <<= 8;
        bytesIn |= *(currSrc+2);
        bytesOut = (bytesIn<<1 | 1) & bytesIn & bytesIn>>1;
        *row2 = *nextSrcRow;
        bytesOut &= (((unsigned int)(*row2))<<8) | 0xffff00ff;
        *currDest = bytesOut&0xff;

        row2++;
        nextSrcRow++;
        *row2 = *nextSrcRow;
        bytesOut &= ((unsigned int)(*row2)) | 0xffffff00;
        *currDest = (bytesOut>>8)&0xff;
        currDest++;
        *currDest = bytesOut&0xff;
    }

    firstSrc += srcParW;
    firstDest += destParW;
   
    /*
     * All rows except the first and last
     */

     if (h > 2) { 
        for(i=0; i < h-2; i++) { 
            currSrc = firstSrc;
            currDest = firstDest;
            nextSrcRow = currSrc + srcParW;
            row0 = firstRow0;
            row2 = firstRow2;

            /* 
             * First byte
             */

            bytesIn = *currSrc;
            bytesIn <<= 8;
            bytesIn |= *(currSrc+1);
            bytesOut = bytesIn<<1 & bytesIn & (bytesIn>>1 | 0x80000000);
            bytesOut &= ((unsigned int)(*row0))<<8;
            *row2 = *nextSrcRow;
            bytesOut &= ((unsigned int)(*row2))<<8;
            *currDest = (bytesOut>>8)&0xff;

            currDest++;
            nextSrcRow++;
            row0++;
            row2++;

            /*
             * Middle bytes
             */
            if (bw > 3 ) {
                DO_N_TIMES(bw/2-1,
                    bytesIn = *currSrc;
                    bytesIn <<= 8;
                    bytesIn |= *(currSrc+1);
                    bytesIn <<= 8;
                    bytesIn |= *(currSrc+2);
                    bytesIn <<= 8;
                    bytesIn |= *(currSrc+3);

                    bytesOut = bytesIn<<1 & bytesIn & bytesIn>>1;
                    bytesOut &= (((unsigned int)(*row0))<<16) | 0xff00ffff;
                    *row2 = *nextSrcRow;
                    bytesOut &= (((unsigned int)(*row2))<<16) | 0xff00ffff;

                    row0++;
                    row2++;
                    nextSrcRow++;
                    bytesOut &= (((unsigned int)(*row0))<<8) | 0xffff00ff;
                    *row2 = *nextSrcRow;
                    bytesOut &= (((unsigned int)(*row2))<<8) | 0xffff00ff;

                    *currDest = (bytesOut>>16)&0xff;
                    currDest++;
                    *currDest = (bytesOut>>8)&0xff;
                    bytesOut <<= 8;
                    currSrc += 2;
                    currDest++;
                    nextSrcRow ++;
                    row0++;
                    row2++;
                );
            } 

            if (bw % 2 == 0) {
                /* 
                 * Last byte if even number of bytes 
                 */
                bytesIn = *currSrc;
                bytesIn <<= 8;
                bytesIn |= *(currSrc+1);
                bytesOut = (bytesIn<<1 | 1) & bytesIn & bytesIn>>1;
                bytesOut &= (unsigned int)(*row0);
                *row2 = *nextSrcRow;
                bytesOut &= (unsigned int)(*row2);
                *currDest = bytesOut&0xff;
             } else {
                 /* 
                  * Last two bytes if odd number of bytes 
                  */
                bytesIn = *currSrc;
                bytesIn <<= 8;
                bytesIn |= *(currSrc+1);
                bytesIn <<= 8;
                bytesIn |= *(currSrc+2);
                bytesOut = (bytesIn<<1 | 1) & bytesIn & bytesIn>>1;
                bytesOut &= (((unsigned int)(*row0))<<8) | 0xffff00ff;
                *row2 = *nextSrcRow;
                bytesOut &= ((unsigned int)(*row2))<<8 | 0xffff00ff;
                *currDest = bytesOut&0xff;

                row0++;
                row2++;
                nextSrcRow++;
                *row2 = *nextSrcRow;
                bytesOut &= (unsigned int)(*row0) | 0xff00;
                bytesOut &= (unsigned int)(*row2) | 0xff00;
                *currDest = (bytesOut>>8)&0xff;
                currDest++;
                *currDest = bytesOut&0xff;
            }

            row0 = firstRow0;
            firstRow0 = row1;
            row1 = firstRow2;
            firstRow2 = row0; 

            firstSrc += srcParW;
            firstDest += destParW;
        }
     } 

     /* 
      * Last row 
      */
     currSrc = firstSrc;
     currDest = firstDest;
     row0 = firstRow0;

     /* 
      * First byte 
      */
     bytesIn = *currSrc;
     bytesIn <<= 8;
     bytesIn |= *(currSrc+1);
     bytesOut = bytesIn<<1 & bytesIn & (bytesIn>>1 | 0x80000000);
     bytesOut &= ((unsigned int)(*row0))<<8;
     *currDest = (bytesOut>>8)&0xff;

     currDest++;
     row0++;
     /*
      * Middle bytes
      */
     if (bw > 3 ) {
         DO_N_TIMES(bw/2-1,
            bytesIn = *currSrc;
            bytesIn <<= 8;
            bytesIn |= *(currSrc+1);
            bytesIn <<= 8;
            bytesIn |= *(currSrc+2);
            bytesIn <<= 8;
            bytesIn |= *(currSrc+3);

            bytesOut = bytesIn<<1 & bytesIn & bytesIn>>1 ;
            bytesOut &= (((unsigned int)(*row0))<<16) | 0xff00ffff;

            row0++;
            nextSrcRow++;
            bytesOut &= (((unsigned int)(*row0))<<8) | 0xffff00ff;

            *currDest = (bytesOut>>16)&0xff;
            currDest++;
            *currDest = (bytesOut>>8)&0xff;
            bytesOut <<= 8;
            currSrc += 2;
            currDest++;
            row0++;
         );
     }

     if (bw % 2 == 0) {
        /* 
         * Last byte if even number of bytes
         */
        bytesIn = *currSrc;
        bytesIn <<= 8;
        bytesIn |= *(currSrc+1);
        bytesOut = (bytesIn<<1 | 1) & bytesIn & bytesIn>>1;
        bytesOut &= ((unsigned int)(*row0));
        *currDest = bytesOut&0xff;
     } else {
        /* 
         * Last two bytes if odd number of bytes 
         */
        bytesIn = *currSrc;
        bytesIn <<= 8;
        bytesIn |= *(currSrc+1);
        bytesIn <<= 8;
        bytesIn |= *(currSrc+2);
        bytesOut = (bytesIn<<1 | 1) & bytesIn & bytesIn>>1;
        bytesOut &= (((unsigned int)(*row0))<<8) | 0xffff00ff;
        *currDest = bytesOut&0xff;

        row0++;
        bytesOut &= (unsigned int)(*row0) | 0xff00;
        *currDest = (bytesOut>>8)&0xff;
        currDest++;
        *currDest = bytesOut&0xff;
     }

    return DVM_VISION_OK;
}
