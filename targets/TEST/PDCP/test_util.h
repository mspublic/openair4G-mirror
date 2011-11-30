
/*
 * Helper methods for PDCP test code
 *
 * Author: Baris Demiray <baris.demiray@eurecom.fr>
 */

#ifndef TEST_UTIL_H
#define TEST_UTIL_H

/*
 * Prints binary representation of given octet prepending 
 * passed log message
 *
 * @param Octet as an unsigned character
 * @return None
 */
void print_binary_representation(unsigned char* message, unsigned char byte)
{
  unsigned char index = 0;
  unsigned char mask = 0x80;

  printf("%s", message);
  for (index = 0; index < 8; ++index) {
    if (byte & mask) printf("1");
    else printf("0");

    mask /= 2;
  }
  printf("\n");
}

#endif
