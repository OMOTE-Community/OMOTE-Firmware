#include "keypad_gui.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "key_map.h"

struct KeyPadManager {
  SDL_Window *window;
  SDL_Renderer *renderer;
  std::vector<KeyPadKey> keys;
};

SDL_Surface* loadSurface( SDL_Surface* screenSurface );

std::queue<KeyEvent> keyEventsQueue;

// https://wrfranklin.org/Research/Short_Notes/pnpoly.html
int pnpoly(int nvert, float *vertx, float *verty, float testx, float testy)
{
  int i, j, c = 0;
  for (i = 0, j = nvert-1; i < nvert; j = i++) {
    if ( ((verty[i]>testy) != (verty[j]>testy)) &&
	 (testx < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) )
       c = !c;
  }
  return c;
}

int event_filter(void *userdata, SDL_Event * event) {
  KeyPadManager* manager = (KeyPadManager*) userdata;

  // Capture all window close events
  if (event->type == SDL_WINDOWEVENT) {
    if (event->window.event == SDL_WINDOWEVENT_CLOSE) {
      exit(0);
    }
  }

  // The event api isn't consistent with the window id, so we need to check the event type
  uint32_t win_id = UINT32_MAX;
  switch(event->type) {
      case SDL_MOUSEBUTTONUP:
      case SDL_MOUSEBUTTONDOWN:
          win_id = event->button.windowID;
          break;
      case SDL_MOUSEMOTION:
          win_id = event->motion.windowID;
          break;
      case SDL_FINGERUP:
      case SDL_FINGERDOWN:
      case SDL_FINGERMOTION:
#if SDL_VERSION_ATLEAST(2,0,12)
          win_id = event->tfinger.windowID;
#endif
          break;
      case SDL_WINDOWEVENT:
          win_id = event->window.windowID;
          break;
      default:
          return 1;
  }

  // Make sure that lvgl doesn't get events for our window
  if (win_id == SDL_GetWindowID(manager->window)) {
    switch(event->type) {
      case SDL_MOUSEBUTTONUP:
      case SDL_MOUSEBUTTONDOWN:
        SDL_MouseButtonEvent *mouse_event = (SDL_MouseButtonEvent*) event;
        for (auto const &key : manager->keys)
        {
          // Check if the mouse event is inside one of the key polygons
          if (pnpoly(key.num_vert, key.vertx, key.verty, mouse_event->x, mouse_event->y))
          {
            KeyEvent keyEvent;
            keyEvent.keyChar = key.key;
            keyEvent.keyCode = key.id;
            keyEvent.keyState = event->type == SDL_MOUSEBUTTONDOWN ? PRESSED_SIMULATOR : RELEASED_SIMULATOR;
            // printf("simulator click event: %c, %d %d, %d, added to queue\r\n", keyEvent.keyChar, keyEvent.keyCode/5, keyEvent.keyCode%5, keyEvent.keyState);
            keyEventsQueue.push(keyEvent);
            break;
          }
        }
    } 
    return 0;
  }

  return 1;
}

SDL_Window* keypad_gui_setup() {
  SDL_Window *window = SDL_CreateWindow("OMOTE Keypad",
                                      SDL_WINDOWPOS_CENTERED,
                                      SDL_WINDOWPOS_CENTERED,
                                      233, 537,
                                      0);

  Uint32 render_flags = SDL_RENDERER_SOFTWARE; 

  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, render_flags);

  SDL_Surface *screenSurface = SDL_GetWindowSurface( window );

  // Load the keypad image
  SDL_Surface* buttonsSurface = loadSurface(screenSurface);
  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, buttonsSurface); 
	SDL_FreeSurface(buttonsSurface); 
 
  // Draw the keypad
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, nullptr, nullptr);
  SDL_RenderPresent(renderer);

  KeyPadManager *manager = new KeyPadManager();
  manager->window = window;
  manager->renderer = renderer;
  manager->keys = loadKeypadMap();

  // Filter SDL events to get the events we need
  SDL_SetEventFilter(event_filter, manager);

  return window;
}

SDL_Surface* loadSurface( SDL_Surface* screenSurface )
{
  //The final optimized image
  SDL_Surface* optimizedSurface = NULL;

  //Load image at specified path
  SDL_Surface* loadedSurface = IMG_Load( "./hardware/windows_linux/keypad_gui/buttons.png" );
  if( loadedSurface == NULL )
  {
    printf( "Unable to load image! SDL_image Error: %s\n", IMG_GetError() );
  }
  else
  {
      //Convert surface to screen format
      optimizedSurface = SDL_ConvertSurface( loadedSurface, screenSurface->format, 0 );
      if( optimizedSurface == NULL )
      {
        printf( "Unable to optimize image! SDL Error: %s\n", SDL_GetError() );
      }

      //Get rid of old loaded surface
      SDL_FreeSurface( loadedSurface );
  }

  return optimizedSurface;
}
