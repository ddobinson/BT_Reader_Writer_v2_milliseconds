

//LIBRARIES
#include <SPI.h>
#include <Wire.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>  //Library extends MFRC522.h to support RATS for ISO-14443-4 PICC.
#include <LCD5110_Graph.h> 
#include <DS3232RTC.h>
#include <TimeLib.h>
//#include <Streaming.h>      //not sure this is needed? Dan
#include "pitches.h"     //if error message involving pitches.h comes up then make sure pitches.h file is in the same directory as the sketch
#include "iTimeAfricaV2.h"
#include <SoftwareSerial.h>
#include <stdlib.h>
#include <EEPROM.h>

//GLOBAL DEFINITIONS
int tagCount = 0;                                           // Cont that incremetns on eatch tag read
int flashLed = LOW;                                         // LED state to implement flashing
int typeIndex = 0xFF;                                       // Index for pod identity in the pod Type List
unsigned long sqCounter = 0;                                // Couner that increments in intterup routine
unsigned long baseTimeS;                                    // Base time in sec from RTC to withc we sync the ms count
bool battGood = 1;
BTProg thisPod ;

// RFID COMPONENT Create MFRC522 instance
MFRC522 mfrc522(RFID_CS, LCD_RST);                            // PH get pins from PCB header file 
MFRC522::MIFARE_Key key;

// LCD COMPONENT
// LCD5110 myGLCD(4,5,6,A1,7); V2.0 green pcb
// LCD5110 myGLCD(4,5,6,7,8); (v1.0 blue PCB)
LCD5110 myGLCD(LCD_CLK, LCD_DIN , LCD_COM , LCD_RST, LCD_CE);   // PH get pins from PCB header file
extern unsigned char TinyFont[];      // for 5110 LCD display
extern unsigned char SmallFont[];     // for 5110 LCD display
extern unsigned char MediumNumbers[]; // for 5110 LCD display
extern unsigned char BigNumbers[];    // for 5110 LCD display

// BLUETOOTH COMPONENT
SoftwareSerial BT(UART_RX, UART_TX); // creates a "virtual" serial port/UART 
int progid = 0;      //define program ID for bluetooth programs defined in global definations (1,2,3,4 etc)           

/*
 * BELOW IS STATION/POD IDENTIFICATION!
 * 
 * Uncomment the appropriate line to identify this station/pod (Don't use this code for finish PODS!)
 */
// move to header file
// String progid = ("BT writer/reader");       // "BT writer/reader"

  // variables used for converting epoch time to readable time
unsigned long temp0=0, temp1=0, temp2=0, temp3=0, temp4=0,
              temp5=0, temp6=0, temp7=0, temp8=0, temp9=0,
              temp10=0, hours=0, mins=0, secs=0, MilliS=0;

//epoch time storage variables
unsigned long ss1Start=0, ss1Finish=0, SS1Time=0,   
              ss2Start=0, ss2Finish=0, SS2Time=0,
              ss3Start=0, ss3Finish=0, SS3Time=0,
              ss4Start=0, ss4Finish=0, SS4Time=0,
              ss5Start=0, ss5Finish=0, SS5Time=0,
              ss6Start=0, ss6Finish=0, SS6Time=0,
              SS1TimeMilliS=0, SS2TimeMilliS=0, SS3TimeMilliS=0,
              SS4TimeMilliS=0, SS5TimeMilliS=0, SS6TimeMilliS=0,
              totalRaceTime=0;
              
byte buffer[18];

void setup() 
{
    Wire.begin();                                         // Init I2C buss
 
    // Serial.begin(57600);             // PH to use status LED serial needs to be disbaled
    BT.begin(9600);                     // Initialize serial communications with BT (Check default Baud Rate of BT module)
    myGLCD.InitLCD();                                     // initialise LCD
    initIoPins();
    initMfrc522();
    configrePod();
}

