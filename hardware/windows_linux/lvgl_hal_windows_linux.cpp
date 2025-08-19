#include <stdlib.h>
#include <sys/time.h>
#include <lvgl.h>
#include <SDL2/SDL_thread.h>
#include "sdl/sdl.h"
#include "SDL2/SDL_events.h"

#include "keypad_gui/keypad_gui.h"

/**
 * A task to measure the elapsed time for LittlevGL
 * @param data unused
 * @return never return
 */
static int tick_thread(void * data)
{
    Uint64 lastTimestamp = SDL_GetTicks64();
    Uint64 newTimestamp = 0;
    while(1) {
        // we don't use this blackbox
        // SDL_Delay(5);   /*Sleep for 5 millisecond*/
        // lv_tick_inc(5); /*Tell lvgl that 5 milliseconds were elapsed*/

        newTimestamp = SDL_GetTicks64();
        if ((newTimestamp - lastTimestamp) > 5) {
          lv_tick_inc(newTimestamp - lastTimestamp);
          lastTimestamp = newTimestamp;
        }
    }

    return 0;
}

static lv_disp_draw_buf_t draw_buf;

void init_lvgl_HAL() {
  // Workaround for sdl2 `-m32` crash
  // https://bugs.launchpad.net/ubuntu/+source/libsdl2/+bug/1775067/comments/7
  #ifndef WIN32
    setenv("DBUS_FATAL_WARNINGS", "0", 1);
  #endif

  #ifdef useTwoBuffersForlvgl
  lv_color_t * bufA = (lv_color_t *) malloc(sizeof(lv_color_t) * SDL_HOR_RES * SDL_VER_RES / 10);
  lv_color_t * bufB = (lv_color_t *) malloc(sizeof(lv_color_t) * SDL_HOR_RES * SDL_VER_RES / 10);
  lv_disp_draw_buf_init(&draw_buf, bufA, bufB, SDL_HOR_RES * SDL_VER_RES / 10);
  #else
  lv_color_t * bufA = (lv_color_t *) malloc(sizeof(lv_color_t) * SDL_HOR_RES * SDL_VER_RES / 10);
  lv_disp_draw_buf_init(&draw_buf, bufA, NULL, SDL_HOR_RES * SDL_VER_RES / 10);
  #endif

  // Initialize the display driver --------------------------------------------------------------------------
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init( &disp_drv );
  disp_drv.hor_res = SDL_HOR_RES;
  disp_drv.ver_res = SDL_VER_RES;
  disp_drv.flush_cb = sdl_display_flush;    /*Used when `LV_VDB_SIZE != 0` in lv_conf.h (buffered drawing)*/
  disp_drv.draw_buf = &draw_buf;
  //disp_drv.disp_fill = monitor_fill;      /*Used when `LV_VDB_SIZE == 0` in lv_conf.h (unbuffered drawing)*/
  //disp_drv.disp_map = monitor_map;        /*Used when `LV_VDB_SIZE == 0` in lv_conf.h (unbuffered drawing)*/
  lv_disp_drv_register( &disp_drv );

  /* Add the mouse as input device
   * Use the 'mouse' driver which reads the PC's mouse*/
  static lv_indev_drv_t indev_drv_mouse;
  lv_indev_drv_init( &indev_drv_mouse );
  indev_drv_mouse.type = LV_INDEV_TYPE_POINTER;
  indev_drv_mouse.read_cb = sdl_mouse_read;  /*This function will be called periodically (by the library) to get the mouse position and state*/
  lv_indev_drv_register( &indev_drv_mouse );

  // /* Add the keyboard as input device
  //  * did not work */
  // static lv_indev_drv_t indev_drv_keyboard;
  // lv_indev_drv_init( &indev_drv_keyboard );
  // indev_drv_keyboard.type = LV_INDEV_TYPE_KEYPAD;
  // indev_drv_keyboard.read_cb = sdl_keyboard_read;  /*This function will be called periodically (by the library) to get the keyboard events*/
  // lv_indev_t *keyboard_device = lv_indev_drv_register( &indev_drv_keyboard );
  // lv_group_t *group = lv_group_create();
  // lv_indev_set_group(keyboard_device, group);
  // lv_group_add_obj(group, lv_scr_act());
  // lv_group_add_obj(group, tabview);
  // lv_group_add_obj(group, lv_tabview_get_content(tabview));
  // lv_group_add_obj(group, tabs);
  //
  // need to be in a loop
  // printf("last key: %d\n",lv_indev_get_key(keyboard_device));

  sdl_init();

  // Get the SDL window via an event
  SDL_Event aWindowIdFinder;
  SDL_PollEvent(&aWindowIdFinder);
  SDL_Window *mSimWindow = SDL_GetWindowFromID(aWindowIdFinder.window.windowID);
  
  SDL_SetWindowTitle(mSimWindow, "OMOTE simulator");

  // Setup the keypad window
  SDL_Window *keypadWindow = keypad_gui_setup();

  // Raise the simulator window, sometimes it will get buried on macOS
  // Unfortunately, at least in WSL2, it is still sometimes buried and has to be brought to the foreground manually
  SDL_RaiseWindow(mSimWindow);

  // Arrange our windows nicely
  int x, y;
  SDL_GetWindowPosition(keypadWindow, &x, &y);
  SDL_SetWindowPosition(mSimWindow, x - (SDL_HOR_RES * SDL_ZOOM) / 2 - 10, y);
  SDL_SetWindowPosition(keypadWindow, x + (SDL_HOR_RES * SDL_ZOOM) / 2 + 10, y);

  /* Tick init.
   * You have to call 'lv_tick_inc()' in periodically to inform lvgl about how much time were elapsed
   * Create an SDL thread to do this*/
  SDL_CreateThread(tick_thread, "tick", NULL);
}
