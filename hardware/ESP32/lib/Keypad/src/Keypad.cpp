/*
||
|| @file Keypad.cpp
|| @version 3.1
|| @author Mark Stanley, Alexander Brevig
|| @contact mstanley@technologist.com, alexanderbrevig@gmail.com
||
|| @description
|| | This library provides a simple interface for using matrix
|| | keypads. It supports multiple keypresses while maintaining
|| | backwards compatibility with the old single key library.
|| | It also supports user selectable pins and definable keymaps.
|| #
||
|| @license
|| | This library is free software; you can redistribute it and/or
|| | modify it under the terms of the GNU Lesser General Public
|| | License as published by the Free Software Foundation; version
|| | 2.1 of the License.
|| |
|| | This library is distributed in the hope that it will be useful,
|| | but WITHOUT ANY WARRANTY; without even the implied warranty of
|| | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
|| | Lesser General Public License for more details.
|| |
|| | You should have received a copy of the GNU Lesser General Public
|| | License along with this library; if not, write to the Free Software
|| | Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
|| #
||
*/
#include <Keypad.h>

// <<constructor>> Allows custom keymap, pin configuration, and keypad sizes.
Keypad::Keypad(char *userKeymap, byte *row, byte *col, byte numRows, byte numCols) {
	rowPins = row;
	columnPins = col;
	sizeKpd.rows = numRows;
	sizeKpd.columns = numCols;

	begin(userKeymap);

	setDebounceTime(10);
	setHoldTime(500);
	keypadEventListener = 0;

	startTime = 0;
	single_key = false;
}

// Let the user define a keymap - assume the same row/column count as defined in constructor
void Keypad::begin(char *userKeymap) {
    keymap = userKeymap;
}

// Returns a single key only. Retained for backwards compatibility.
char Keypad::getKey() {
	single_key = true;

	if (getKeys() && key[0].stateChanged && (key[0].kstate==PRESSED))
		return key[0].kchar;
	
	single_key = false;

	return NO_KEY;
}

// Populate the key list.
bool Keypad::getKeys() {
	bool keyActivity = false;

	// Limit how often the keypad is scanned. This makes the loop() run 10 times as fast.
	if ( (millis()-startTime)>debounceTime ) {
		scanKeys();
		keyActivity = updateList();
		startTime = millis();
	}

	return keyActivity;
}

// Private : Hardware scan
void Keypad::scanKeys() {
	// Re-intialize the row pins. Allows sharing these pins with other hardware.
	for (byte r=0; r<sizeKpd.rows; r++) {
		// Logic needs to be inverted. This way the ESP32’s EXT1 wakeup can be used to detect if the accelerometer pin or any button pin goes high
		// original from Keypad
		// pin_mode(rowPins[r],INPUT_PULLUP);
		// changed for usage in OMOTE
		pin_mode(rowPins[r],INPUT);
	}

	// bitMap stores ALL the keys that are being pressed.
	for (byte c=0; c<sizeKpd.columns; c++) {
		pin_mode(columnPins[c],OUTPUT);
		// original from Keypad
		// pin_write(columnPins[c], LOW);	// Begin column pulse output.
		// changed for usage in OMOTE
		pin_write(columnPins[c], HIGH);	// Begin column pulse output.
		for (byte r=0; r<sizeKpd.rows; r++) {
			// original from Keypad
			// bitWrite(bitMap[r], c, !pin_read(rowPins[r]));  // keypress is active low so invert to high.
			// changed for usage in OMOTE
			bitWrite(bitMap[r], c, pin_read(rowPins[r]));  // keypress is active low so invert to high.
		}
		// Set pin to high impedance input. Effectively ends column pulse.
		// original from Keypad
		// pin_write(columnPins[c],HIGH);
		// changed for usage in OMOTE
		pin_write(columnPins[c], LOW);
		pin_mode(columnPins[c],INPUT);
	}
}

// Manage the list without rearranging the keys. Returns true if any keys on the list changed state.
bool Keypad::updateList() {

	bool anyActivity = false;

	// Delete any IDLE keys
	for (byte i=0; i<LIST_MAX; i++) {
		if (key[i].kstate==IDLE) {
			key[i].kchar = NO_KEY;
			key[i].kcode = -1;
			key[i].stateChanged = false;
		}
	}

	// Add new keys to empty slots in the key list.
	for (byte r=0; r<sizeKpd.rows; r++) {
		for (byte c=0; c<sizeKpd.columns; c++) {
			boolean button = bitRead(bitMap[r],c);
			char keyChar = keymap[r * sizeKpd.columns + c];
			int keyCode = r * sizeKpd.columns + c;
			int idx = findInList (keyCode);
			// Key is already on the list so set its next state.
			if (idx > -1)	{
				nextKeyState(idx, button);
			}
			// Key is NOT on the list so add it.
			if ((idx == -1) && button) {
				for (byte i=0; i<LIST_MAX; i++) {
					if (key[i].kchar==NO_KEY) {		// Find an empty slot or don't add key to list.
						key[i].kchar = keyChar;
						key[i].kcode = keyCode;
						key[i].kstate = IDLE;		// Keys NOT on the list have an initial state of IDLE.
						nextKeyState (i, button);
						break;	// Don't fill all the empty slots with the same key.
					}
				}
			}
		}
	}

	// Report if the user changed the state of any key.
	for (byte i=0; i<LIST_MAX; i++) {
		if (key[i].stateChanged) anyActivity = true;
	}

	return anyActivity;
}

