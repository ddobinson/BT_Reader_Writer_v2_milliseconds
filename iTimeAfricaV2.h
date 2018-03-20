/**
*header file for ITA v2 PCB
*/

#define VERSION "V2.2"
//GLOBAL DEFINITIONS         

#define DS3231_I2C_ADDRESS 	0x68

//PCB pin definition for V32
#define LCD_RST				A1
#define SQ_WAVE				A2
#define CONFIG 				A3
#define SDA					SDA
#define SCL					SCL
#define V_MEAS				A7
#define STATUS_LED	  1 //bluetooth also uses D1 so if BT required, all statusled commands must be taken out
#define BUZZER 				2
#define LED_TOP				3
#define LED_BOT				A0
#define LCD_CLK				4
#define LCD_DIN				5
#define LCD_COM 			6
#define LCD_CE 				7
#define UART_TX				8
#define UART_RX				9
#define RFID_CS				10 
#define RFID_MOSI			MOSI 
#define RFID_MISO			MISO
#define RFID_SCK			SCK 


// EEPROM stuffs
// Arduino have different amounts of EEPROM
// 1024 bytes on the ATmega328P

#define ADDRESS_POD_TYPE 0
#define ADDRESS_BTPROG 0

#define BATT_GOOD       3.8
#define BATT_LOW        3.5
#define BATT_OK_FLASH   2000
#define BATT_LOW_FLASH  100
// BELOW IS STATION/POD IDENTIFICATION!
#define LIST_SIZE 16
#define START_POD  0
#define FINISH_POD 1

// BELOW IS BT program TYPE IDENTIFICATION!
#define LIST_SIZE1 4
struct BTProg {
	int progid;
	char progname[16]; 
} ;


struct BTProg BTProgList [LIST_SIZE1] = {
	{ 1 , "BT WRITE2WATCH"},
	{ 2 , "BT D/L RESULTS"}, 
	{ 3, "BT WIPE ALL" },
	{ 4, "BT WIPE TIMES"},
    };
/*
// BELOW IS STATION/POD IDENTIFICATION!
#define LIST_SIZE2 16
struct podType {
  int block;
  char podID[16]; 
} ;


struct podType podTypeList [LIST_SIZE2] = {
  { 8 , "Stage 1 Start"},
  { 9 , "Stage 1 Finish"}, 
  { 12, "Stage 2 Start" },
  { 13, "Stage 2 Finish"},
  { 16, "Stage 3 Start" },
  { 17, "Stage 3 Finish"},
  { 20, "Stage 4 Start" },
  { 21, "Stage 4 Finish"},
  { 24, "Stage 5 Start" },
  { 25, "Stage 5 Finish"},
  { 28, "Stage 6 Start" },
  { 29, "Stage 6 Finish"},
  { 32, "Stage 7 Start" },
  { 33, "Stage 7 Finish"},
  { 36, "Stage 8 Start" },
  { 37, "Stage 8 Finish"},
    };
*/        
// variouse values
#define CONFIG_TYME_MS 10000  
// int main()
// {
//     printf("Hello World\n");
//     printf( "%i\n",STRUCTSIZE);
//     for ( int i = 0 ; i < STRUCTSIZE ; i++ ) {
//          printf("%s\n", podTypeList[i].podID);
//     }

//     return 0;
// }
