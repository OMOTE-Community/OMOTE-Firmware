#include <lvgl.h>

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_GRADIENTLEFT
#define LV_ATTRIBUTE_IMG_GRADIENTLEFT
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_GRADIENTLEFT uint8_t gradientLeft_map[] = {
  0xfa, 0xf2, 0xea, 0xe2, 0xda, 0xd1, 0xc7, 0xbe, 0xb7, 0xae, 0xa6, 0x9e, 0x95, 0x8d, 0x84, 0x7d, 0x74, 0x6c, 0x62, 0x5a, 0x51, 0x48, 0x41, 0x38, 0x2f, 0x28, 0x1f, 0x17, 0x0f, 0x07, 
};

const lv_img_dsc_t gradientLeft = {
  .header.cf = LV_IMG_CF_ALPHA_8BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 30,
  .header.h = 1,
  .data_size = 30,
  .data = gradientLeft_map,
};


#ifndef LV_ATTRIBUTE_IMG_GRADIENTRIGHT
#define LV_ATTRIBUTE_IMG_GRADIENTRIGHT
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_GRADIENTRIGHT uint8_t gradientRight_map[] = {
  0x07, 0x0f, 0x17, 0x1f, 0x28, 0x2f, 0x38, 0x41, 0x48, 0x51, 0x5a, 0x62, 0x6c, 0x74, 0x7d, 0x84, 0x8d, 0x95, 0x9e, 0xa6, 0xae, 0xb7, 0xbe, 0xc7, 0xd1, 0xda, 0xe2, 0xea, 0xf2, 0xfa, 
};

const lv_img_dsc_t gradientRight = {
  .header.cf = LV_IMG_CF_ALPHA_8BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 30,
  .header.h = 1,
  .data_size = 30,
  .data = gradientRight_map,
};
