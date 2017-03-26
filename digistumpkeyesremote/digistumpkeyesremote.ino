/**********************************************************************************************
 * NEC Infrared Remote Receiver for Digistump (c) Michael Nixon 2017.
 * For digistump basic boards.
 * 
 * Libraries required:
 * BasicSerial3 (provided as a subdirectory - please place this in your libraries)
 * 
 * Shoutouts / credits go to:
 * http://nerdralph.blogspot.jp/2014/01/avr-half-duplex-software-uart.html - for the BasicSerial3
 * library required for this to operate.
 * http://ediy.com.my/blog/item/74-digispark-infrared-receiver - for the infrared NEC decoder
 * information and code I modified here.
 * 
 * Connect P2 to the IR receiver.
 * Connect P3 to the UART RX.
 * The baud rate is 57600 with my modified BasicSerial3 library that I provide with this code.
 * 1 stop bit, 8 data bits, no flow control. You can connect the RX line directly to a 5V
 * device, or use a level shifter / resistor divider for a 3.3V device.
 * 
 * Sends:
 * At powerup: \nOK\n
 * Data received: IR:<n>\n (e.g. IR:40) - where <n> is a 2 digit hex code.
 **********************************************************************************************/

#include <BasicSerial3.h>

#define PIN_LED     1
#define PIN_IR_RX   2
#define PIN_UART    3

/* These are the original timings provided by the guide I started with.
int start_bit = 2200; //Start bit threshold (Microseconds)
int bin_1 = 1000; //Binary 1 threshold (Microseconds)
int bin_0 = 400; //Binary 0 threshold (Microseconds)
*/

// Tighter timings that seem to work better
int start_bit = 3000;   // Start bit threshold (Microseconds)
int bin_1     = 1400;   // Binary 1 threshold (Microseconds)
int bin_0     = 500;    // Binary 0 threshold (Microseconds)

// How many bits per block (NEC protocol)
const byte BIT_PER_BLOCK = 32;

int16_t getIRKey();

// --------------------------------------------------------------------------------------
// Setup function
// --------------------------------------------------------------------------------------
void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_IR_RX, INPUT);
  pinMode(PIN_UART, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  // Let the host know we are OK. Mostly for debugging.
  serOut("\nOK\n");
}

// --------------------------------------------------------------------------------------
// Serial string output from SRAM
// --------------------------------------------------------------------------------------
void serOut(const char* str)
{
   while (*str) TxByte (*str++);
}

// --------------------------------------------------------------------------------------
// Main loop
// --------------------------------------------------------------------------------------
void loop() {
  char temp[10];
  uint32_t key;

  // The next function blocks forever until we get something
  key = getIRKey();

  // -1 is an error code
  if ((key != 0) && (key != -1)) {
    // We pulse the LED briefly for debugging
    digitalWrite(PIN_LED, HIGH);
    sprintf(temp, "IR:%02X\n", key);
    serOut(temp);
    digitalWrite(PIN_LED, LOW);
  }
}
 
// --------------------------------------------------------------------------------------
// Wait for and decode an IR signal (NEC)
// --------------------------------------------------------------------------------------
int16_t getIRKey() {
  uint32_t data[BIT_PER_BLOCK];
  uint8_t i;
  uint8_t result = 0;
  
  while(pulseIn(PIN_IR_RX, HIGH) < start_bit); // Wait for a start bit, blocking

  // 32 data bits
  for(i = 0; i < BIT_PER_BLOCK; i++) {
    data[i] = pulseIn(PIN_IR_RX, HIGH, 5000);
    if (!data[i]) {
      // Timeout
      return -1;
    }
  }

  // Parse data
  for(i = 0 ; i < BIT_PER_BLOCK ; i++) {
    if (data[i] > bin_1) {
      data[i] = 1;
    } else if(data[i] > bin_0) {
      data[i] = 0;
    } else {
      return -1; // Invalid data
    }
  }
  
  // NEC protocol, command data is from bits 16 to bit 24 (8 bits)
  // Convert binary data to a byte
  for(i = 16; i < 24; i++) {
    if (data[i] == 1) {
      result |= (1 << i - 16);
    }
  }

  // Finally return the valid data
  return result;
}