int updateCnt =0;
void loop() 
{
    updateLcdScreen();                            // invoke LCD Screen fucntion
    // when setSyncInterval(interval); runs out the status is changed to "timeNeedsSync"
    if ( timeStatus()  == timeNeedsSync ) {      
        setSyncProvider(RTC.get);           // manual resynch is done
    }  
  if ( ! mfrc522.PICC_IsNewCardPresent())     // Look for new cards
      return;
 
  if ( ! mfrc522.PICC_ReadCardSerial())       // Select one of the cards
      return;
  else {    
      if(BTProgList[typeIndex].progid == 1){             // Runs BT program 1
        writeRiderData();   //If value is 1 then invoke writeriderdata function
 
      }
      
      if(BTProgList[typeIndex].progid == 2){         // Runs BT program 2
             readCardData();     //If value is 2 then invoke readcard function

  mfrc522.PICC_HaltA();                       // Halt PICC
  mfrc522.PCD_StopCrypto1();                  // Stop encryption on PCD     
  } 
     
}
/*
// interupt is triggered on up and down flanks, we only want one flank
ISR (PCINT1_vect) {     // handle pin change interrupt for A0 to A5 here
    if ( digitalRead(SQ_WAVE) == HIGH) {
        sqCounter ++;                 
    }
}
*/
void readCardData()
{

  byte size = sizeof(buffer);
  byte status;
  byte block;


/*
 * FIRST NAME
 */
 
  block = 1;                                                                                 // first name

// Authentication
 
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,block, &key, &(mfrc522.uid));
   
    if (status != MFRC522::STATUS_OK){ 
//       BT.print(F("PCD_Authenticate() failed: "));
//       BT.println(mfrc522.GetStatusCodeName(status));
         BT.println(F("Error-ReScan"));
         BT.println(); 
        return;  }

// Read data from the block into buffer
    
    status = mfrc522.MIFARE_Read(block, buffer, &size);
    if (status != MFRC522::STATUS_OK){
         BT.println(F("Error-ReScan"));
         BT.println();  
    return;   }
  
// Write buffer to console
  
    for (uint8_t i = 0; i < 16; i++) 
      {
        BT.write(buffer[i]) ;   //writes data byte by byte to console
//        myGLCD.print(temp10), CENTER, 30);   //writes data byte by byte to console
      }

BT.print("\t") ;             //tab separator

/*
 * SURNAME
 */
/* 
block = 2;

// Authentication
 
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,block, &key, &(mfrc522.uid));
   
    if (status != MFRC522::STATUS_OK){ 
//       BT.print(F("PCD_Authenticate() failed: "));
//       BT.println(mfrc522.GetStatusCodeName(status));
         BT.println(F("Error-ReScan"));
         BT.println(); 
        return;  }

// Read data from the block into buffer
    
    status = mfrc522.MIFARE_Read(block, buffer, &size);
    if (status != MFRC522::STATUS_OK){ 

          return;   }

// Write buffer to console
  
    for (uint8_t i = 0; i < 16; i++) 
      {
        BT.write(buffer[i]) ;   //writes data byte by byte to console
      }

BT.print("\t") ;             //tab separator 
*/
/*
 * WATCH NUMBER
 */
 
block = 4;

// Authentication
 
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,block, &key, &(mfrc522.uid));
   
    if (status != MFRC522::STATUS_OK){ 
//       BT.print(F("PCD_Authenticate() failed: "));
//       BT.println(mfrc522.GetStatusCodeName(status));
         BT.println(F("Error-ReScan"));
         BT.println(); 
        return;  }
 
// Read data from the block into buffer
    
    status = mfrc522.MIFARE_Read(block, buffer, &size);
    if (status != MFRC522::STATUS_OK){
         BT.println(F("Error-ReScan"));
         BT.println();  
    return;   }

// Write buffer to console
  
    for (uint8_t i = 0; i < 16; i++) 
      {
        BT.write(buffer[i]) ;   //writes data byte by byte to console
      }

BT.print("\t") ;             //tab separator

/*
 * STAGE 1 START TIME   // reads SS1 Start Time from PICC  block 8 and stores in buffer
 */
 
block = 8;

// Authentication
 
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,block, &key, &(mfrc522.uid));
  
    if (status != MFRC522::STATUS_OK){ 
//       BT.print(F("PCD_Authenticate() failed: "));
//       BT.println(mfrc522.GetStatusCodeName(status));
         BT.println(F("Error-ReScan"));
         BT.println(); 
        return;  }
 
// Read data from the block into buffer
    
    status = mfrc522.MIFARE_Read(block, buffer, &size);
    if (status != MFRC522::STATUS_OK){
         BT.println(F("Error-ReScan"));
         BT.println();  
    return;   }

// Convert buffer type byte to unsigned long int and store

  buffer2epoch();
  ss1Start = temp10;

/*
 * STAGE 1 FINISH TIME
 * // reads SS1 Finish Time from PICC  block 9, then subtracts ss1 finish from ss1 start and calculates SS1 Time
 * // and then downloads SS1 Time to BT serial
 */
 
  block = 9;

// Authentication
 
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,block, &key, &(mfrc522.uid));
   
    if (status != MFRC522::STATUS_OK){ 
//       BT.print(F("PCD_Authenticate() failed: "));
//       BT.println(mfrc522.GetStatusCodeName(status));
         BT.println(F("Error-ReScan"));
         BT.println(); 
        return;  }
 
// Read data from the block into buffer
    
    status = mfrc522.MIFARE_Read(block, buffer, &size);
    if (status != MFRC522::STATUS_OK){
         BT.println(F("Error-ReScan"));
         BT.println();  
    return;   }

