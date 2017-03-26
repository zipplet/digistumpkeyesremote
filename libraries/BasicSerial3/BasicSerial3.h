/* Optimized half-duplex serial uart implementation
 * timing within 2% using at 230.4kbps @ 8Mhz 
 * @author: Ralph Doncaster
 * @version: $Id$
 */

//#define BAUD_RATE 115200

// Works better on digistump boards with 57600 baud in my experience
#define BAUD_RATE 57600

// Original CPU speed independent code
/*
#ifdef F_CPU
  // account for integer truncation by adding 3/2 = 1.5
  #define TXDELAY (((F_CPU/BAUD_RATE)-7 +1.5)/3)
  #define RXDELAY (((F_CPU/BAUD_RATE)-5 +1.5)/3)
  #define RXROUNDED (((F_CPU/BAUD_RATE)-5 +2)/3)
#else
  #error CPU frequency F_CPU undefined
#endif
*/

// Force to 16MHz, otherwise it compiles for 16.5MHz for USB but this code will run without USB connected - override
#define F_CPU2  16000000
#define TXDELAY (((F_CPU2/BAUD_RATE)-7 +1.5)/3)
#define RXDELAY (((F_CPU2/BAUD_RATE)-5 +1.5)/3)
#define RXROUNDED (((F_CPU2/BAUD_RATE)-5 +2)/3)

#if RXROUNDED > 127
  #error low baud rates unsupported - use higher BAUD_RATE
#endif

extern "C" {
void TxTimedByte(char, char);
char RxTimedByte(char, char); /* 1.5 bit delay, 1 bit delay */
}

#define TxByte(C) TxTimedByte(C , TXDELAY)
#define RxByte() RxTimedByte((RXDELAY*1.5)-2.5, RXDELAY)

