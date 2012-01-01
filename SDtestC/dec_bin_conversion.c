/*
 * dec_bin_conversion.c
 *
 *  Created on: Dec 20, 2011
 *      Author: ParallelsWin7
 */


void dec2bin(long i)
{
	char* str; char* p;

	str = malloc( sizeof(long)*8*sizeof(char) );
	p = str;
	while( i > 0 )
	{
		/* bitwise AND operation with the last bit */
		(i & 0x1) ? (*p++='1') : (*p++='0');
		/* bit shift to the right, when there are no
		bits left the value is 0, so the loop ends */
		i >>= 1;
	}
//	while( p-- != str ) /* print out the result backwards */
//		 printf("%c",*p);

	free(str);
}
