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

#define STATE_INITIAL  0
#define STATE_MODEM_SEARCH_ONGOING  1
#define STATE_READY_FOR_SLAC        2
#define STATE_WAITING_FOR_MODEM_RESTARTED  3
#define STATE_WAITING_FOR_SLAC_PARAM_CNF   4
#define STATE_SLAC_PARAM_CNF_RECEIVED      5
#define STATE_BEFORE_START_ATTEN_CHAR      6
#define STATE_SOUNDING                     7
#define STATE_WAIT_FOR_ATTEN_CHAR_IND      8
#define STATE_ATTEN_CHAR_IND_RECEIVED      9
#define STATE_DELAY_BEFORE_MATCH           10
#define STATE_WAITING_FOR_SLAC_MATCH_CNF   11
#define STATE_WAITING_FOR_RESTART2         12
#define STATE_FIND_MODEMS2                 13
#define STATE_WAITING_FOR_SW_VERSIONS      14
#define STATE_READY_FOR_SDP                15
#define STATE_SDP                          16

#define iAmPev 1 /* This project is intended only for PEV mode at the moment. */
#define iAmEvse 0

const uint8_t MAC_BROADCAST[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

char strVersion[200];
uint8_t verLen;
uint8_t sourceMac[6];
uint8_t localModemMac[6];
uint8_t evseMac[6];
uint8_t NID[7];
uint8_t NMK[16];
uint8_t localModemCurrentKey[16];
uint8_t localModemFound;
uint8_t numberOfSoftwareVersionResponses;
uint8_t numberOfFoundModems;
uint8_t pevSequenceState;
uint16_t pevSequenceCyclesInState;
uint16_t pevSequenceDelayCycles;
uint8_t nRemainingStartAttenChar;
uint8_t remainingNumberOfSounds;
uint8_t AttenCharIndNumberOfSounds;
uint8_t SdpRepetitionCounter;
uint8_t isSDPDone;
uint8_t sdp_state;
uint8_t nEvseModemMissingCounter;

void callbackReadyForTcp(uint8_t x) {
  //pevStateMachine_ReInit(); /* let the PEV state machine start the TCP stuff */
}

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

void setNmkAt(uint8_t index) {
  /* sets the Network Membership Key (NMK) at a certain position in the transmit buffer */
  uint8_t i;
  for (i=0; i<16; i++) {
    mytransmitbuffer[index+i]=NMK[i]; // NMK
  }
}

void setNidAt(uint8_t index) {
  /* copies the network ID (NID, 7 bytes) into the wished position in the transmit buffer */
  uint8_t i;
  for (i=0; i<7; i++) {
    mytransmitbuffer[index+i]=NID[i];
  }
}

uint16_t getManagementMessageType(void) {
  /* calculates the MMTYPE (base value + lower two bits), see Table 11-2 of homeplug spec */
  return (myreceivebuffer[16]<<8) + myreceivebuffer[15];
}

void composeGetSwReq(void) {
	/* GET_SW.REQ request, as used by the win10 laptop */
    mytransmitbufferLen = 60;
    cleanTransmitBuffer();
    /* Destination MAC */
    fillDestinationMac(MAC_BROADCAST);
    /* Source MAC */
    fillSourceMac(myMAC);
    /* Protocol */
    mytransmitbuffer[12]=0x88; // Protocol HomeplugAV
    mytransmitbuffer[13]=0xE1; //
    mytransmitbuffer[14]=0x00; // version
    mytransmitbuffer[15]=0x00; // GET_SW.REQ
    mytransmitbuffer[16]=0xA0; // 
    mytransmitbuffer[17]=0x00; // Vendor OUI
    mytransmitbuffer[18]=0xB0; // 
    mytransmitbuffer[19]=0x52; //  
}

void composeSlacParamReq(void) {
	/* SLAC_PARAM request, as it was recorded 2021-12-17 WP charger 2 */
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
    mytransmitbuffer[15]=0x64; // SLAC_PARAM.REQ
    mytransmitbuffer[16]=0x60; // 
    mytransmitbuffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    mytransmitbuffer[18]=0x00; // 
    mytransmitbuffer[19]=0x00; // 
    mytransmitbuffer[20]=0x00; //
    fillSourceMac(myMAC, 21); // 21 to 28: 8 bytes runid. The Ioniq uses the PEV mac plus 00 00.
    mytransmitbuffer[27]=0x00; // 
    mytransmitbuffer[28]=0x00; // 
    // rest is 00
}

void evaluateSlacParamCnf(void) {
  /* As PEV, we receive the first response from the charger. */
  addToTrace("[PEVSLAC] Checkpoint102: received SLAC_PARAM.CNF");
  if (iAmPev) {
    if (pevSequenceState==STATE_WAITING_FOR_SLAC_PARAM_CNF) { //  we were waiting for the SlacParamCnf
      pevSequenceDelayCycles = 4; // original Ioniq is waiting 200ms
      slac_enterState(STATE_SLAC_PARAM_CNF_RECEIVED); // enter next state. Will be handled in the cyclic runSlacSequencer
		}
	}		
}

void composeStartAttenCharInd(void) {
    /* reference: see wireshark interpreted frame from ioniq */
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
    mytransmitbuffer[38] = testData0;
    mytransmitbuffer[39] = testData0+1;
    mytransmitbuffer[40] = testData0+2;
    mytransmitbuffer[41] = testData0+3;

}

void composeNmbcSoundInd(void) {
    /* reference: see wireshark interpreted frame from Ioniq */
    uint8_t i;
    mytransmitbufferLen = 71;
    cleanTransmitBuffer();
    //Destination MAC
    fillDestinationMac(MAC_BROADCAST);
    // Source MAC
    fillSourceMac(myMAC);
    // Protocol
    mytransmitbuffer[12]=0x88; // Protocol HomeplugAV
    mytransmitbuffer[13]=0xE1; //
    mytransmitbuffer[14]=0x01; // version
    mytransmitbuffer[15]=0x76; // NMBC_SOUND.IND
    mytransmitbuffer[16]=0x60; //  
    mytransmitbuffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    mytransmitbuffer[18]=0x00; // 
    mytransmitbuffer[19]=0x00; // apptype
    mytransmitbuffer[20]=0x00; // sectype
    mytransmitbuffer[21]=0x00; // 21 to 37 sender ID, all 00
    mytransmitbuffer[38]=remainingNumberOfSounds; // countdown. Remaining number of sounds. Starts with 9 and counts down to 0.
    fillSourceMac(myMAC, 39); // 39 to 46: runid, filled with MAC of PEV and two bytes 00 00
    mytransmitbuffer[47]=0x00; // 47 to 54: reserved, all 00
    //55 to 70: random number. All 0xff in the ioniq message.
    for (i=55; i<71; i++) { // i in range(55, 71):
        mytransmitbuffer[i]=0xFF;
    }
}

void evaluateAttenCharInd(void) {
  uint8_t i;  
  addToTrace("[PEVSLAC] received ATTEN_CHAR.IND");
  if (iAmPev==1) {
        //addToTrace("[PEVSLAC] received AttenCharInd in state " + str(pevSequenceState))
        if (pevSequenceState==STATE_WAIT_FOR_ATTEN_CHAR_IND) { // we were waiting for the AttenCharInd
            //todo: Handle the case when we receive multiple responses from different chargers.
            //      Wait a certain time, and compare the attenuation profiles. Decide for the nearest charger.
            //Take the MAC of the charger from the frame, and store it for later use.
            for (i=0; i<6; i++) {
                evseMac[i] = myreceivebuffer[6+i]; // source MAC starts at offset 6
            }
            AttenCharIndNumberOfSounds = myreceivebuffer[69];
            //addToTrace("[PEVSLAC] number of sounds reported by the EVSE (should be 10): " + str(AttenCharIndNumberOfSounds)) 
            composeAttenCharRsp();
            addToTrace("[PEVSLAC] transmitting ATTEN_CHAR.RSP...");
            myEthTransmit();               
            pevSequenceState=STATE_ATTEN_CHAR_IND_RECEIVED; // enter next state. Will be handled in the cyclic runSlacSequencer
		    }
	}
}

void composeAttenCharRsp(void) {
    /* reference: see wireshark interpreted frame from Ioniq */
    mytransmitbufferLen = 70;
    cleanTransmitBuffer();
    // Destination MAC
    fillDestinationMac(evseMac);
    // Source MAC
    fillSourceMac(myMAC);
    // Protocol
    mytransmitbuffer[12]=0x88; // Protocol HomeplugAV
    mytransmitbuffer[13]=0xE1; //
    mytransmitbuffer[14]=0x01; // version
    mytransmitbuffer[15]=0x6F; // ATTEN_CHAR.RSP
    mytransmitbuffer[16]=0x60; // 
    mytransmitbuffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    mytransmitbuffer[18]=0x00; // 
    mytransmitbuffer[19]=0x00; // apptype
    mytransmitbuffer[20]=0x00; // sectype
    fillSourceMac(myMAC, 21); // 21 to 26: source MAC
    fillDestinationMac(myMAC, 27); // 27 to 34: runid. The PEV mac, plus 00 00.
    // 35 to 51: source_id, all 00
    // 52 to 68: resp_id, all 00
    // 69: result. 0 is ok
}
		
void composeSlacMatchReq(void) {
    /* reference: see wireshark interpreted frame from Ioniq */
    mytransmitbufferLen = 85;
    cleanTransmitBuffer();
    // Destination MAC
    fillDestinationMac(evseMac);
    // Source MAC
    fillSourceMac(myMAC);
    // Protocol
    mytransmitbuffer[12]=0x88; // Protocol HomeplugAV
    mytransmitbuffer[13]=0xE1; //
    mytransmitbuffer[14]=0x01; // version
    mytransmitbuffer[15]=0x7C; // SLAC_MATCH.REQ
    mytransmitbuffer[16]=0x60; // 
    mytransmitbuffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    mytransmitbuffer[18]=0x00; // 
    mytransmitbuffer[19]=0x00; // apptype
    mytransmitbuffer[20]=0x00; // sectype
    mytransmitbuffer[21]=0x3E; // 21 to 22: length
    mytransmitbuffer[22]=0x00; // 
    // 23 to 39: pev_id, all 00
    fillSourceMac(myMAC, 40); // 40 to 45: PEV MAC
    // 46 to 62: evse_id, all 00
    fillDestinationMac(evseMac, 63); // 63 to 68: EVSE MAC
    fillSourceMac(myMAC, 69); // 69 to 76: runid. The PEV mac, plus 00 00.
    // 77 to 84: reserved, all 00        
}

void evaluateSlacMatchCnf(void) {

}

void composeSetKey(void) {
	/* CM_SET_KEY.REQ request */
  /* From example trace from catphish https://openinverter.org/forum/viewtopic.php?p=40558&sid=9c23d8c3842e95c4cf42173996803241#p40558
     Table 11-88 in the homeplug_av21_specification_final_public.pdf */
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
  mytransmitbuffer[15]=0x08; // CM_SET_KEY.REQ
  mytransmitbuffer[16]=0x60; // 
  mytransmitbuffer[17]=0x00; // frag_index
  mytransmitbuffer[18]=0x00; // frag_seqnum
  mytransmitbuffer[19]=0x01; // 0 key info type

  mytransmitbuffer[20]=0xaa; // 1 my nonce
  mytransmitbuffer[21]=0xaa; // 2
  mytransmitbuffer[22]=0xaa; // 3
  mytransmitbuffer[23]=0xaa; // 4

  mytransmitbuffer[24]=0x00; // 5 your nonce
  mytransmitbuffer[25]=0x00; // 6
  mytransmitbuffer[26]=0x00; // 7
  mytransmitbuffer[27]=0x00; // 8
        
  mytransmitbuffer[28]=0x04; // 9 nw info pid
        
  mytransmitbuffer[29]=0x00; // 10 info prn
  mytransmitbuffer[30]=0x00; // 11
  mytransmitbuffer[31]=0x00; // 12 pmn
  mytransmitbuffer[32]=0x00; // 13 cco cap
  setNidAt(33); // 14-20 nid  7 bytes from 33 to 39
                //          Network ID to be associated with the key distributed herein.
                //          The 54 LSBs of this field contain the NID (refer to Section 3.4.3.1). The
                //          two MSBs shall be set to 0b00.
  mytransmitbuffer[40]=0x01; // 21 peks (payload encryption key select) Table 11-83. 01 is NMK. We had 02 here, why???
                               // with 0x0F we could choose "no key, payload is sent in the clear"
  setNmkAt(41); 
  #define variation 0
  mytransmitbuffer[41]+=variation; // to try different NMKs
  // and three remaining zeros
}

void evaluateSetKeyCnf(void) {
 
}

void composeGetKey(void) {
		/* CM_GET_KEY.REQ request
         from https://github.com/uhi22/plctool2/blob/master/listen_to_eth.c
         and homeplug_av21_specification_final_public.pdf */
    mytransmitbufferLen = 60;
    cleanTransmitBuffer();
    // Destination MAC
    fillDestinationMac(MAC_BROADCAST);
    // Source MAC
    fillSourceMac(myMAC);
    // Protocol
    mytransmitbuffer[12]=0x88; // Protocol HomeplugAV
    mytransmitbuffer[13]=0xE1;
    mytransmitbuffer[14]=0x01; // version
    mytransmitbuffer[15]=0x0C; // CM_GET_KEY.REQ https://github.com/uhi22/plctool2/blob/master/plc_homeplug.h
    mytransmitbuffer[16]=0x60; // 
    mytransmitbuffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    mytransmitbuffer[18]=0x00; //        
    mytransmitbuffer[19]=0x00; // 0 Request Type 0=direct
    mytransmitbuffer[20]=0x01; // 1 RequestedKeyType only "NMK" is permitted over the H1 interface.
                               //           value see HomeplugAV2.1 spec table 11-89. 1 means AES-128.
                                       
    setNidAt(21); // NID starts here (table 11-91 Homeplug spec is wrong. Verified by accepted command.)
    mytransmitbuffer[28]=0xaa; // 10-13 mynonce. The position at 28 is verified by the response of the devolo.
    mytransmitbuffer[29]=0xaa; // 
    mytransmitbuffer[30]=0xaa; // 
    mytransmitbuffer[31]=0xaa; // 
    mytransmitbuffer[32]=0x04; // 14 PID. According to  ISO15118-3 fix value 4, "HLE protocol"
    mytransmitbuffer[33]=0x00; // 15-16 PRN Protocol run number
    mytransmitbuffer[34]=0x00; // 
    mytransmitbuffer[35]=0x00; // 17 PMN Protocol message number
}

void evaluateGetKeyCnf(void) {

}

void sendTestFrame(void) {
  //composeGetSwReq();
  composeStartAttenCharInd();
  myEthTransmit();
}

void evaluateGetSwCnf(void) {
 
}

uint8_t isEvseModemFound(void) {
  /* todo: look whether the MAC of the EVSE modem is in the list of detected modems */
  /* as simple solution we say: If we see two modems, then it should be one
     local in the car, and one in the charger. */
  return numberOfFoundModems>1;
}

void slac_enterState(int n) {
  addToTrace("[PEVSLAC] from " + String(pevSequenceState) + " entering " + String(n));
  pevSequenceState = n;
  pevSequenceCyclesInState = 0;
}

int isTooLong(void) {
  /* The timeout handling function. */
  return (pevSequenceCyclesInState > 500);
}

void runSlacSequencer(void) {

}

void runSdpStateMachine(void) {

}

void evaluateReceivedHomeplugPacket(void) {
  switch (getManagementMessageType()) {
    case CM_GET_KEY + MMTYPE_CNF:    evaluateGetKeyCnf();    break;   
    case CM_SLAC_MATCH + MMTYPE_CNF: evaluateSlacMatchCnf(); break;
    case CM_SLAC_PARAM + MMTYPE_CNF: evaluateSlacParamCnf(); break;
    case CM_ATTEN_CHAR + MMTYPE_IND: evaluateAttenCharInd(); break;
    case CM_SET_KEY + MMTYPE_CNF:    evaluateSetKeyCnf();    break;
    case CM_GET_SW + MMTYPE_CNF:     evaluateGetSwCnf();     break;
  }
}

int homeplug_sanityCheck(void) {
  if (pevSequenceState>STATE_SDP) {
    addToTrace("ERROR: Sanity check of the homeplug state machine failed." + String(pevSequenceState));    
    return -1;
  }
  if (sdp_state>=2) {
    addToTrace("ERROR: Sanity check of the SDP state machine failed." + String(sdp_state));    
    return -1;
  }
  return 0;  
}

void homeplugInit(void) {
  pevSequenceState = STATE_READY_FOR_SLAC;
  pevSequenceCyclesInState = 0;
  pevSequenceDelayCycles = 0;
  numberOfSoftwareVersionResponses = 0;
  numberOfFoundModems = 0;
}