// Convert buffer type byte to unsigned long int and store

  buffer2epoch();
  ss1Finish = temp10;
  
  if (ss1Start > ss1Finish && ss1Finish !=0)                                        // if ss1Start bigger than ss1Finish and SS1Finish is not equal to zero, then write 'error'
     {
      BT.print("ERROR") ;                                         // write ERROR to BT
      SS1TimeMilliS = 0;                                                  // set stage time to zero for total count
    }
  else  
  if (ss1Start != 0 && ss1Finish != 0)
          { 
            SS1TimeMilliS = ss1Finish - ss1Start;
            MilliS = SS1TimeMilliS % 1000;
            SS1Time = (SS1TimeMilliS - MilliS) / 1000;
            hours = (SS1Time/60/60);
            mins = (SS1Time-(hours*60*60))/60;
            secs = SS1Time-(hours*60*60)-(mins*60);
          printTime();

    }
  else
    {
      BT.print("DNS/DNF") ;
      SS1TimeMilliS = 0;
    }

BT.print("\t") ;             //tab separator

/*
 * STAGE 2 START TIME   // reads SS2 Start Time from PICC  block 8 and stores in buffer
 */

block = 12;
// Authentication
 
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,block, &key, &(mfrc522.uid));
   
    if (status != MFRC522::STATUS_OK){ 
//       BT.print(F("PCD_Authenticate() failed: "));
//       BT.println(mfrc522.GetStatusCodeName(status));
         BT.println(F("Error-ReScan"));
         BT.println(); 
         return;  }
 
// Read data from the block into buffer
    
    status = mfrc522.MIFARE_Read(block, buffer, &size);
    if (status != MFRC522::STATUS_OK){
         BT.println(F("Error-ReScan"));
         BT.println();  
    return;   }

// Convert buffer type byte to unsigned long int and store

  buffer2epoch();
  ss2Start = temp10;
  
/*
 * STAGE 2 FINISH TIME
 */
 
  block = 13;

// Authentication
 
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,block, &key, &(mfrc522.uid));
  
    if (status != MFRC522::STATUS_OK){ 
//       BT.print(F("PCD_Authenticate() failed: "));
//       BT.println(mfrc522.GetStatusCodeName(status));
         BT.println(F("Error-ReScan"));
         BT.println(); 
        return;  }

// Read data from the block into buffer
    
    status = mfrc522.MIFARE_Read(block, buffer, &size);
    if (status != MFRC522::STATUS_OK){
         BT.println(F("Error-ReScan"));
         BT.println();  
    return;   }

// Convert buffer type byte to unsigned long int and store

  buffer2epoch();
      
  ss2Finish = temp10;

  if (ss2Start > ss2Finish && ss2Finish !=0)                                        // if ss2Start bigger than ss2Finish and SS2Finish is not equal to zero, then write 'error'
    {
      BT.print("ERROR") ;                                         // write ERROR to BT
      SS2TimeMilliS = 0;                                                  // set stage time to zero for total count
    }
  else  
  if (ss2Start != 0 && ss2Finish != 0)
          { 
            SS2TimeMilliS = ss2Finish - ss2Start;
            MilliS = SS2TimeMilliS % 1000;
            SS2Time = (SS2TimeMilliS - MilliS) / 1000;
            hours = (SS2Time/60/60);
            mins = (SS2Time-(hours*60*60))/60;
            secs = SS2Time-(hours*60*60)-(mins*60);
      printTime();

    }
  else 
    {
      BT.print("DNS/DNF") ;
      SS2TimeMilliS = 0;
    }

  BT.print("\t") ;             //tab separator

/*
 * STAGE 3 START TIME   // reads SS3 Start Time from PICC  block 8 and stores in buffer
 */
 
  block = 16;                                                                                    // Stage 3 Start time

// Authentication
 
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,block, &key, &(mfrc522.uid));
   
    if (status != MFRC522::STATUS_OK){ 
//       BT.print(F("PCD_Authenticate() failed: "));
//       BT.println(mfrc522.GetStatusCodeName(status));
         BT.println(F("Error-ReScan"));
         BT.println(); 
        return;  }
 
// Read data from the block into buffer
    
    status = mfrc522.MIFARE_Read(block, buffer, &size);
    if (status != MFRC522::STATUS_OK){
         BT.println(F("Error-ReScan"));
         BT.println();  
    return;   }

// Convert buffer type byte to unsigned long int and store

  buffer2epoch();
  ss3Start = temp10;
  
/*
 * STAGE 3 FINISH TIME
 */
 
  block = 17;                                                                                    // Stage 3 Finish time

// Authentication
 
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,block, &key, &(mfrc522.uid));
  
    if (status != MFRC522::STATUS_OK){ 
//       BT.print(F("PCD_Authenticate() failed: "));
//       BT.println(mfrc522.GetStatusCodeName(status));
         BT.println(F("Error-ReScan"));
         BT.println(); 
        return;  }
 
// Read data from the block into buffer
    
    status = mfrc522.MIFARE_Read(block, buffer, &size);
    if (status != MFRC522::STATUS_OK){
         BT.println(F("Error-ReScan"));
         BT.println();  
    return;   }

