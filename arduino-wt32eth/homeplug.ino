/* Homeplug message handling */

#define CM_SET_KEY  0x6008
#define CM_GET_KEY  0x600C
#define CM_SC_JOIN  0x6010
#define CM_CHAN_EST  0x6014
#define CM_TM_UPDATE  0x6018
#define CM_AMP_MAP  0x601C
#define CM_BRG_INFO  0x6020
#define CM_CONN_NEW  0x6024
#define CM_CONN_REL  0x6028
#define CM_CONN_MOD  0x602C
#define CM_CONN_INFO  0x6030
#define CM_STA_CAP  0x6034
#define CM_NW_INFO  0x6038
#define CM_GET_BEACON  0x603C
#define CM_HFID  0x6040
#define CM_MME_ERROR  0x6044
#define CM_NW_STATS  0x6048
#define CM_SLAC_PARAM  0x6064
#define CM_START_ATTEN_CHAR  0x6068
#define CM_ATTEN_CHAR  0x606C
#define CM_PKCS_CERT  0x6070
#define CM_MNBC_SOUND  0x6074
#define CM_VALIDATE  0x6078
#define CM_SLAC_MATCH  0x607C
#define CM_SLAC_USER_DATA  0x6080
#define CM_ATTEN_PROFILE  0x6084
#define CM_GET_SW  0xA000

#define MMTYPE_REQ  0x0000
#define MMTYPE_CNF  0x0001
#define MMTYPE_IND  0x0002
#define MMTYPE_RSP  0x0003

const uint8_t MAC_BROADCAST[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

uint8_t sourceMac[6];
uint8_t localModemMac[6];
uint8_t evseMac[6];
uint16_t nSpecialMessageTxAliveCounter;

/* Extracting the EtherType from a received message. */
uint16_t getEtherType(uint8_t *messagebufferbytearray) {
  uint16_t etherType=0;
  etherType=messagebufferbytearray[12]*256 + messagebufferbytearray[13];
  return etherType;
}

void fillSourceMac(const uint8_t *mac, uint8_t offset=6) {
 /* at offset 6 in the ethernet frame, we have the source MAC.
    we can give a different offset, to re-use the MAC also in the data area */
  memcpy(&mytransmitbuffer[offset], mac, 6); 
}

void fillDestinationMac(const uint8_t *mac, uint8_t offset=0) {
 /* at offset 0 in the ethernet frame, we have the destination MAC.
    we can give a different offset, to re-use the MAC also in the data area */
  memcpy(&mytransmitbuffer[offset], mac, 6); 
}

void cleanTransmitBuffer(void) {
  /* fill the complete ethernet transmit buffer with 0x00 */
  int i;
  for (i=0; i<MY_ETH_TRANSMIT_BUFFER_LEN; i++) {
    mytransmitbuffer[i]=0;
  }
}

uint16_t getManagementMessageType(void) {
  /* calculates the MMTYPE (base value + lower two bits), see Table 11-2 of homeplug spec */
  return (myreceivebuffer[16]<<8) + myreceivebuffer[15];
}

void composeStartAttenCharIndAsSpecialMessage(void) {
    mytransmitbufferLen = 60;
    cleanTransmitBuffer();
    // Destination MAC
    fillDestinationMac(MAC_BROADCAST);
    // Source MAC
    fillSourceMac(myMAC);
    // Protocol
    mytransmitbuffer[12]=0x88; // Protocol HomeplugAV
    mytransmitbuffer[13]=0xE1; //
    mytransmitbuffer[14]=0x01; // version
    mytransmitbuffer[15]=0x6A; // START_ATTEN_CHAR.IND
    mytransmitbuffer[16]=0x60; // 
    mytransmitbuffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    mytransmitbuffer[18]=0x00; // 
    mytransmitbuffer[19]=0x00; // apptype
    mytransmitbuffer[20]=0x00; // sectype
    mytransmitbuffer[21]=0x0a; // number of sounds: 10
    mytransmitbuffer[22]=6; // timeout N*100ms. Normally 6, means in 600ms all sounds must have been tranmitted.
                            // Todo: As long we are a little bit slow, lets give 1000ms instead of 600, so that the
                            // charger is able to catch it all.
    mytransmitbuffer[23]=0x01; // response type 
    fillSourceMac(myMAC, 24); // 24 to 29: sound_forwarding_sta, MAC of the PEV
    fillSourceMac(myMAC, 30); // 30 to 37: runid, filled with MAC of PEV and two bytes 00 00
    // rest is 00 normally. We use it here for some data transmission.
    mytransmitbuffer[38] = 0xDC;
    mytransmitbuffer[39] = 0x55;
    mytransmitbuffer[40] = 0xAA;
    mytransmitbuffer[41] = (uint8_t)(targetVoltage_0V1>>8);
    mytransmitbuffer[42] = (uint8_t)targetVoltage_0V1;
    mytransmitbuffer[43] = (uint8_t)(presentHighVoltage_0V1>>8);
    mytransmitbuffer[44] = (uint8_t)presentHighVoltage_0V1;
    mytransmitbuffer[45] = (uint8_t)(nSpecialMessageTxAliveCounter>>8);
    mytransmitbuffer[46] = (uint8_t)nSpecialMessageTxAliveCounter;

}

void evaluateAttenCharIndAsSpecialMessage(void) {
  uint16_t u1, u2, i1, i2;  
  addToTrace("received ATTEN_CHAR.IND, treating as special message");
  if ((myreceivebuffer[71]==0xAF) && (myreceivebuffer[72]==0xFE) && (myreceivebuffer[73]==0xDC)) {
    addToTrace("special message header is correct");
    u1 = myreceivebuffer[74]; /* MSB */ u1<<=8;
    u1 |= myreceivebuffer[75]; /* LSB */
    u2 = myreceivebuffer[76]; /* MSB */ u2<<=8;
    u2 |= myreceivebuffer[77]; /* LSB */
    //addToTrace("u1=" + String(u1) + " u2=" + String(u2));
    if (u1==u2) {
      targetVoltage_0V1 = u1;
      addToTrace("targetVoltage_0V1=" + String(targetVoltage_0V1));
    }
  }
}

void sendTestFrame(void) {
  composeStartAttenCharIndAsSpecialMessage();
  myEthTransmit();
}

void evaluateReceivedHomeplugPacket(void) {
  switch (getManagementMessageType()) {
    case CM_ATTEN_CHAR + MMTYPE_IND: evaluateAttenCharIndAsSpecialMessage(); break;
  }
}

void homeplugInit(void) {
}
