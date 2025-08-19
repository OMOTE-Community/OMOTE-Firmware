/* @file   DynamicKeypad.pde
|| @version 1.2
|| @author Mark Stanley
|| @contact mstanley@technologist.com
||
||  07/11/12 - Re-modified (from DynamicKeypadJoe2) to use direct-connect kpds
||  02/28/12 - Modified to use I2C i/o G. D. (Joe) Young
||
||
|| @dificulty:  Intermediate
||
|| @description
|| |    This is a demonstration of keypadEvents. It's used to switch between keymaps
|| |    while using only one keypad.  The main concepts being demonstrated are:
|| |
|| |        Using the keypad events, PRESSED, HOLD and RELEASED to simplify coding.
|| |        How to use setHoldTime() and why.
|| |        Making more than one thing happen with the same key.
|| |        Assigning and changing keymaps on the fly.
|| |
|| |    Another useful feature is also included with this demonstration although
|| |    it's not really one of the concepts that I wanted to show you.  If you look
|| |    at the code in the PRESSED event you will see that the first section of that
|| |    code is used to scroll through three different letters on each key.  For
|| |    example, pressing the '2' key will step through the letters 'd', 'e' and 'f'.
|| |
|| |
|| |  Using the keypad events, PRESSED, HOLD and RELEASED to simplify coding
|| |    Very simply, the PRESSED event occurs imediately upon detecting a pressed
|| |    key and will not happen again until after a RELEASED event.  When the HOLD
|| |    event fires it always falls between PRESSED and RELEASED.  However, it will
|| |    only occur if a key has been pressed for longer than the setHoldTime() interval.
|| |
|| |  How to use setHoldTime() and why
|| |    Take a look at keypad.setHoldTime(500) in the code.  It is used to set the
|| |    time delay between a PRESSED event and the start of a HOLD event.  The value
|| |    500 is in milliseconds (mS) and is equivalent to half a second.  After pressing
|| |    a key for 500mS the HOLD event will fire and any code contained therein will be
|| |    executed.  This event will stay active for as long as you hold the key except
|| |    in the case of bug #1 listed above.
|| |
|| |  Making more than one thing happen with the same key.
|| |    If you look under the PRESSED event (case PRESSED:) you will see that the '#'
|| |    is used to print a new line, Serial.println().  But take a look at the first
|| |    half of the HOLD event and you will see the same key being used to switch back
|| |    and forth between the letter and number keymaps that were created with alphaKeys[4][5]
|| |    and numberKeys[4][5] respectively.
|| |
|| |  Assigning and changing keymaps on the fly
|| |    You will see that the '#' key has been designated to perform two different functions
|| |    depending on how long you hold it down.  If you press the '#' key for less than the
|| |    setHoldTime() then it will print a new line.  However, if you hold if for longer
|| |    than that it will switch back and forth between numbers and letters.  You can see the
|| |    keymap changes in the HOLD event.
|| |
|| |
|| |  In addition...
|| |      You might notice a couple of things that you won't find in the Arduino language
|| |    reference.  The first would be #include <ctype.h>.  This is a standard library from
|| |    the C programming language and though I don't normally demonstrate these types of
|| |    things from outside the Arduino language reference I felt that its use here was
|| |    justified by the simplicity that it brings to this sketch.
|| |      That simplicity is provided by the two calls to isalpha(key) and isdigit(key).
|| |    The first one is used to decide if the key that was pressed is any letter from a-z
|| |    or A-Z and the second one decides if the key is any number from 0-9.  The return
|| |    value from these two functions is either a zero or some positive number greater
|| |    than zero.  This makes it very simple to test a key and see if it is a number or
|| |    a letter.  So when you see the following:
|| |
|| |    if (isalpha(key))    // this tests to see if your key was a letter
|| |
|| |    And the following may be more familiar to some but it is equivalent:
|| |
|| |    if (isalpha(key) != 0)   // this tests to see if your key was a letter
|| |
|| |  And Finally...
|| |    To better understand how the event handler affects your code you will need to remember
|| |    that it gets called only when you press, hold or release a key.  However, once a key
|| |    is pressed or held then the event handler gets called at the full speed of the loop().
|| |
|| #
*/
#include <Keypad.h>
#include <ctype.h>

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
// Define the keymaps.  The blank spot (lower left) is the space character.
char alphaKeys[ROWS][COLS] = {
    { 'a','d','g' },
    { 'j','m','p' },
    { 's','v','y' },
    { ' ','.','#' }
};