// Convert buffer type byte to unsigned long int and store

  buffer2epoch();
      
  ss3Finish = temp10;

  if (ss3Start > ss3Finish && ss3Finish !=0)                                        // if ss3Start bigger than ss3Finish and SS3Finish is not equal to zero, then write 'error'
    {
      BT.print("ERROR") ;                                         // write ERROR to BT
      SS3TimeMilliS = 0;                                                  // set stage time to zero for total count
    }
  else  
  
  if (ss3Start != 0 && ss3Finish != 0)
          { 
            SS3TimeMilliS = ss3Finish - ss3Start;
            MilliS = SS3TimeMilliS % 1000;
            SS3Time = (SS3TimeMilliS - MilliS) / 1000;
            hours = (SS3Time/60/60);
            mins = (SS3Time-(hours*60*60))/60;
            secs = SS3Time-(hours*60*60)-(mins*60);
      printTime();
    }
  else 
    {
      BT.print("DNS/DNF") ;
      SS3TimeMilliS = 0;
    }

  BT.print("\t") ;             //tab separator

/*
 * STAGE 4 START TIME   // reads SS4 Start Time from PICC  block 8 and stores in buffer
 */
 
  block = 20;

// Authentication
 
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,block, &key, &(mfrc522.uid));
  
    if (status != MFRC522::STATUS_OK){ 
//       BT.print(F("PCD_Authenticate() failed: "));
//       BT.println(mfrc522.GetStatusCodeName(status));
         BT.println(F("Error-ReScan"));
         BT.println(); 
        return;  }
 
// Read data from the block into buffer
    
    status = mfrc522.MIFARE_Read(block, buffer, &size);
    if (status != MFRC522::STATUS_OK){
         BT.println(F("Error-ReScan"));
         BT.println();  
    return;   }

// Convert buffer type byte to unsigned long int and store

  buffer2epoch();
  ss4Start = temp10;
  
/*
 * STAGE 4 FINISH TIME
 */
 
  block = 21;                                                                                    // Stage 4 Finish time

/// Authentication
 
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,block, &key, &(mfrc522.uid));
   
    if (status != MFRC522::STATUS_OK){ 
//       BT.print(F("PCD_Authenticate() failed: "));
//       BT.println(mfrc522.GetStatusCodeName(status));
         BT.println(F("Error-ReScan"));
         BT.println(); 
        return;  }
 
// Read data from the block into buffer
    
    status = mfrc522.MIFARE_Read(block, buffer, &size);
    if (status != MFRC522::STATUS_OK){
         BT.println(F("Error-ReScan"));
         BT.println();  
    return;   }
// Convert buffer type byte to unsigned long int and store

  buffer2epoch();
      
  ss4Finish = temp10;

    if (ss4Start > ss4Finish && ss4Finish !=0)                                        // if ss4Start bigger than ss4Finish and SS4Finish is not equal to zero, then write 'error'
    {
      BT.print("ERROR") ;                                         // write ERROR to BT
      SS4TimeMilliS = 0;                                                  // set stage time to zero for total count
    }
  else  
  if (ss4Start != 0 && ss4Finish != 0)
          { 
            SS4TimeMilliS = ss4Finish - ss4Start;
            MilliS = SS4TimeMilliS % 1000;
            SS4Time = (SS4TimeMilliS - MilliS) / 1000;
            hours = (SS4Time/60/60);
            mins = (SS4Time-(hours*60*60))/60;
            secs = SS4Time-(hours*60*60)-(mins*60);
      printTime();
    }
  else 
    {
      BT.print("DNS/DNF") ;
      SS4TimeMilliS = 0;
    }

  
  BT.print("\t") ;             //tab separator

/*
 * STAGE 5 START TIME   // reads SS5 Start Time from PICC  block 8 and stores in buffer
 */
 
  block = 24;                                                                                    // Stage 5 Start time

// Authentication
 
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,block, &key, &(mfrc522.uid));
  
    if (status != MFRC522::STATUS_OK){ 
//       BT.print(F("PCD_Authenticate() failed: "));
//       BT.println(mfrc522.GetStatusCodeName(status));
         BT.println(F("Error-ReScan"));
         BT.println(); 
        return;  }
 
// Read data from the block into buffer
    
    status = mfrc522.MIFARE_Read(block, buffer, &size);
    if (status != MFRC522::STATUS_OK){
         BT.println(F("Error-ReScan"));
         BT.println();  
    return;   }

// Convert buffer type byte to unsigned long int and store

  buffer2epoch();
  ss5Start = temp10;
  
/*
 * STAGE 5 FINISH TIME
 */
 
  block = 25;

// Authentication
 
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,block, &key, &(mfrc522.uid));
   
    if (status != MFRC522::STATUS_OK){ 
//       BT.print(F("PCD_Authenticate() failed: "));
//       BT.println(mfrc522.GetStatusCodeName(status));
         BT.println(F("Error-ReScan"));
         BT.println(); 
        return;  }
 
