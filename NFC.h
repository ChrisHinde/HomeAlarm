#include <SPI.h>


#define	uchar	unsigned char
#define	uint	unsigned int


//Maximum length of the array
#define MAX_LEN 16

//4 bytes card serial number, the first 5 bytes for the checksum byte
uchar serNum[8];

const byte CT_NONE = 255;
const byte CT_UNKNOWN = 0;
const byte CT_MIFARE_CLASSIC_1K_CARD = 1;
const byte CT_CARD = CT_MIFARE_CLASSIC_1K_CARD;
const byte CT_MIFARE_ULTRALIGHT = 2;
const byte CT_TAG = CT_MIFARE_ULTRALIGHT;


//MF522 Command word
#define PCD_IDLE              0x00               //NO action; Cancel the current command
#define PCD_AUTHENT           0x0E               //Authentication Key
#define PCD_RECEIVE           0x08               //Receive Data
#define PCD_TRANSMIT          0x04               //Transmit data
#define PCD_TRANSCEIVE        0x0C               //Transmit and receive data,
#define PCD_RESETPHASE        0x0F               //Reset
#define PCD_CALCCRC           0x03               //CRC Calculate

// Mifare_One card command word
# define PICC_REQIDL          0x26               // find the antenna area does not enter hibernation
# define PICC_REQALL          0x52               // find all the cards antenna area
# define PICC_ANTICOLL        0x93               // anti-collision
# define PICC_SElECTTAG       0x93               // election card
# define PICC_AUTHENT1A       0x60               // authentication key A
# define PICC_AUTHENT1B       0x61               // authentication key B
# define PICC_READ            0x30               // Read Block
# define PICC_WRITE           0xA0               // write block
# define PICC_DECREMENT       0xC0               // debit
# define PICC_INCREMENT       0xC1               // recharge
# define PICC_RESTORE         0xC2               // transfer block data to the buffer
# define PICC_TRANSFER        0xB0               // save the data in the buffer
# define PICC_HALT            0x50               // Sleep

#define PICC_ANTICOLL2        0x95		 // anticollision level 2

//And MF522 The error code is returned when communication
#define MI_OK                 0
#define MI_NOTAGERR           1
#define MI_ERR                2


//------------------MFRC522 Register---------------
//Page 0:Command and Status
#define     Reserved00            0x00    
#define     CommandReg            0x01    
#define     CommIEnReg            0x02    
#define     DivlEnReg             0x03    
#define     CommIrqReg            0x04    
#define     DivIrqReg             0x05
#define     ErrorReg              0x06    
#define     Status1Reg            0x07    
#define     Status2Reg            0x08    
#define     FIFODataReg           0x09
#define     FIFOLevelReg          0x0A
#define     WaterLevelReg         0x0B
#define     ControlReg            0x0C
#define     BitFramingReg         0x0D
#define     CollReg               0x0E
#define     Reserved01            0x0F
//Page 1:Command     
#define     Reserved10            0x10
#define     ModeReg               0x11
#define     TxModeReg             0x12
#define     RxModeReg             0x13
#define     TxControlReg          0x14
#define     TxAutoReg             0x15
#define     TxSelReg              0x16
#define     RxSelReg              0x17
#define     RxThresholdReg        0x18
#define     DemodReg              0x19
#define     Reserved11            0x1A
#define     Reserved12            0x1B
#define     MifareReg             0x1C
#define     Reserved13            0x1D
#define     Reserved14            0x1E
#define     SpeedReg        0x1F
//Page 2:CFG    
#define     Reserved20            0x20  
#define     CRCResultRegM         0x21
#define     CRCResultRegL         0x22
#define     Reserved21            0x23
#define     ModWidthReg           0x24
#define     Reserved22            0x25
#define     RFCfgReg              0x26
#define     GsNReg                0x27
#define     CWGsPReg	          0x28
#define     ModGsPReg             0x29
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E
#define     TCounterValueRegL     0x2F
//Page 3:TestRegister     
#define     Reserved30            0x30
#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39  
#define     TestDAC2Reg           0x3A   
#define     TestADCReg            0x3B   
#define     Reserved31            0x3C   
#define     Reserved32            0x3D   
#define     Reserved33            0x3E   
#define     Reserved34		  0x3F
//-----------------------------------------------