// Private
// This function is a state machine but is also used for debouncing the keys.
void Keypad::nextKeyState(byte idx, boolean button) {
	key[idx].stateChanged = false;

	switch (key[idx].kstate) {
		case IDLE:
			if (button==CLOSED) {
				transitionTo (idx, PRESSED);
				holdTimer = millis(); }		// Get ready for next HOLD state.
			break;
		case PRESSED:
			if ((millis()-holdTimer)>holdTime)	// Waiting for a key HOLD...
				transitionTo (idx, HOLD);
			else if (button==OPEN)				// or for a key to be RELEASED.
				transitionTo (idx, RELEASED);
			break;
		case HOLD:
			if (button==OPEN)
				transitionTo (idx, RELEASED);
			break;
		case RELEASED:
			transitionTo (idx, IDLE);
			break;
	}
}

// New in 2.1
bool Keypad::isPressed(char keyChar) {
	for (byte i=0; i<LIST_MAX; i++) {
		if ( key[i].kchar == keyChar ) {
			if ( (key[i].kstate == PRESSED) && key[i].stateChanged )
				return true;
		}
	}
	return false;	// Not pressed.
}

// Search by character for a key in the list of active keys.
// Returns -1 if not found or the index into the list of active keys.
int Keypad::findInList (char keyChar) {
	for (byte i=0; i<LIST_MAX; i++) {
		if (key[i].kchar == keyChar) {
			return i;
		}
	}
	return -1;
}

// Search by code for a key in the list of active keys.
// Returns -1 if not found or the index into the list of active keys.
int Keypad::findInList (int keyCode) {
	for (byte i=0; i<LIST_MAX; i++) {
		if (key[i].kcode == keyCode) {
			return i;
		}
	}
	return -1;
}

// New in 2.0
char Keypad::waitForKey() {
	char waitKey = NO_KEY;
	while( (waitKey = getKey()) == NO_KEY );	// Block everything while waiting for a keypress.
	return waitKey;
}

// Backwards compatibility function.
KeyState Keypad::getState() {
	return key[0].kstate;
}

// The end user can test for any changes in state before deciding
// if any variables, etc. needs to be updated in their code.
bool Keypad::keyStateChanged() {
	return key[0].stateChanged;
}

// The number of keys on the key list, key[LIST_MAX], equals the number
// of bytes in the key list divided by the number of bytes in a Key object.
byte Keypad::numKeys() {
	return sizeof(key)/sizeof(Key);
}

// Minimum debounceTime is 1 mS. Any lower *will* slow down the loop().
void Keypad::setDebounceTime(uint debounce) {
	debounce<1 ? debounceTime=1 : debounceTime=debounce;
}

void Keypad::setHoldTime(uint hold) {
    holdTime = hold;
}

void Keypad::addEventListener(void (*listener)(char)){
	keypadEventListener = listener;
}

void Keypad::transitionTo(byte idx, KeyState nextState) {
	key[idx].kstate = nextState;
	key[idx].stateChanged = true;

	// Sketch used the getKey() function.
	// Calls keypadEventListener only when the first key in slot 0 changes state.
	if (single_key)  {
	  	if ( (keypadEventListener!=NULL) && (idx==0) )  {
			keypadEventListener(key[0].kchar);
		}
	}
	// Sketch used the getKeys() function.
	// Calls keypadEventListener on any key that changes state.
	else {
	  	if (keypadEventListener!=NULL)  {
			keypadEventListener(key[idx].kchar);
		}
	}
}

/*
|| @changelog
|| | 3.1 2013-01-15 - Mark Stanley     : Fixed missing RELEASED & IDLE status when using a single key.
|| | 3.0 2012-07-12 - Mark Stanley     : Made library multi-keypress by default. (Backwards compatible)
|| | 3.0 2012-07-12 - Mark Stanley     : Modified pin functions to support Keypad_I2C
|| | 3.0 2012-07-12 - Stanley & Young  : Removed static variables. Fix for multiple keypad objects.
|| | 3.0 2012-07-12 - Mark Stanley     : Fixed bug that caused shorted pins when pressing multiple keys.
|| | 2.0 2011-12-29 - Mark Stanley     : Added waitForKey().
|| | 2.0 2011-12-23 - Mark Stanley     : Added the public function keyStateChanged().
|| | 2.0 2011-12-23 - Mark Stanley     : Added the private function scanKeys().
|| | 2.0 2011-12-23 - Mark Stanley     : Moved the Finite State Machine into the function getKeyState().
|| | 2.0 2011-12-23 - Mark Stanley     : Removed the member variable lastUdate. Not needed after rewrite.
|| | 1.8 2011-11-21 - Mark Stanley     : Added decision logic to compile WProgram.h or Arduino.h
|| | 1.8 2009-07-08 - Alexander Brevig : No longer uses arrays
|| | 1.7 2009-06-18 - Alexander Brevig : Every time a state changes the keypadEventListener will trigger, if set.
|| | 1.7 2009-06-18 - Alexander Brevig : Added setDebounceTime. setHoldTime specifies the amount of
|| |                                          microseconds before a HOLD state triggers
|| | 1.7 2009-06-18 - Alexander Brevig : Added transitionTo
|| | 1.6 2009-06-15 - Alexander Brevig : Added getState() and state variable
|| | 1.5 2009-05-19 - Alexander Brevig : Added setHoldTime()
|| | 1.4 2009-05-15 - Alexander Brevig : Added addEventListener
|| | 1.3 2009-05-12 - Alexander Brevig : Added lastUdate, in order to do simple debouncing
|| | 1.2 2009-05-09 - Alexander Brevig : Changed getKey()
|| | 1.1 2009-04-28 - Alexander Brevig : Modified API, and made variables private
|| | 1.0 2007-XX-XX - Mark Stanley : Initial Release
|| #
*/