// Read data from the block into buffer
    
    status = mfrc522.MIFARE_Read(block, buffer, &size);
    if (status != MFRC522::STATUS_OK){
         BT.println(F("Error-ReScan"));
         BT.println();  
    return;   }

// Convert buffer type byte to unsigned long int and store

  buffer2epoch();
      
  ss5Finish = temp10;

    if (ss5Start > ss5Finish && ss5Finish !=0)                                        // if ss5Start bigger than ss5Finish and SS5Finish is not equal to zero, then write 'error'
    {
      BT.print("ERROR") ;                                         // write ERROR to BT
      SS5TimeMilliS = 0;                                                  // set stage time to zero for total count
    }
  else  
  if (ss5Start != 0 && ss5Finish != 0)
    {   
            SS5TimeMilliS = ss5Finish - ss5Start;
            MilliS = SS5TimeMilliS % 1000;
            SS5Time = (SS5TimeMilliS - MilliS) / 1000;
            hours = (SS5Time/60/60);
            mins = (SS5Time-(hours*60*60))/60;
            secs = SS5Time-(hours*60*60)-(mins*60);
        printTime();

    
    }
  else
    {
      BT.print("DNS/DNF") ;
      SS5TimeMilliS = 0;
    }

  BT.print("\t") ;             //tab separator

/*
 * STAGE 6 START TIME   // reads SS6 Start Time from PICC  block 8 and stores in buffer
 */
 
  block = 28;

// Authentication
 
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,block, &key, &(mfrc522.uid));
  
    if (status != MFRC522::STATUS_OK){ 
//       BT.print(F("PCD_Authenticate() failed: "));
//       BT.println(mfrc522.GetStatusCodeName(status));
         BT.println(F("Error-ReScan"));
         BT.println(); 
        return;  }
 
// Read data from the block into buffer
    
    status = mfrc522.MIFARE_Read(block, buffer, &size);
    if (status != MFRC522::STATUS_OK){
         BT.println(F("Error-ReScan"));
         BT.println();  
    return;   }
// Convert buffer type byte to unsigned long int and store

  buffer2epoch();
  ss6Start = temp10;
  
/*
 * STAGE 6 FINISH TIME
 */
 
  block = 29;

// Authentication
 
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,block, &key, &(mfrc522.uid));
 
    if (status != MFRC522::STATUS_OK){ 
//       BT.print(F("PCD_Authenticate() failed: "));
//       BT.println(mfrc522.GetStatusCodeName(status));
         BT.println(F("Error-ReScan"));
         BT.println(); 
         return;  }
 
// Read data from the block into buffer
    
    status = mfrc522.MIFARE_Read(block, buffer, &size);
    if (status != MFRC522::STATUS_OK){
         BT.println(F("Error-ReScan"));
         BT.println();  
    return;   }
          
// Convert buffer type byte to unsigned long int and store

  buffer2epoch();
      
  ss6Finish = temp10;
  
  if (ss6Start > ss6Finish && ss6Finish !=0)                                        // if ss6Start bigger than ss6Finish and SS6Finish is not equal to zero, then write 'error'
    {
      BT.print("ERROR") ;                                         // write ERROR to BT
      SS6TimeMilliS = 0;                                                  // set stage time to zero for total count
    }
  else    
  if (ss6Start != 0 && ss6Finish != 0)
    {   
            SS6TimeMilliS = ss6Finish - ss6Start;
            MilliS = SS6TimeMilliS % 1000;
            SS6Time = (SS6TimeMilliS - MilliS) / 1000;
            hours = (SS6Time/60/60);
            mins = (SS6Time-(hours*60*60))/60;
            secs = SS6Time-(hours*60*60)-(mins*60);
      printTime();
    }
  else
    {
      BT.print("DNS/DNF") ;
      SS6TimeMilliS = 0;
    }

  BT.print("\t") ;             //tab separator

  
/*
 * TOTAL TIME
 * Add all stage times and print the total to bt SERIAL
 */
 
totalRaceTime = SS1TimeMilliS + SS2TimeMilliS + SS3TimeMilliS + SS4TimeMilliS + SS5TimeMilliS + SS6TimeMilliS;
            MilliS = totalRaceTime %1000;                                             //storing remainder of totalracetime in milliseconds
            totalRaceTime=(totalRaceTime-MilliS)/1000;                               //converting totalracetime to whole seconds
            hours = (totalRaceTime/(60*60));
            mins = (totalRaceTime-(hours*60*60))/60;
            secs = totalRaceTime-(hours*60*60)-(mins*60);
            

printTime();
BT.print("\t") ;    //tab separator
 
/*
 * NUMBER OF STAGES
 * Count number of succesful stages and add up
 */
 int StageCount = 0;
 
if (SS1TimeMilliS != 0){                                        // if ss1Time is not equal to zero
    StageCount = StageCount + 1;                                                  // set stage time to zero for total count
    }
    else {
        StageCount = StageCount +0;                                                  // set stage time to zero for total count
    }     