#define CT_UNKNOWN 0
#define CT_MIFARE 1
#define CT_TAG 2


//Sector A password, 16 sectors, each sector password 6Byte
uchar sectorKeyA[16][16] = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                            {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                            //{0x19, 0x84, 0x07, 0x15, 0x76, 0x14},
                            {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                          };
uchar sectorNewKeyA[16][16] = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                               {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff,0x07,0x80,0x69, 0x19,0x84,0x07,0x15,0x76,0x14},
                               //you can set another ket , such as  " 0x19, 0x84, 0x07, 0x15, 0x76, 0x14 "
                                //{0x19, 0x84, 0x07, 0x15, 0x76, 0x14, 0xff,0x07,0x80,0x69, 0x19,0x84,0x07,0x15,0x76,0x14},
                                // but when loop, please set the  sectorKeyA, the same key, so that RFID module can read the card
                               {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff,0x07,0x80,0x69, 0x19,0x33,0x07,0x15,0x34,0x14},
                              };
uchar defaultKeyA[16] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uchar madKeyA[16] =     { 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5 };
uchar NDEFKeyA[16] =    { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 };



/*
 * Function Nameï¼šWrite_MFRC5200
 * Function Description: To a certain MFRC522 register to write a byte of data
 * Input Parametersï¼šaddr - register address; val - the value to be written
 * Return value: None
 */
void Write_MFRC522(uchar addr, uchar val)
{
	digitalWrite(chipSelectPin, LOW);

	//Address Formatï¼š0XXXXXX0
	SPI.transfer((addr<<1)&0x7E);
	SPI.transfer(val);
	
	digitalWrite(chipSelectPin, HIGH);
}


/*
 * Function Nameï¼šRead_MFRC522
 * Description: From a certain MFRC522 read a byte of data register
 * Input Parameters: addr - register address
 * Returns: a byte of data read from the
 */
uchar Read_MFRC522(uchar addr)
{
	uchar val;

	digitalWrite(chipSelectPin, LOW);

	//Address Formatï¼š1XXXXXX0
	SPI.transfer(((addr<<1)&0x7E) | 0x80);	
	val =SPI.transfer(0x00);
	
	digitalWrite(chipSelectPin, HIGH);
	
	return val;	
}

/*
 * Function Nameï¼šSetBitMask
 * Description: Set RC522 register bit
 * Input parameters: reg - register address; mask - set value
 * Return value: None
 */
void SetBitMask(uchar reg, uchar mask)  
{
    uchar tmp;
    tmp = Read_MFRC522(reg);
    Write_MFRC522(reg, tmp | mask);  // set bit mask
}


/*
 * Function Name: ClearBitMask
 * Description: clear RC522 register bit
 * Input parameters: reg - register address; mask - clear bit value
 * Return value: None
*/
void ClearBitMask(uchar reg, uchar mask)  
{
    uchar tmp;
    tmp = Read_MFRC522(reg);
    Write_MFRC522(reg, tmp & (~mask));  // clear bit mask
} 

/*
 * Function Nameï¼šAntennaOn
 * Description: Open antennas, each time you start or shut down the natural barrier between the transmitter should be at least 1ms interval
 * Input: None
 * Return value: None
 */
void AntennaOn(void)
{
  uchar temp;

  temp = Read_MFRC522(TxControlReg);
  if (!(temp & 0x03))
  {
    SetBitMask(TxControlReg, 0x03);
  }
}

/*
  * Function Name: AntennaOff
  * Description: Close antennas, each time you start or shut down the natural barrier between the transmitter should be at least 1ms interval
  * Input: None
  * Return value: None
 */
void AntennaOff(void)
{
  ClearBitMask(TxControlReg, 0x03);
}

/*
 * Function Name: ResetMFRC522
 * Description: Reset RC522
 * Input: None
 * Return value: None
 */
void MFRC522_Reset(void)
{
  Write_MFRC522(CommandReg, PCD_RESETPHASE);
}

