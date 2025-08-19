#include <Keypad.h>


const byte ROWS = 2; // use 4X4 keypad for both instances
const byte COLS = 2;
char keys[ROWS][COLS] = {
  {'1','2'},
  {'3','4'}
};
byte rowPins[ROWS] = {5, 4}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 6}; //connect to the column pinouts of the keypad
Keypad kpd( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


const byte ROWSR = 2;
const byte COLSR = 2;
char keysR[ROWSR][COLSR] = {
  {'a','b'},
  {'c','d'}
};
byte rowPinsR[ROWSR] = {3, 2}; //connect to the row pinouts of the keypad
byte colPinsR[COLSR] = {7, 6}; //connect to the column pinouts of the keypad
Keypad kpdR( makeKeymap(keysR), rowPinsR, colPinsR, ROWSR, COLSR );


const byte ROWSUR = 4;
const byte COLSUR = 1;
char keysUR[ROWSUR][COLSUR] = {
  {'M'},
  {'A'},
  {'R'},
  {'K'}
};
// Digitran keypad, bit numbers of PCF8574 i/o port
byte rowPinsUR[ROWSUR] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
byte colPinsUR[COLSUR] = {8}; //connect to the column pinouts of the keypad

Keypad kpdUR( makeKeymap(keysUR), rowPinsUR, colPinsUR, ROWSUR, COLSUR );


void setup(){
//  Wire.begin( );
  kpdUR.begin( makeKeymap(keysUR) );
  kpdR.begin( makeKeymap(keysR) );
  kpd.begin( makeKeymap(keys) );
  Serial.begin(9600);
  Serial.println( "start" );
}

//byte alternate = false;
char key, keyR, keyUR;
void loop(){

//  alternate = !alternate;
  key = kpd.getKey( );
  keyUR = kpdUR.getKey( );
  keyR = kpdR.getKey( );

  if (key){
    Serial.println(key);
  }
  if( keyR ) {
    Serial.println( keyR );
  }
  if( keyUR ) {
    Serial.println( keyUR );
  }
}