if (SS2TimeMilliS != 0){                                        // if ss1Time is not equal to zero
    StageCount = StageCount + 1;                                                  // set stage time to zero for total count
    }
    else {
        StageCount = StageCount +0;                                                  // set stage time to zero for total count
    }
if (SS3TimeMilliS != 0){                                        // if ss1Time is not equal to zero
    StageCount = StageCount + 1;                                                  // set stage time to zero for total count
    }
    else {
        StageCount = StageCount +0;                                                  // set stage time to zero for total count
    }
if (SS4TimeMilliS != 0){                                        // if ss1Time is not equal to zero
    StageCount = StageCount + 1;                                                  // set stage time to zero for total count
    }
    else {
        StageCount = StageCount +0;                                                  // set stage time to zero for total count
    }
if (SS5TimeMilliS != 0){                                        // if ss1Time is not equal to zero
    StageCount = StageCount + 1;                                                  // set stage time to zero for total count
    }
    else {
        StageCount = StageCount +0;                                                  // set stage time to zero for total count
    }
if (SS6TimeMilliS != 0){                                        // if ss1Time is not equal to zero
    StageCount = StageCount + 1;                                                  // set stage time to zero for total count
    }
    else {
        StageCount = StageCount +0;                                                  // set stage time to zero for total count
    }
    
BT.print(StageCount);
  BT.print("\t") ;             //tab separator
  BT.println();
  beepsLights();
    tagCount ++;  
}

/*
 * Inserts leading zero if required and prints HH:MM:SS.000 format
 */
void printTime() {
  
//String timestamp;
if (hours < 10) {BT.print("0");}       
BT.print(hours);BT.print(F(":"));
if (mins < 10) {BT.print("0");}
BT.print(mins);BT.print(F(":"));
if (secs < 10) {BT.print("0");}
BT.print(secs);BT.print(F("."));
if (MilliS < 10) {
  BT.print("00");
  BT.print(MilliS);
}    
     
else if (MilliS > 9 && MilliS < 100) {
  BT.print("0");
  BT.print(MilliS);
}
else BT.print(MilliS);   

}
  
/* 
 *  Converts type byte to unsigned long int.
 *  Reads each digit from the buffer to the temp placeholders
 */
void buffer2epoch() {
//  byte size = sizeof(buffer);       // byte size = number of digits in the buffer  
      temp0 = buffer[0]-'0';          //writes 1st digit of epoch time from buffer to temp0
      temp1 = buffer[1]-'0';          //writes 2nd digit of epoch time from buffer to temp1
      temp2 = buffer[2]-'0';          //writes 3rd digit of epoch time from buffer to temp2
      temp3 = buffer[3]-'0';          //writes 4th digit of epoch time from buffer to temp3
      temp4 = buffer[4]-'0';          //writes 5th digit of epoch time from buffer to temp4
      temp5 = buffer[5]-'0';          //writes 6th digit of epoch time from buffer to temp5
      temp6 = buffer[6]-'0';          //writes 7th digit of epoch time from buffer to temp6
      temp7 = buffer[7]-'0';          //writes 8th digit of epoch time from buffer to temp7
      temp8 = buffer[8]-'0';          //writes 9th digit of epoch time from buffer to temp8
      temp9 = buffer[9]-'0';          //writes 10th digit of epoch time from buffer to temp9
// use this for times with 7 digits. ie times written to watches between 00h00 and 02h42 include only hours,minutes, seconds and millis (initAfricaTimer()function in Timing POD code baseTimeS definition)
//        temp10 = ((temp0*1000000) + (temp1*100000) + (temp2*10000) + (temp3*1000) + (temp4*100) + (temp5*10) + temp6);
// use this for times with 8 digits. ie times written to watches  include only hours,minutes, seconds and millis (initAfricaTimer()function in Timing POD code baseTimeS definition)
         temp10 = ((temp0*10000000) + (temp1*1000000) + (temp2*100000) + (temp3*10000) + (temp4*1000) + (temp5*100) + (temp6*10) + temp7);
// use this for times with 9 digits. ie times written to watches  include day,hours,minutes, seconds and millis (initAfricaTimer()function in Timing POD code baseTimeS definition)
//        temp10 = ((temp0*100000000) + (temp1*10000000) + (temp2*1000000) + (temp3*100000) + (temp4*10000) + (temp5*1000) + (temp6*100) + (temp7*10) + temp8);}
}
void writeRiderData()
{

  byte buffer[16] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  byte block;
  MFRC522::StatusCode status;
  byte len;

  BT.setTimeout(20000) ;     // 20 second time limit for input from serial
//Need to add in else statement so it loops around when that settimeout time is up

  BT.println(F("*** CARD PRESENTED TO READER ***"));    //shows in serial that it is ready to read
  BT.println(F("Type First Name, ending with #"));
  len = BT.readBytesUntil('#', (char *) buffer, 16) ; // read first name from serial
//for (byte i = len; i < 16; i++) buffer[i] = ' ';        // pad with spaces

///////////////////////////////////
// Ask personal data: First Name
///////////////////////////////////   

  block = 1;
//  Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    BT.println(F("PCD_Authenticate() failed: "));
    return;
  }

  // Write block
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    BT.println(F("MIFARE_Write() failed: "));
//    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
//    BT.println(F("MIFARE_Write(status) success: "));
    BT.println(F("Name written succesfully"));
 