/*
 * Function Nameï¼šInitMFRC522
 * Description: Initialize RC522
 * Input: None
 * Return value: None
*/
void MFRC522_Init(void)
{
  digitalWrite(NRSTPD,HIGH);

  MFRC522_Reset();
	 	
  //Timer: TPrescaler*TreloadVal/6.78MHz = 24ms
  Write_MFRC522(TModeReg, 0x8D);		//Tauto=1; f(Timer) = 6.78MHz/TPreScaler
  Write_MFRC522(TPrescalerReg, 0x3E);	//TModeReg[3..0] + TPrescalerReg
  Write_MFRC522(TReloadRegL, 30);           
  Write_MFRC522(TReloadRegH, 0);
	
  Write_MFRC522(TxAutoReg, 0x40);		//100%ASK
  Write_MFRC522(ModeReg, 0x3D);		//CRC Initial value 0x6363	???

  //ClearBitMask(Status2Reg, 0x08);		//MFCrypto1On=0
  //Write_MFRC522(RxSelReg, 0x86);		//RxWait = RxSelReg[5..0]
  //Write_MFRC522(RFCfgReg, 0x7F);   		//RxGain = 48dB

  AntennaOn();		//Open the antenna
}


/*
 * Function Name: MFRC522_ToCard
 * Description: RC522 and ISO14443 card communication
 * Input Parameters: command - MF522 command word,
 *			 sendData--RC522 sent to the card by the data
 *			 sendLen--Length of data sent	 
 *			 backData--Received the card returns data,
 *			 backLen--Return data bit length
 * Return value: the successful return MI_OK
 */
uchar MFRC522_ToCard(uchar command, uchar *sendData, uchar sendLen, uchar *backData, uint *backLen)
{
  uchar status = MI_ERR;
  uchar irqEn = 0x00;
  uchar waitIRq = 0x00;
  uchar lastBits;
  uchar n;
  uint i;

  switch (command)
  {
    case PCD_AUTHENT:		//Certification cards close
    {
      irqEn = 0x12;
      waitIRq = 0x10;
      break;
    }
    case PCD_TRANSCEIVE:	//Transmit FIFO data
    {
      irqEn = 0x77;
      waitIRq = 0x30;
      break;
    }
    default:
      break;
  }
   
  Write_MFRC522(CommIEnReg, irqEn|0x80);	//Interrupt request
  ClearBitMask(CommIrqReg, 0x80);			//Clear all interrupt request bit
  SetBitMask(FIFOLevelReg, 0x80);			//FlushBuffer=1, FIFO Initialization
    
  Write_MFRC522(CommandReg, PCD_IDLE);	//NO action; Cancel the current command???

	//Writing data to the FIFO
  for (i=0; i<sendLen; i++)
  {   
    Write_MFRC522(FIFODataReg, sendData[i]);    
  }

  // Execute the command
  Write_MFRC522(CommandReg, command);
  if (command == PCD_TRANSCEIVE)
  {    
    SetBitMask(BitFramingReg, 0x80);		//StartSend=1,transmission of data starts  
  }   
    
  //Waiting to receive data to complete
  i = 2000;	//i according to the clock frequency adjustment, the operator M1 card maximum waiting time 25ms???
  do 
  {
    //CommIrqReg[7..0]
    //Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
    n = Read_MFRC522(CommIrqReg);
    i--;
  } while ((i!=0) && !(n&0x01) && !(n&waitIRq));

  ClearBitMask(BitFramingReg, 0x80);			//StartSend=0
	
  if (i != 0)
  {    
    if(!(Read_MFRC522(ErrorReg) & 0x1B))	//BufferOvfl Collerr CRCErr ProtecolErr
    {
      status = MI_OK;
      if (n & irqEn & 0x01)
      {   
        status = MI_NOTAGERR;			//??   
      }

      if (command == PCD_TRANSCEIVE)
      {
        n = Read_MFRC522(FIFOLevelReg);
        lastBits = Read_MFRC522(ControlReg) & 0x07;
        if (lastBits)
        {   
	  *backLen = (n-1)*8 + lastBits;   
	}
        else
        {   
	  *backLen = n*8;   
	}

        if (n == 0)
        {   
	  n = 1;    
	}
        if (n > MAX_LEN)
        {   
	  n = MAX_LEN;   
	}
				
	// Reading the received data in FIFO
        for (i=0; i<n; i++)
        {   
	  backData[i] = Read_MFRC522(FIFODataReg);    
        }
      }
    }
    else
    {   
      status = MI_ERR;  
    }    
  }
	
  //SetBitMask(ControlReg,0x80);           //timer stops
  //Write_MFRC522(CommandReg, PCD_IDLE); 

  return status;
}


