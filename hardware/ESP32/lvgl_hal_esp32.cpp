#include <lvgl.h>
#include "tft_hal_esp32.h"
#include "sleep_hal_esp32.h"

// -----------------------
// https://docs.lvgl.io/8.3/porting/display.html?highlight=lv_disp_draw_buf_init#buffering-modes
// With two buffers, the rendering and refreshing of the display become parallel operations
// Second buffer needs 15.360 bytes more memory in heap.
#define useTwoBuffersForlvgl

// Display flushing
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p ){
  uint32_t w = ( area->x2 - area->x1 + 1 );
  uint32_t h = ( area->y2 - area->y1 + 1 );

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  #ifdef useTwoBuffersForlvgl
  tft.pushPixelsDMA((uint16_t*)&color_p->full, w * h);
  #else
  tft.pushColors((uint16_t*)&color_p->full, w * h, true);
  #endif
  tft.endWrite();

  lv_disp_flush_ready( disp );
}

// Read the touchpad
void my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data) {
    uint16_t x, y;
    if (tft.getTouch(&x, &y)) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = x;
        data->point.y = y;
        setLastActivityTimestamp_HAL();    
        
        // Uncomment this to show the touchpoint
        //tft.drawFastHLine(0, y, SCR_WIDTH, TFT_RED);
        //tft.drawFastVLine(x, 0, SCR_HEIGHT, TFT_RED);
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

static lv_disp_draw_buf_t draw_buf;

void init_lvgl_HAL() {
  // first init TFT
  init_tft();

  #ifdef useTwoBuffersForlvgl
  lv_color_t * bufA = (lv_color_t *) malloc(sizeof(lv_color_t) * SCR_WIDTH * SCR_HEIGHT / 10);
  lv_color_t * bufB = (lv_color_t *) malloc(sizeof(lv_color_t) * SCR_WIDTH * SCR_HEIGHT / 10);
  lv_disp_draw_buf_init(&draw_buf, bufA, bufB, SCR_WIDTH * SCR_HEIGHT / 10);
  #else
  lv_color_t * bufA = (lv_color_t *) malloc(sizeof(lv_color_t) * SCR_WIDTH * SCR_HEIGHT / 10);
  lv_disp_draw_buf_init(&draw_buf, bufA, NULL, SCR_WIDTH * SCR_HEIGHT / 10);
  #endif

  // Initialize the display driver --------------------------------------------------------------------------
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init( &disp_drv );
  disp_drv.hor_res = SCR_WIDTH;
  disp_drv.ver_res = SCR_HEIGHT;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register( &disp_drv );

  // Initialize the touchscreen driver
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init( &indev_drv );
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register( &indev_drv );

}