//////////////////////////////////
// Ask personal data: Watch Number
//////////////////////////////////

 for (int i = 0; i < 16; i++) buffer[i] = 0;     // clears the buffer

  BT.println(F("Type Watch Number, ending with #"));
  len = BT.readBytesUntil('#', (char *) buffer, 16) ;          // read race number from serial into buffer

  block = 4;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    BT.println(F("PCD_Authenticate() failed: "));
    return;
  }

  // Write block
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    BT.println(F("MIFARE_Write() failed: "));
    return;
  }
  else {
//BT.println(F("MIFARE_Write(status) success: "));
  BT.println(F("Watch Number written succesfully "));
  }
  
BT.println(F("\n*** WRITE COMPLETE ***"));
BT.println(F("\n*** PRESENT NEXT CARD ***"));
BT.println();
beepsLights();
    tagCount ++;
}


unsigned long flashCount = 0;
String getBatteryVoltage() {
    int sensorValue = analogRead(V_MEAS);          // read the input on analog pin A2 for battery voltage reading:
    //Serial.println(String(sensorValue));
    float voltage   = (float)sensorValue / 155;      // Convert the analog reading to a voltage adjusted to Pieters multimeter
    String batMsg   = "Good";
    if (voltage > BATT_GOOD)  {                   // If Voltage higher than 3.8 display "Good" on LCD
        batMsg    = "Good";
        flashLed  = LOW;
        battGood = 1;
    } else if ( voltage > BATT_LOW ) {            // If Voltage lower than 3.8 display "Turn off POD" on LCD and flash lights. POD will work until 3.5V but will stop working after that.
        batMsg = "OK";
        battGood = 1;
        if ( flashCount < sqCounter ){
            flashCount = sqCounter + BATT_OK_FLASH;            // set toggle time to 5s
            if ( flashLed == LOW ) {                 
              flashLed = HIGH;                        // toggle status led previouse state 
            } else {
               flashLed = LOW;
            }
        }
    } else {
        battGood = 0;
        if ( flashCount < sqCounter ){
            flashCount = sqCounter + BATT_LOW_FLASH;            // set toggle time to 1s
            if ( flashLed == LOW ) {                 
              flashLed = HIGH;                        // toggle status led previouse state 
            } else {
              flashLed = LOW;
            }
        }
        batMsg = "PowerOff";             
    }
    char volt[5];
    String messageBattery = dtostrf(voltage,1,2,volt);       // conver float to string
    //Serial.println(messageBattery);
    messageBattery        = messageBattery + "V " + batMsg;  // append status 
    digitalWrite(STATUS_LED, flashLed); 
    return messageBattery;
}

// Displays main Screen Time & Pod station ID
void updateLcdScreen()    
{
    myGLCD.invert (false); //turn oFF inverted screen    myGLCD.setFont(SmallFont);
    
    // 1st line of display
    myGLCD.invertText (true); //turn on inverted text
    myGLCD.print((thisPod.progname),CENTER,0);         // Displays POD ID 
    myGLCD.invertText (false); //turn off inverted text

    //2nd line  Display

    myGLCD.print("Hold Watch",CENTER,10);          //Instruction

    // 3rd Line
    myGLCD.print("and don't move",CENTER,20);          //Instruction

    // 4th Line
    int temp    = RTC.temperature();
    int mC      = ((temp*100) % 400 ) /100;
    String msg  = String(temp/4);
    msg         = msg + "." + mC + "C";
    myGLCD.print(msg,LEFT,30);     
    myGLCD.print("Tags: ",RIGHT,30);    
    myGLCD.print( String(tagCount),RIGHT,30);

    // 5th Line
    String messageBattery = getBatteryVoltage();  
    myGLCD.print(messageBattery,LEFT,40);

    myGLCD.update();
    myGLCD.clrScr();     
            
}

void beepsLights()                                            // Beeps and leds for succesfull scan
{
  int melody[] = {NOTE_A7, NOTE_C7 };        // notes in the melody:
  int noteDurations[] = {8, 8};                        // note durations 4 = quarter note, 8 = eighth note, etc.::
    
    for (int thisNote = 0; thisNote < 2; thisNote++) {
        int noteDuration = 1000 / noteDurations[thisNote];      //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
        tone(BUZZER, melody[thisNote], noteDuration);
        digitalWrite(LED_TOP, LOW);                             // turn  LED on
        digitalWrite(LED_BOT, LOW);                             // turn  LED on

        int pauseBetweenNotes = noteDuration * 1.30;            // the note's duration + 30% seems to work well:
        delay(pauseBetweenNotes);
    
        noTone(BUZZER);                                         // stop the tone playing:
        digitalWrite(LED_TOP, HIGH);
        digitalWrite(LED_BOT, HIGH);
    }
}