/*
 * Function Nameï¼šMFRC522_Request
 * Description: Find cards, read the card type number
 * Input parameters: reqMode - find cards way
 *			 TagType - Return Card Type
 *			 	0x4400 = Mifare_UltraLight
 *				0x0400 = Mifare_One(S50)
 *				0x0200 = Mifare_One(S70)
 *				0x0800 = Mifare_Pro(X)
 *				0x4403 = Mifare_DESFire
 * Return value: the successful return MI_OK
 */
uchar MFRC522_Request(uchar reqMode, uchar *TagType)
{
  uchar status;  
  uint backBits;			//The received data bits

  Write_MFRC522(BitFramingReg, 0x07);		//TxLastBists = BitFramingReg[2..0]	???
	
  TagType[0] = reqMode;
  status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

  if ((status != MI_OK) || (backBits != 0x10))
  {    
    status = MI_ERR;
  }
   
  return status;
}
/*
 * Function Name: MFRC522_Anticoll
 * Description: Anti-collision detection, reading selected card serial number card
 * Input parameters: serNum - returns 4 bytes card serial number, the first 5 bytes for the checksum byte
 * Return value: the successful return MI_OK
 */
uchar MFRC522_Anticoll(uchar *serNum)
{
  uchar status;
  uchar i;
  uchar serNumCheck=0;
  uint unLen;
    

    //ClearBitMask(Status2Reg, 0x08);		//TempSensclear
    //ClearBitMask(CollReg,0x80);			//ValuesAfterColl
  Write_MFRC522(BitFramingReg, 0x00);		//TxLastBists = BitFramingReg[2..0]
 
  serNum[0] = PICC_ANTICOLL;
  serNum[1] = 0x20;
  status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

  if (status == MI_OK)
  {
    //Check card serial number
    for (i=0; i<4; i++)
    {   
      serNumCheck ^= serNum[i];
    }
    if (serNumCheck != serNum[i])
    {   
      status = MI_ERR;    
    }
  }

  //SetBitMask(CollReg, 0x80);		//ValuesAfterColl=1

  return status;
}


//ANTICOLL cascading level 2
uchar MFRC522_Anticoll2(uchar *serNum)
{
    uchar status;
    uchar i;
    uchar serNumCheck=0;
    uint unLen;
    

    //ClearBitMask(Status2Reg, 0x08);		//TempSensclear
    //ClearBitMask(CollReg,0x80);		//ValuesAfterColl
    Write_MFRC522(BitFramingReg, 0x00);		//TxLastBists = BitFramingReg[2..0]
 
    serNum[0] = PICC_ANTICOLL2;
    serNum[1] = 0x20;
    status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

    if (status == MI_OK)
    {
	 //Verify card serial number
         for (i=0; i<4; i++)
	 {    
	     serNumCheck ^= serNum[i];
	 }
	 if (serNumCheck != serNum[i])
	 {   
	     status = MI_ERR;    
	 }
    }
    //SetBitMask(CollReg, 0x80);		//ValuesAfterColl=1
    return status;
}

/*
 * Function：MFRC522_SelectTag
 * Description：Select card, read card storage volume
 * Input parameter：serNum--Send card serial number
 * sak see ISO14443-3 Table 9 — Coding of SAK
 * return return MI_OK if successed
 */
uchar MFRC522_SelectTag2(uchar *serNum, uchar *sak)
{
    uchar i;
    uchar status;
    uint recvBits;
    uchar buffer[9];

    buffer[0] = PICC_SElECTTAG;
    buffer[1] = 0x70;
    for (i=0; i<5; i++)
    {
    	buffer[i+2] = *(serNum+i);
    }
//    CalulateCRC(buffer, 7, &buffer[7]);		//??
    status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);
    //TODO: the above call returns 2 instead of MI_OK -> why?
    status = MI_OK;

    *sak = buffer[0];
    return status;
}




