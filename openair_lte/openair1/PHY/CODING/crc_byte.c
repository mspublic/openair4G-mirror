/* file: crc_byte.c
   purpose: generate 3GPP LTE CRC. Byte-oriented implementation of CRC's
   author: raymond.knopp@eurecom.fr
   date: 21.10.2009 

   Original version by
   P. Humblet
   May 10, 2001
   Modified in June, 2001, to include  the length non multiple of 8
*/


#ifndef USER_MODE
#define __NO_VERSION__

#endif

#include "PHY/types.h"
#include "defs.h"


/*ref 36-212 v8.6.0 , pp 8-9 */
/* the highest degree is set by default */
u32             poly24a = 0xc656fb00;    //1100 0110 0100 1100 1111 1011  D^24 + D^23 + D^18 + D^17 + D^14 + D^11 + D^10 + D^7 + D^6 + D^5 + D^4 + D^3 + D + 1
u32             poly24b = 0x80006300;    // 1000 0000 0000 0000 0110 0011  D^24 + D^23 + D^6 + D^5 + D + 1
u32             poly16 = 0x10210000;    // 0001 0000 0010 0001            D^16 + D^12 + D^5 + 1
u32             poly12 = 0x80F00000;    // 1000 0000 1111                 D^12 + D^11 + D^3 + D^2 + D + 1
u32             poly8 = 0x9B000000;     // 1001 1011                      D^8  + D^7  + D^4 + D^3 + D + 1
/*********************************************************

For initialization && verification purposes, 
   bit by bit implementation with any polynomial

The first bit is in the MSB of each byte

*********************************************************/
unsigned int
crcbit (unsigned char * inputptr, int octetlen, unsigned int poly)
{
  unsigned int             i, crc = 0, c;
  while (octetlen-- > 0) {
    c = (*inputptr++) << 24;
    for (i = 8; i != 0; i--) {
      if ((1 << 31) & (c ^ crc))
        crc = (crc << 1) ^ poly;
      else
        crc <<= 1;
      c <<= 1;
    }
  }
  return crc;
}

/*********************************************************

crc table initialization

*********************************************************/
static unsigned int      crc24aTable[256];
static unsigned int      crc24bTable[256];
static unsigned short      crc16Table[256];
static unsigned short      crc12Table[256];
static unsigned char       crc8Table[256];

void crcTableInit (void)
{
  unsigned char              c = 0;
  do {
    crc24aTable[c] = crcbit (&c, 1, poly24a);
    crc24bTable[c] = crcbit (&c, 1, poly24b);
    crc16Table[c] = (u16) (crcbit (&c, 1, poly16) >> 16);
    crc12Table[c] = (u16) (crcbit (&c, 1, poly12) >> 16);
    crc8Table[c] = (u8) (crcbit (&c, 1, poly8) >> 24);
  } while (++c);
}
/*********************************************************

Byte by byte implementations, 
assuming initial byte is 0 padded (in MSB) if necessary

*********************************************************/
unsigned int
crc24a (unsigned char * inptr, int bitlen)
{

  int             octetlen, resbit;
  unsigned int             crc = 0;
  octetlen = bitlen / 8;        /* Change in octets */
  resbit = (bitlen % 8);
  while (octetlen-- > 0) {
    crc = (crc << 8) ^ crc24aTable[(*inptr++) ^ (crc >> 24)];
  }
  if (resbit > 0)
    crc = (crc << resbit) ^ crc24aTable[((*inptr) >> (8 - resbit)) ^ (crc >> (32 - resbit))];
  return crc;
}

crc24b (unsigned char * inptr, int bitlen)
{

  int             octetlen, resbit;
  unsigned int             crc = 0;
  octetlen = bitlen / 8;        /* Change in octets */
  resbit = (bitlen % 8);
  while (octetlen-- > 0) {
    crc = (crc << 8) ^ crc24bTable[(*inptr++) ^ (crc >> 24)];
  }
  if (resbit > 0)
    crc = (crc << resbit) ^ crc24bTable[((*inptr) >> (8 - resbit)) ^ (crc >> (32 - resbit))];
  return crc;
}

unsigned int
crc16 (unsigned char * inptr, int bitlen)
{
  int             octetlen, resbit;
  unsigned int             crc = 0;
  octetlen = bitlen / 8;        /* Change in octets */
  resbit = (bitlen % 8);
  while (octetlen-- > 0) {
    crc = (crc << 8) ^ (crc16Table[(*inptr++) ^ (crc >> 24)] << 16);
  }
  if (resbit > 0)
    crc = (crc << resbit) ^ (crc16Table[((*inptr) >> (8 - resbit)) ^ (crc >> (32 - resbit))] << 16);
  return crc;
}

unsigned int
crc12 (unsigned char * inptr, int bitlen)
{
  int             octetlen, resbit;
  unsigned int             crc = 0;
  octetlen = bitlen / 8;        /* Change in octets */
  resbit = (bitlen % 8);
  while (octetlen-- > 0) {
    crc = (crc << 8) ^ (crc12Table[(*inptr++) ^ (crc >> 24)] << 16);
  }
  if (resbit > 0)
    crc = (crc << resbit) ^ (crc12Table[((*inptr) >> (8 - resbit)) ^ (crc >> (32 - resbit))] << 16);
  return crc;
}

unsigned int
crc8 (unsigned char * inptr, int bitlen)
{
  int             octetlen, resbit;
  unsigned int             crc = 0;
  octetlen = bitlen / 8;        /* Change in octets */
  resbit = (bitlen % 8);
  while (octetlen-- > 0) {
    crc = crc8Table[(*inptr++) ^ (crc >> 24)] << 24;
  }
  if (resbit > 0)
    crc = (crc << resbit) ^ (crc8Table[((*inptr) >> (8 - resbit)) ^ (crc >> (32 - resbit))] << 24);
  return crc;
}

#ifdef DEBUG_CRC
/*******************************************************************/
/**
   Test code 
********************************************************************/

#include <stdio.h>

main()
{
  u8 test[] = "Thebigredfox";
  crcTableInit();
  printf("%x\n", crcbit(test, sizeof(test) - 1, poly24));
  printf("%x\n", crc24(test, (sizeof(test) - 1)*8));
  printf("%x\n", crcbit(test, sizeof(test) - 1, poly8));
  printf("%x\n", crc8(test, (sizeof(test) - 1)*8));
}
#endif

