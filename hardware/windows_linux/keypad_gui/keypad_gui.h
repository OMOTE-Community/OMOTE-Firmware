#include <SDL2/SDL.h>
#include <queue>

enum guiKeyStates {IDLE_SIMULATOR, PRESSED_SIMULATOR, RELEASED_SIMULATOR};

const char NO_KEY = '\0';
struct KeyEvent {
  char keyChar;
  int keyCode;
  guiKeyStates keyState;
};

SDL_Window* keypad_gui_setup();

// A queue for all mouse events between two loops
extern std::queue<KeyEvent> keyEventsQueue;