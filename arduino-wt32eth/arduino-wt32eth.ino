/* Board: WT32-ETH01 */
/*   data sheet: https://download.bastelgarage.ch/Produkte/WT32-ETH01_datasheet_422282.pdf */

/* MCP4725 DAC 12 bit via I2C */
/*   Library: "Adafruit MCP4725" */

/* ADS1015 ADC 12 bit 4 channels via I2C */
/*   Library: ADS1X15 by Rob Tillaart */

#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include <ADS1X15.h>
#include "ccs32_globals.h"

#define mySDA 15
#define mySCL 14
#define myI2CFreq 400000

uint32_t currentTime;
uint32_t lastTime50ms, lastTime500ms;

TwoWire myI2C = TwoWire(1);
Adafruit_MCP4725 dac;
ADS1015 myADS1015(0x48, &myI2C);

uint8_t testData0;
uint16_t targetVoltage_0V1;
uint16_t presentHighVoltage_0V1;

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

/* This task runs twice per second. */
void task500ms(void) {
  sendTestFrame();
  testData0++;
}

#define mV_at_400V 2470
#define mV_at_0V 828
#define FOURHUNDREDVOLT 4000 /* in 0.1V */

void task50ms(void) {
    int32_t tmp, u_mV;
    tmp = targetVoltage_0V1;
    tmp *=(mV_at_400V-mV_at_0V); /* 2500mV */
    tmp /= FOURHUNDREDVOLT; /* at 400.0V highvoltage */
    /* tmp is now in millivolt */
    tmp += mV_at_0V; /* 830mV is the offset to get 0V highvoltage */
    tmp*=4096; /* 4096 ADC full scale */
    tmp/=3300; /* 3.3V reference voltage */
    if (tmp>4095) tmp=4095;
    if (tmp<0) tmp=0;
    dac.setVoltage(tmp, false);

    /* get present voltage from the ADC */
    u_mV = myADS1015.toVoltage(myADS1015.readADC(0))*1000; /* The ADS1015 has an internal reference,
                                                        and with the default gain it has 6.144V full scale. */
    tmp = u_mV - mV_at_0V; /* at 0V highvoltage, the muehlpower output divided by 5k6 and 8k2 provides 828mV */
    tmp*=FOURHUNDREDVOLT; /* 400.0V */
    tmp/=(mV_at_400V-mV_at_0V); /* at 400V highvoltage, after the divider we have ~2500mV */
    if (tmp<0) tmp=0; /* we do not show negative voltages */
    presentHighVoltage_0V1 = tmp;
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

  Serial.println("initializing ADC...");
  myADS1015.begin();
  if (!myADS1015.isConnected()) {
    Serial.println("Error: ADS1015 is not connected.");
  }
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
  currentTime = millis();
  lastTime50ms = currentTime;
  lastTime500ms = currentTime;
}

void loop() { 
  currentTime = millis();
  if ((currentTime - lastTime50ms)>50) {
    lastTime50ms += 50;
    task50ms();
  }
  if ((currentTime - lastTime500ms)>500) {
    lastTime500ms += 500;
    task500ms();
  }
}