// Read the id of the card
byte readCardID( byte &length )
{
  uchar status;
  uchar buffer[MAX_LEN];
  byte cardType = CT_NONE;

  //Find cards, return card type
  status = MFRC522_Request(PICC_REQIDL, buffer);

  if (status == MI_OK) {
//    memcpy(serNum, buffer, 5);      // Transfer the data to serNum
    uchar sak = 0;

    //Prevent conflict, return the 4 bytes Serial number of the card
    status = MFRC522_Anticoll(buffer);
    if (status == MI_OK) {
      memcpy(serNum, buffer, 5);     // Transfer the data to serNum
      length = 4;                    // Tell the caller the length of the ID

      // Read the card again and get the "SAK zalue"
      status = MFRC522_SelectTag2(buffer, &sak);

      // All ok?!
      if ( status == MI_OK && ((sak & 0x04) == 0x00) ) {
        if ( (sak & 0x20) == 0x20 ) {
          /********
           * TODO: check what type of card this is etc!!?!
           */

           //ISO/IEC FCD 14443-3: Table 9 — Coding of SAK
#if SERIAL
           Serial.println(" UID complete, PICC compliant with ISO/IEC 14443-4");
#endif
           //send RATS (Request for Answer To Select)
          /* uchar ats[MAX_LEN];
           uint unLen = 0;
           status = MFRC522_RATS(ats, &unLen);
           if (status == MI_OK)
           {
                Serial.println(" ATS: ");
                dumpHex((char*)ats, ats[0]);
                Serial.println("");
           } else
                Serial.println("Not Ok??");*/
        }
      // cascading level 2
      } else {
        // Shift the current values of the ID one step to the left ("popping")
        memcpy(serNum, &serNum[1], 3);//cascading L1

        // Get more data from the tag
        status = MFRC522_Anticoll2(buffer);
        if (status == MI_OK) {
          memcpy(&serNum[3], buffer, 4);    // Store the last of the ID
          length = 7;                       // Tell the caller the length of th ID
        } else {
            return CT_NONE;
        }
      }

      // Check the type of the tag (TODO: Add more types)
      if ( sak == 0x04 ) {
        cardType = CT_MIFARE_ULTRALIGHT;       // Tag
      } else if ( sak == 0x88 ) {
        cardType = CT_MIFARE_CLASSIC_1K_CARD;  // Card
      } else {
#if SERIAL
        Serial.print("Unknown card detected: ");
        Serial.println( sak, HEX );
#endif
        cardType = CT_UNKNOWN;
      }
    }
  }

  return cardType;
}

// Check the ID of the card (serNum) with the valid IDs and return the # of the found ID (if we found it)
byte checkID( byte length )
{
  byte result = CT_NONE;  // Holds the "verdict"
  boolean ok = false;     // Holds the state of the check

  // Loop through the valid IDs (stops if we found an valid ID)
  for (byte id = 0; (id < NUM_OF_IDS) && ! ok ; id++ ) {
    // Start out with "all ok"
    ok = true;
    // Loop through the bytes of the ID (stops if we found a byte that didn't match)
    for ( byte b = 0; ok && (b < length) && (b < MAX_ID_LENGTH); b++ ) {
      // If the current bytes doesn't match
      if ( valid_ids[id][b] != serNum[b] ) {
        // Indicate mismatch and stop this inner loop
        ok = false;
      } // if
    } // for

    // If the whole ID was ok
    if ( ok ) {
      // Store the # of the current ID
      result = id;
    } // if
  } // for

  // Return the result
  return result;
}

/*
#if SERIAL
// Dump the ID to the serial communication
void dumpID(byte* buffer, int len)
{
  for(byte i=0; i < len; i++) {
     char text[4];
     if (i > 0) {
        Serial.print(":"); 
     }
     sprintf(text, "%02X\x00", (byte)(*(buffer + i)));
     Serial.print(text);
  }
}
#endif*/