char numberKeys[ROWS][COLS] = {
    { '1','2','3' },
    { '4','5','6' },
    { '7','8','9' },
    { ' ','0','#' }
};

boolean alpha = false;   // Start with the numeric keypad.

byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {8, 7, 6}; //connect to the column pinouts of the keypad

// Create two new keypads, one is a number pad and the other is a letter pad.
Keypad numpad( makeKeymap(numberKeys), rowPins, colPins, sizeof(rowPins), sizeof(colPins) );
Keypad ltrpad( makeKeymap(alphaKeys), rowPins, colPins, sizeof(rowPins), sizeof(colPins) );


unsigned long startTime;
const byte ledPin = 13;                            // Use the LED on pin 13.

void setup() {
    Serial.begin(9600);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);                 // Turns the LED on.
    ltrpad.begin( makeKeymap(alphaKeys) );
    numpad.begin( makeKeymap(numberKeys) );
    ltrpad.addEventListener(keypadEvent_ltr);  // Add an event listener.
    ltrpad.setHoldTime(500);                   // Default is 1000mS
    numpad.addEventListener(keypadEvent_num);  // Add an event listener.
    numpad.setHoldTime(500);                   // Default is 1000mS
}

char key;

void loop() {

    if( alpha )
        key = ltrpad.getKey( );
    else
        key = numpad.getKey( );

    if (alpha && millis()-startTime>100) {           // Flash the LED if we are using the letter keymap.
        digitalWrite(ledPin,!digitalRead(ledPin));
        startTime = millis();
    }
}

static char virtKey = NO_KEY;      // Stores the last virtual key press. (Alpha keys only)
static char physKey = NO_KEY;      // Stores the last physical key press. (Alpha keys only)
static char buildStr[12];
static byte buildCount;
static byte pressCount;

static byte kpadState;

// Take care of some special events.

void keypadEvent_ltr(KeypadEvent key) {
    // in here when in alpha mode.
    kpadState = ltrpad.getState( );
    swOnState( key );
} // end ltrs keypad events

void keypadEvent_num( KeypadEvent key ) {
    // in here when using number keypad
    kpadState = numpad.getState( );
    swOnState( key );
} // end numbers keypad events

void swOnState( char key ) {
    switch( kpadState ) {
        case PRESSED:
            if (isalpha(key)) {              // This is a letter key so we're using the letter keymap.
                if (physKey != key) {        // New key so start with the first of 3 characters.
                    pressCount = 0;
                    virtKey = key;
                    physKey = key;
                }
                else {                       // Pressed the same key again...
                    virtKey++;                   // so select the next character on that key.
                    pressCount++;                // Tracks how many times we press the same key.
                }
                    if (pressCount > 2) {    // Last character reached so cycle back to start.
                        pressCount = 0;
                        virtKey = key;
                    }
                    Serial.print(virtKey);   // Used for testing.
                }
                if (isdigit(key) || key == ' ' || key == '.')
                    Serial.print(key);
                if (key == '#')
                    Serial.println();
                break;

        case HOLD:
            if (key == '#')  {               // Toggle between keymaps.
                if (alpha == true)  {        // We are currently using a keymap with letters
                    alpha = false;           // Now we want a keymap with numbers.
                    digitalWrite(ledPin, LOW);
                }
                else  {                      // We are currently using a keymap with numbers
                    alpha = true;            // Now we want a keymap with letters.
                }
            }
            else  {                          // Some key other than '#' was pressed.
                buildStr[buildCount++] = (isalpha(key)) ? virtKey : key;
                buildStr[buildCount] = '\0';
                Serial.println();
                Serial.println(buildStr);
            }
            break;

        case RELEASED:
            if (buildCount >= sizeof(buildStr))  buildCount = 0;  // Our string is full. Start fresh.
            break;
    }  // end switch-case
}// end switch on state function

