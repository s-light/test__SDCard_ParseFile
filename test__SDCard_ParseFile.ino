/**************************************************************************************************/
/** libraries                                                                                    **/
/**************************************************************************************************/
#include <SD.h> // SD-Card handling


/**************************************************************************************************/
/** sketch info                                                                                  **/
/**************************************************************************************************/
void print_info(Print &pOut) {
	pOut.println();
	//             "|~~~~~~~~~|~~~~~~~~~|~~~..~~~|~~~~~~~~~|~~~~~~~~~|"
	pOut.println(F("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
	pOut.println(F("|                       ^ ^                      |"));
	pOut.println(F("|                      (0,0)                     |"));
	pOut.println(F("|                      ( _ )                     |"));
	pOut.println(F("|                       \" \"                      |"));
	pOut.println(F("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
	pOut.println(F("| test__SDCard_ParseFile.ino"));
	pOut.println(F("|   test / example sketch to read lines of a file on an SDCard"));
	pOut.println(F("|   and parse it accordingly to first char of each line"));
	pOut.println(F("|"));
	pOut.println(F("| This Sketch has a debug-menu:"));
	pOut.println(F("| send '?'+Return for help"));
	pOut.println(F("|"));
	pOut.println(F("| dream on & have fun :-)"));
	pOut.println(F("|"));
	pOut.println(F("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
	pOut.println(F("|"));
	//pOut.println(F("| Version: Nov 11 2013  20:35:04"));
	pOut.print(F("| version: "));
	pOut.print(F(__DATE__));
	pOut.print(F("  "));
	pOut.print(F(__TIME__));
	pOut.println();
	pOut.println(F("|"));
	pOut.println(F("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
	pOut.println();
	
	//pOut.println(__DATE__); Nov 11 2013
	//pOut.println(__TIME__); 20:35:04
}
/**************************************************************************************************
		
	test__SDCard_ParseFile.ino
		test / example sketch to read lines of a file on an SDCard
		and parse it accordingly to first char of each line
		debugout on usbserial interface: 115200baud
	
	hardware:
		Board:
		Arduino compatible with Ethernet-Shield
	
	libraries used:
		~ SD.h
	
	
	written by stefan krueger (s-light), mail@s-light.eu, http://s-light.eu, https://github.com/s-light/
	
	SD handling based on library reference & examples:
		http://arduino.cc/en/Reference/SD
		http://arduino.cc/en/Tutorial/DumpFile
	
	changelog / history
		18.02.2014 07:36 created
		18.02.2014 08:50 first draft of implementation finished.
		18.02.2014 23:18 reading and parsing works. some small bugs
		19.02.2014 09:06 all things tested successfully.
	
	TO DO:
		~ add SD-Card get Informations (http://arduino.cc/en/Tutorial/CardInfo)
		~ add 'Read File List'
		
**************************************************************************************************/
/**************************************************************************************************
	license
	
	CC BY SA
		This work is licensed under the
		Creative Commons Attribution-ShareAlike 3.0 Unported License.
		To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/.
		
	Apache License Version 2.0
		Copyright 2014 stefan krueger
		
		Licensed under the Apache License, Version 2.0 (the "License");
		you may not use this file except in compliance with the License.
		You may obtain a copy of the License at
		
		http://www.apache.org/licenses/LICENSE-2.0
		
		Unless required by applicable law or agreed to in writing, software
		distributed under the License is distributed on an "AS IS" BASIS,
		WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
		See the License for the specific language governing permissions and
		limitations under the License.
		
	The MIT License (MIT)
		Copyright (c) 2014 stefan krueger
		Permission is hereby granted, free of charge, to any person obtaining a copy
		of this software and associated documentation files (the "Software"),
		to deal in the Software without restriction, including without limitation
		the rights to use, copy, modify, merge, publish, distribute, sublicense,
		and/or sell copies of the Software, and to permit persons to whom the Software
		is furnished to do so, subject to the following conditions:
		The above copyright notice and this permission notice shall be included in all
		copies or substantial portions of the Software.
		THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
		INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
		PARTICULAR PURPOSE AND NONINFRINGEMENT.
		IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
		OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
		OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
		http://opensource.org/licenses/mit-license.php
	
**************************************************************************************************/


/** Serial.print to Flash:
	for information about F():  http://arduino.cc/en/Serial/Print
	Notepad++ Replace RegEx:
		Find what:		Serial.print(.*)\("(.*)"\);
		Replace with:	Serial.print\1(F("\2"));
**/
 

 
 
 
/**************************************************************************************************/
/** Include yourselves header file  (used to define structs and co)                               **/
/**************************************************************************************************/
// use "" for files in same directory as .ino
//#include "file.h"


 



/**************************************************************************************************/
/** definitions (gloabl)                                                                         **/
/**************************************************************************************************/


/**************************************************/
/**  DebugOut                                    **/
/**************************************************/

boolean bLEDState = 0;
const byte cbID_LED_Info = 9; //D9

unsigned long ulDebugOut_LiveSign_TimeStamp_LastAction	= 0;
const word cwDebugOut_LiveSign_UpdateInterval			= 1000; //ms

boolean bDebugOut_LiveSign_Serial_Enabled	= 0;
boolean bDebugOut_LiveSign_LED_Enabled		= 1;


/**************************************************/
/** Menu Input                                   **/
/**************************************************/

// a string to hold new data
char  sMenu_Input_New[]				= "testcommand";
// flag if string is complete
boolean bMenu_Input_New_FlagComplete	= false;

// string for Currently to process Command
char  sMenu_Command_Current[]		= "testcommand";



/**************************************************/
/** SD-Card                                      **/
/**************************************************/


const byte bPIN_CS_SDCard = 4;


/**************************************************/
/** other things...                              **/
/**************************************************/


/**************************************************************************************************/
/** functions                                                                                    **/
/**************************************************************************************************/

/************************************************/
/**  Debug things                              **/
/************************************************/

// http://forum.arduino.cc/index.php?topic=183790.msg1362282#msg1362282
int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


void printBinary8(byte bIn)  {
	
	for (unsigned int mask = 0b10000000; mask; mask >>= 1) {
		if (mask & bIn) {
			Serial.print('1');
		}
		else {
			Serial.print('0');
		}
	}
}

void printBinary12(word bIn)  {
	//                       B12345678   B12345678
	//for (unsigned int mask = 0x8000; mask; mask >>= 1) {
	for (unsigned int mask = 0b100000000000; mask; mask >>= 1) {
		if (mask & bIn) {
			Serial.print('1');
		}
		else {
			Serial.print('0');
		}
	}
}

void printBinary16(word wIn)  {
	
	for (unsigned int mask = 0b1000000000000000; mask; mask >>= 1) {
		if (mask & wIn) {
			Serial.print('1');
		}
		else {
			Serial.print('0');
		}
	}
}



/************************************************/
/**  Menu System                               **/
/************************************************/

// Modes for Menu Switcher
const uint8_t cbMenuMode_MainMenu	= 1;
const uint8_t cbMenuMode_SubMenu1	= 2;
uint8_t bMenuMode = cbMenuMode_MainMenu;


// SubMenu SetValues
void handle_SubMenu1(Print &pOut, char *caCommand) {
	pOut.println(F("SubMenu1:"));
	pOut.println(F("\t nothing here."));
	pOut.println(F("\t finished."));
	// exit submenu
	// reset state manschine of submenu
	// jump to main
	bMenuMode = cbMenuMode_MainMenu;
	bMenu_Input_New_FlagComplete = true;
}


// Main Menu
void handle_MainMenu(Print &pOut, char *caCommand) {
	/* pOut.print("sCommand: '");
	pOut.print(sCommand);
	pOut.println("'"); */
	switch (caCommand[0]) {
		case 'h':
		case 'H':
		case '?': {
			// help
			pOut.println(F("____________________________________________________________"));
			pOut.println();
			pOut.println(F("Help for Commands:"));
			pOut.println();
			pOut.println(F("\t '?': this help"));
			pOut.println(F("\t 'i': sketch info"));
			pOut.println(F("\t 'y': toggle DebugOut livesign print"));
			pOut.println(F("\t 'Y': toggle DebugOut livesign LED"));
			pOut.println(F("\t 'x': tests"));
			pOut.println();
			pOut.println(F("\t 'c': Show Card Info "));
			pOut.println(F("\t 'r': read file 'test.txt' content and parse lines"));
			pOut.println();
			pOut.println(F("\t 'set:' enter SubMenu1"));
			pOut.println();
			pOut.println(F("____________________________________________________________"));
		} break;
		case 'i': {
			print_info(pOut);
		} break;
		case 'y': {
			pOut.println(F("\t toggle DebugOut livesign Serial:"));
			bDebugOut_LiveSign_Serial_Enabled = !bDebugOut_LiveSign_Serial_Enabled;
			pOut.print(F("\t bDebugOut_LiveSign_Serial_Enabled:"));
			pOut.println(bDebugOut_LiveSign_Serial_Enabled);
		} break;
		case 'Y': {
			pOut.println(F("\t toggle DebugOut livesign LED:"));
			bDebugOut_LiveSign_LED_Enabled = !bDebugOut_LiveSign_LED_Enabled;
			pOut.print(F("\t bDebugOut_LiveSign_LED_Enabled:"));
			pOut.println(bDebugOut_LiveSign_LED_Enabled);
		} break;
		case 'x': {
			// get state
			pOut.println(F("__________"));
			pOut.println(F("Tests:"));
			
			word wTest = atoi("65535");
			pOut.print(F("wTest: "));
			pOut.print(wTest);
			pOut.println();
			
			int iTest = atoi("65535");
			pOut.print(F("iTest: "));
			pOut.print(iTest);
			pOut.println();
			
			pOut.println(F("__________"));
		} break;
		//--------------------------------------------------------------------------------
		case 'c': {
			pOut.println(F("\t Card Info:"));
			pOut.println(F("\t   TODO: show card info here"));
		} break;
		case 'r': {
			pOut.print(F("\t read file:"));
			readSDCard();
		} break;
		//--------------------------------------------------------------------------------
		case 's': {
			// SubMenu1
			if ( (caCommand[1] == 'e') && (caCommand[2] == 't') && (caCommand[3] == ':') ) { 
				//if full command is 'set:' enter submenu
				bMenuMode = cbMenuMode_SubMenu1;
				if(1){	//if ( caCommand[4] != '\0' ) {
					//full length command 
					//handle_SetValues(pOut, &caCommand[4]);
				} else {
					bMenu_Input_New_FlagComplete = true;
				}
			}
		} break;
		//--------------------------------------------------------------------------------
		default: {
			pOut.print(F("command '"));
			pOut.print(caCommand);
			pOut.println(F("' not recognized. try again."));
			sMenu_Input_New[0] = '?';
			bMenu_Input_New_FlagComplete = true;
		}
	} //end switch
	
	//end Command Parser
}


// Menu Switcher
void menuSwitcher(Print &pOut, char *caCommand) {
	switch (bMenuMode) {
			case cbMenuMode_MainMenu: {
				handle_MainMenu(pOut, caCommand);
			} break;
			case cbMenuMode_SubMenu1: {
				handle_SubMenu1(pOut, caCommand);
			} break;
			default: {
				// something went wronge - so reset and show MainMenu
				bMenuMode = cbMenuMode_MainMenu;
			}
		} // end switch bMenuMode
}

// Check for NewLineComplete and enter menuSwitcher
// sets Menu Output channel (pOut)
void check_NewLineComplete() {
	// if SMenuCurrent is a full Line (terminated with \n) than parse things
	if (bMenu_Input_New_FlagComplete) {
		/*
		Serial.print(F("bMenu_Input_New_FlagComplete: sMenu_Input_New: '"));
		Serial.print(sMenu_Input_New);
		Serial.println(F("'"));
		Serial.print(F("   bState_UI: '"));
		Serial.print(bState_UI);
		Serial.println(F("'"));/**/
		
		// coppy to current buffer
		strcpy (sMenu_Command_Current, sMenu_Input_New);
		
		// reset input
		memset(sMenu_Input_New, '\0', sizeof(sMenu_Input_New)-1);
		bMenu_Input_New_FlagComplete = false;
		
		// run command
		menuSwitcher(Serial, sMenu_Command_Current);
		
	} // if bMenu_Input_New_FlagComplete
}

/************************************************/
/**  Serial Receive Handling                   **/
/************************************************/

void handle_SerialReceive() {
	// collect next input text
	while (Serial.available()) {
		// get the new byte:
		char charNew = (char)Serial.read();
		/*Serial.print(F("charNew '"));
		Serial.print(charNew);
		Serial.print(F("' : "));
		Serial.println(charNew, DEC);
		Serial.print(F("'\\n' : "));
		Serial.println('\n', DEC);
		Serial.print(F("'\\r' : '"));
		Serial.println('\r', DEC);*/
		
		/* http://forums.codeguru.com/showthread.php?253826-C-String-What-is-the-difference-between-n-and-r-n
			'\n' == 10 == LineFeed == LF
			'\r' == 13 == Carriag Return == CR
			Windows: '\r\n'
			Linux: '\n'
			Apple: '\r'
		*/
		// if the incoming character is a linefeed '\r' or newline '\n'
		//if ((charNew == '\r') || (charNew == '\n')) {
		if (charNew == '\n') {
			//clean up last received char if it is a '\r'
			if (sMenu_Input_New[strlen(sMenu_Input_New) - 1] == '\r') {
				sMenu_Input_New[strlen(sMenu_Input_New) - 1] = '\0';
			}
			//set complete flag
			bMenu_Input_New_FlagComplete = true;
		} else {
			// add it to the sMenu_Input_New:
			//sMenu_Input_New = sMenu_Input_New + charNew;
			// char array version
			//check for length..
			if (strlen(sMenu_Input_New) < sizeof(sMenu_Input_New) ) {
				sMenu_Input_New[strlen(sMenu_Input_New)] = charNew;
			} else {
				Serial.println(F("inputstring to long!"));
				// reset
				memset(sMenu_Input_New,'\0',sizeof(sMenu_Input_New)-1);
				bMenu_Input_New_FlagComplete = false;
			}
		}
	}
}





/************************************************/
/** SD-Card / File handling                    **/
/************************************************/

void parseLine(Print &pOut, char *caLine) {
	/*pOut.print(F("   caLine: '"));
	pOut.print(caLine);
	pOut.println(F("'"));*/
	/**
	format of file content:
	name:a test file
	age:42
	x:22;65535;42;21,17,1234
	#comment
	
	**/
	switch (caLine[0]) {
		case '#': {
			// do nothing - this line is a comment
			pOut.println(F("# ignore line - comment"));
		} break;
		case 'n': {
			pOut.print(F("name: "));
			char * pCH;
			pCH = strchr(caLine,':');
			if (pCH != NULL) {
				// set pointer to start of value
				pCH = pCH +1;
				// do something with value
				pOut.print(F("'"));
				pOut.print(pCH);
				pOut.print(F("'"));
			} else {
				pOut.print(F(" Line-Format is not valid!"));
			}
			pOut.println();
		} break;
		case 'a': {
			pOut.print(F("age: "));
			char * pCH;
			pCH = strchr(caLine,':');
			if (pCH != NULL) {
				// set pointer to start of value
				pCH = pCH +1;
				// do something with value
				int iAge = 0;
				iAge = atoi(pCH);
				pOut.print(iAge);
			} else {
				pOut.print(F(" Line-Format is not valid!"));
			}
			pOut.println();
		} break;
		case 'x': {
			pOut.println(F("x array: "));
			char * pCH;
			pCH = strchr(caLine,':');
			if (pCH != NULL) {
				// set pointer to start of value
				pCH = pCH +1;
				pOut.print(F("   value: "));
				pOut.println(pCH);
				
				// do something with value
				// parse array
				// word waTest[6] = {0, 0, 0, 0, 0, 0};
				char * pToken;
				pToken = strtok(pCH, "; ");
				//for (byte bIndex = 0; (bIndex < 6) && (pToken != NULL); bIndex++) {
				for (byte bIndex = 0; (pToken != NULL); bIndex++) {
					//pOut.print(F("\t token: "));
					//pOut.println(pToken);
					
					//waTest[bIndex] = atoi(pToken);
					
					word wValue = (word)atoi(pToken);
					
					// print value:
					pOut.print(F("\t  ["));
					pOut.print(bIndex);
					pOut.print(F("] = "));
					pOut.print(wValue);
					pOut.println();
					// get next pointer
					pToken = strtok(NULL, "; ");
				}
			} else {
				pOut.print(F(" Line-Format is not valid!"));
			}
			pOut.println();
		} break;
		//--------------------------------------------------------------------------------
		default: {
			pOut.print(F("parsing error: caLine[0]: '"));
			pOut.print(caLine[0]);
			pOut.println(F("' not recognized."));
		}
	} //end switch
	
	
}//end Line Parser



void readLinesAndParse(Print &pOut, File f) {
	
	
	// a char array (string) to hold new data
	// this array must be so long as the longest line in file.
	char  sSDCard_Line_New[] = "this text is as long as the longest Line";
	// reset memory
	memset(sSDCard_Line_New,'\0',sizeof(sSDCard_Line_New)-1);
	
	// flag for line end detection
	boolean bFlag_EOL	= false;
	boolean bFlag_LongLine = false;
	boolean bFlag_SkipRest = false;
	
	// read file content
	while (f.available()) {
		
		// get the new byte:
		char charNew = (char)f.read();
		
		// collect next full line
		/* http://forums.codeguru.com/showthread.php?253826-C-String-What-is-the-difference-between-n-and-r-n
			'\n' == 10 == LineFeed == LF
			'\r' == 13 == Carriag Return == CR
			Windows: '\r\n'
			Linux: '\n'
			Apple: '\r'
		*/
		// check for line end
		switch (charNew) {
			case '\r':
			case '\n': {
				if (strlen(sSDCard_Line_New) > 0) {
					bFlag_EOL = true;
				}
			} break;
			default: {
				// normal char -
				// add it to the sSDCard_Line_New:
				//check for length
				if (strlen(sSDCard_Line_New) < (sizeof(sSDCard_Line_New)-1) ) {
					sSDCard_Line_New[strlen(sSDCard_Line_New)] = charNew;
				} else {
					// Serial.println(F(" line to long! ignoring rest of line"));
					// set complete flag so line will be parsed
					bFlag_EOL = true;
					// skip rest of line 
					bFlag_LongLine = true;
				}
			}// default
		}// switch charNew
		
		// complete line found:
		if (bFlag_EOL) {
			// pOut.println(F(" bFlag_EOL: "));
			// pOut.print(F("  sSDCard_Line_New: '"));
			// pOut.print(sSDCard_Line_New);
			// pOut.println(F("'"));
			
			if ( !bFlag_SkipRest) {
				// pOut.println(F("   parse Line:"));
				// parse line
				parseLine(pOut, sSDCard_Line_New);
				if(bFlag_LongLine) {
					bFlag_SkipRest = true;
					bFlag_LongLine = false;
				}
			} else {
				// pOut.println(F("   skip rest of Line"));
				bFlag_SkipRest = false;
			}
			
			
			// reset flag
			bFlag_EOL = false;
			
			// reset memory
			memset(sSDCard_Line_New,'\0',sizeof(sSDCard_Line_New)-1);
			
		}// if Flag complete
		
	}
}

void readFileParseLines(Print &pOut, char * cFileName) {
	
	if (SD.exists(cFileName)) {
		// Open File for reading
		File f = SD.open(cFileName, FILE_READ);
		// check if file can be read
		if (f) {
			// read file
			// split at lines..
			// parse line
			readLinesAndParse(pOut, f);
			
			//close access to card
			f.close();
			
		} else {
			pOut.println(F("error at opening file for reading."));
		}
	} else {
		pOut.println(F("File does not exist on card!!"));
	}
	
}



void listFiles(Print &pOut) {
	Serial.println(F("listFiles: TODO"));
	// see http://arduino.cc/en/Tutorial/CardInfo
}

void readSDCard() {
	// check if SD Card is Present:
	if (SD.begin(bPIN_CS_SDCard)) {
		Serial.println(F("Card available"));
		// list all files on card:
		listFiles(Serial);
		
		// longest filename possible:
		char * cFileName = "12345678.txt";
		
		// read file
		Serial.println(F("File with Windows Line Ending (CR LF)"));
		cFileName = "test1.txt";
		readFileParseLines(Serial, cFileName);
		Serial.println(F("File with Linux Line Ending (CR)"));
		cFileName = "test2.txt";
		readFileParseLines(Serial, cFileName);
		Serial.println(F("File with Mac Line Ending (LF)"));
		cFileName = "test3.txt";
		readFileParseLines(Serial, cFileName);
		
		
	} else {
		Serial.println(F("Card not present or defect"));
	}
}


/************************************************/
/** other things                               **/
/************************************************/


/**************************************************/
/**                                              **/
/**************************************************/



/****************************************************************************************************/
/** Setup                                                                                          **/
/****************************************************************************************************/
void setup() {
	
	/************************************************/
	/** Initialise PINs                            **/
	/************************************************/
		
		//LiveSign
		pinMode(cbID_LED_Info, OUTPUT);
		digitalWrite(cbID_LED_Info, HIGH);
		
		// as of arduino 1.0.1 you can use INPUT_PULLUP
		
	/************************************************/
	/** init serial                                **/
	/************************************************/
		
		// for ATmega32U4 devices:
		#if defined (__AVR_ATmega32U4__)
			//wait for arduino IDE to release all serial ports after upload.
			delay(2000);
		#endif
		
		Serial.begin(115200);
		
		// for ATmega32U4 devices:
		#if defined (__AVR_ATmega32U4__)
			// Wait for Serial Connection to be Opend from Host or 6second timeout
			unsigned long ulTimeStamp_Start = millis();
			while( (! Serial) && ( (millis() - ulTimeStamp_Start) < 6000 ) ) {
				1;
			}
		#endif
		
		Serial.println();
		
		Serial.print(F("# Free RAM = "));
		Serial.println(freeRam());
		
	/************************************************/
	/** Welcom                                     **/
	/************************************************/
		
		print_info(Serial);
		
	/************************************************/
	/** setup XXX1                                 **/
	/************************************************/
		
		Serial.print(F("# Free RAM = "));
		Serial.println(freeRam());
		
		Serial.println(F("setup SD Lib:")); {
			
			Serial.println(F("\t sub action"));
		}
		Serial.println(F("\t finished."));
		
	/************************************************/
	/** show Serial Commands                       **/
	/************************************************/
		
		// reset Serial Debug Input
		memset(sMenu_Input_New, '\0', sizeof(sMenu_Input_New)-1);
		//print Serial Options
		sMenu_Input_New[0] = '?';
		bMenu_Input_New_FlagComplete = true;
		
		
	/************************************************/
	/** GO                                         **/
	/************************************************/
		
		Serial.println(F("Loop:"));
		
		
		
} /** setup **/


/****************************************************************************************************/
/** Main Loop                                                                                      **/
/****************************************************************************************************/
void loop() {
	
	/**************************************************/
	/** Menu Input                                   **/
	/**************************************************/
		// Serial
		handle_SerialReceive();
		check_NewLineComplete();
		
	/**************************************************/
	/** Timed things                                 **/
	/**************************************************/
		
		/*
		// every XXXXms
		if ( ( millis() - ulTimeStamp_LastAction ) > cwUpdateInterval) {
			ulTimeStamp_LastAction =  millis();
			//
		}
		*/
		
		
	/**************************************************/
	/** Debug Out                                    **/
	/**************************************************/
		
		if ( (millis() - ulDebugOut_LiveSign_TimeStamp_LastAction) > cwDebugOut_LiveSign_UpdateInterval) {
			ulDebugOut_LiveSign_TimeStamp_LastAction = millis();
			
			if ( bDebugOut_LiveSign_Serial_Enabled ) {
				Serial.print(millis());
				Serial.print(F("ms;"));
				Serial.print(F("  free RAM = "));
				Serial.println(freeRam());
			}
			
			if ( bDebugOut_LiveSign_LED_Enabled ) {
				bLEDState = ! bLEDState;
				if (bLEDState) {
					//set LED to HIGH
					digitalWrite(cbID_LED_Info, HIGH);
				} else {
					//set LED to LOW
					digitalWrite(cbID_LED_Info, LOW);
				}
			}
			
		}
		
	/**************************************************/
	/**                                              **/
	/**************************************************/
		
} /** loop **/


/****************************************************************************************************/
/** THE END                                                                                        **/
/****************************************************************************************************/