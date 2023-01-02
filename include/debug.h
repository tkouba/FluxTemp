//***************************************************************
//   Example of use: 
//   #define DEBUG  //                              <---<<< this line must appear before the include line
//
//   #include <debug.h>
// 
//If you comment the line:    #define DEBUG
//the Macro lines are defined as blank, thus would be ignored by the compiler
//#define DEBUG  // if this line is NOT commented, these macros will be included in the sketch
//examples:
//  This  converts to >>>>----------------------->  This OR a Blank Line.  
// DPRINTLN("Testing123");                          Serial.println("Testing123");  
// DPRINTLN(0xC0FFEEul,DEC);                        Serial.println(0xC0FFEEul,DEC); 
// DPRINTLN(12648430ul,HEX);                        Serial.println(12648430ul,HEX); 
// DPRINTLNF("This message came from your flash");  Serial.println(F("This message came from your flash"));
// DPRINT(myVariable);                              Serial.print(myVariable);
//
// Source: https://forum.arduino.cc/t/how-to-debug-code-in-arduino-ide/209670/20
// Also, this works  #define INFO(...)  { Console->printf("INFO: "); Console->printf(__VA_ARGS__); }   >>>--->   where {} allows multiple lines of code.
// See: http://forum.arduino.cc/index.php?topic=511393.msg3485833#new
#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef DEBUG
#define SERIALBEGIN(...)   Serial.begin(__VA_ARGS__)
#define DPRINT(...)        Serial.print(__VA_ARGS__)
#define DPRINTLN(...)      Serial.println(__VA_ARGS__)
#define DPRINTF(...)       Serial.printf(__VA_ARGS__)
#define DPRINTFLN(...)     Serial.printf(__VA_ARGS__); Serial.println()
#define DPRINT_F(...)      Serial.print(F(__VA_ARGS__))
#define DPRINTLN_F(...)    Serial.println(F(__VA_ARGS__)) //printing text using the F macro

//***************************************************************
#else
#define SERIALBEGIN(...)  
#define DPRINT(...)       
#define DPRINTLN(...)     
#define DPRINTF(...)
#define DPRINTFLN(...)
#define DPRINT_F(...)
#define DPRINTLN_F(...)

#endif
//***************************************************************
#endif