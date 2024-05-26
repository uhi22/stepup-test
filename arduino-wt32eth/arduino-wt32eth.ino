/* Board: WT32-ETH01 */
/*   data sheet: https://download.bastelgarage.ch/Produkte/WT32-ETH01_datasheet_422282.pdf */

/* MCP4725 DAC 12 bit via I2C */
/*   Library: "Adafruit MCP4725" */
#include <Wire.h>
#include <Adafruit_MCP4725.h>

#include "ccs32_globals.h"

#define mySDA 15
#define mySCL 14
#define myI2CFreq 400000

TwoWire myI2C = TwoWire(1);
Adafruit_MCP4725 dac;

uint8_t testData0;

/**********************************************************/
/* The logging macros and functions */
#undef log_v
#undef log_e
#define log_v(format, ...) log_printf(ARDUHAL_LOG_FORMAT(V, format), ##__VA_ARGS__)
#define log_e(format, ...) log_printf(ARDUHAL_LOG_FORMAT(E, format), ##__VA_ARGS__)

void addToTrace_chararray(char *s) {
  log_v("%s", s);  
}

void addToTrace(String strTrace) {
  //Serial.println(strTrace);  
  log_v("%s", strTrace.c_str());  
}

void showAsHex(uint8_t *arr, uint16_t len, char *info) {
 char strTmp[10];
 #define MAX_RESULT_LEN 700
 char strResult[MAX_RESULT_LEN];
 uint16_t i;
 sprintf(strResult, "%s has %d bytes:", info, len);
 for (i=0; i<len; i++) {
  sprintf(strTmp, "%02hx ", arr[i]);
  if (strlen(strResult)<MAX_RESULT_LEN-10) {  
    strcat(strResult, strTmp);
  } else {
    /* does not fit. Just ignore the remaining bytes. */
  }
 }
 addToTrace_chararray(strResult);
} 

void setup() {
  Serial.begin(115200);
  Serial.println("Started.");
  delay(500);
  /* https://wolles-elektronikkiste.de/en/how-to-use-the-i2c-interfaces-of-the-esp32 */
  //TwoWire myI2C = TwoWire(1);
  myI2C.begin(mySDA, mySCL, myI2CFreq);
  delay(500);

  /* MCP4725A1 the address is 0x62 (default) or 0x63 (ADDR pin tied to VCC) */
  // For MCP4725A0 the address is 0x60 or 0x61
  // For MCP4725A2 the address is 0x64 or 0x65
  Serial.println("initializing DAC...");
  dac.begin(0x60, &myI2C);
  Serial.println("done");
  Serial.println("Initializing ethernet...");
  if (!initEth()) {
    log_v("Error: Ethernet init failed.");
    while (1);
  }
  Serial.println("done");
  delay(100);
  //dac.setVoltage(1000, false);
  //Serial.println("done");
  //delay(500);
}

void loop() { 
    int32_t counter;
    // Run through the full 12-bit scale for a triangle wave
    for (counter = 0; counter < 4095; counter+=2) {
      dac.setVoltage(counter, false);
      delay(1);
    }
    for (counter = 4095; counter > 0; counter-=2) {
      dac.setVoltage(counter, false);
      delay(1);
    }
    sendTestFrame();
    testData0++;
}