// These functions are to define the POD role and sync ms with seconds

void configrePod() {
    //turn on all LEDS for config mode warning
    digitalWrite(STATUS_LED, LOW);    
    digitalWrite(LED_TOP, LOW);                       
    digitalWrite(LED_BOT, LOW);
    // read last pod type index from eeprom and select that identiry as default
    typeIndex = EEPROM.read(ADDRESS_BTPROG);
    unsigned long ConfigTimeoutMs = millis() + CONFIG_TYME_MS;
    unsigned long msLeft = CONFIG_TYME_MS;
    while ( millis() < ConfigTimeoutMs ) {
        // check for card read and incremetn typeIndex
        if ( mfrc522.PICC_IsNewCardPresent()) {    // Look for new cards
            typeIndex++;
        }
        if (typeIndex >= LIST_SIZE1) {
            typeIndex = 0;                          // if uninitilized set to 0
        }
        thisPod = BTProgList[typeIndex];
        // display curretn pod id and start countdown to applying config
        //Serial.println("array size = " + sizeof(thisPod));
        msLeft = ConfigTimeoutMs - millis();
        configDisplay (msLeft );
        delay (100);
//        Serial.println(String(typeIndex) + "   "+ String(msLeft));
//        Serial.println(BTProgList[typeIndex].progid);
//        Serial.println(BTProgList[typeIndex].progname);

    }
    // save pod type in EEPROM for next boot
    EEPROM.write(ADDRESS_BTPROG, typeIndex);
    //turn off all LEDS for config mode warning
    digitalWrite(STATUS_LED, HIGH);
    digitalWrite(LED_TOP, HIGH);                       
    digitalWrite(LED_BOT, HIGH);  

  if(BTProgList[typeIndex].progid == 2){         // Runs BT program 2

  // Display Column headings (only if bluetooth connected before configure() completes)
  BT.print("FIRST NAME "); BT.print("\t");
  BT.print("SURNAME "); BT.print("\t");
  BT.print("WATCH NUMBER "); BT.print("\t");
  BT.print("SS 1 "); BT.print("\t");
  BT.print("SS 2 "); BT.print("\t");
  BT.print("SS 3 "); BT.print("\t");
  BT.print("SS 4 "); BT.print("\t");
  BT.print("SS 5 "); BT.print("\t");
  BT.print("SS 6 "); BT.print("\t");
  BT.print("TOTAL"); BT.print("\t");
  BT.print("Stages Done"); BT.print("\t");
  BT.println();
    }
}


// Display fucntion during config loop
void configDisplay( unsigned long timeLeftMs)   {
//    myGLCD.invert (true);
     // 1st line of display
    myGLCD.setFont(SmallFont);
    String msg = VERSION;
    msg = msg + " Configure ";
    myGLCD.print(msg,LEFT,0);       

   //2nd line  Display
    myGLCD.setFont(SmallFont);          
    myGLCD.print("DON'T TAG!",CENTER,10);        

    //3rd line  Display
    myGLCD.setFont(SmallFont);          
    myGLCD.print((thisPod.progname),CENTER,20);        
 
    // 4th Line
    myGLCD.setFont(SmallFont);
    myGLCD.print("Swipe2Change",CENTER,30);

    // 5th Line
    String timeMsg = "Done in ";
    unsigned long ms = (timeLeftMs % 1000)/100;
    timeMsg = timeMsg + (timeLeftMs/1000) + "." + ms + "s";
    myGLCD.setFont(SmallFont);
    myGLCD.print(timeMsg,LEFT,40);

    myGLCD.update();
    myGLCD.clrScr();
}

// All Genral Purpouse IO pin Setup stuffs 
void initIoPins() {
    // Input Pins
    pinMode(SQ_WAVE, INPUT);

    // Output Pins
    pinMode(LED_TOP, OUTPUT);
    pinMode(LED_BOT, OUTPUT);
    pinMode(STATUS_LED, OUTPUT);
    // set pins start condition: LOW = LED on, HIGH = LED off
    digitalWrite(LED_TOP, HIGH);                       
    digitalWrite(LED_BOT, HIGH);                       
    digitalWrite(STATUS_LED, LOW);                     
}

// All RFID init Stuffs
void initMfrc522() {
    SPI.begin();                                          // Init SPI bus
    mfrc522.PCD_Init();                                   // Init MFRC522 card
    mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);       // Enhance the MFRC522 Receiver Gain to maximum value of some 48 dB (default is 33dB)

    for (byte i = 0; i < 6; i++) {                // Prepare the key (used both as key A and as key B) using FFFFFFFFFFFFh which is the default at chip delivery from the factory
        key.keyByte[i] = 0xFF;
    }
}
