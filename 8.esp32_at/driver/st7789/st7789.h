#ifndef __ST7789_H__
#define __ST7789_H__

#include "stm32f4xx.h"
#include "image.h"

// LCD 屏幕分辨率
#define LCD_WIDTH   240
#define LCD_HEIGHT  320

// 常用颜色定义 (RGB565格式)
#define WHITE       0xFFFF
#define BLACK       0x0000
#define BLUE        0x001F
#define BRED        0XF81F
#define GRED        0XFFE0
#define GBLUE       0X07FF
#define RED         0xF800
#define MAGENTA     0xF81F
#define GREEN       0x07E0
#define CYAN        0x7FFF
#define YELLOW      0xFFE0
#define BROWN       0XBC40 //棕色
#define BRRED       0XFC07 //棕红色
#define GRAY        0X8430 //灰色

#define mkcolor(r, g, b) ((r & 0xF8) << 8) | (g & 0xFC) << 3 | (b >> 3))
//1111 1000 0001 1111
//          1111 1000 F8
//          1111 1100 FC
//					0001 1111 
					
/* 函数声明 */
void ST7789_HW_Init(void); // 硬件初始化
void ST7789_Init(void);    // 屏幕驱动初始化
void ST7789_SetWindow(uint16_t xStar, uint16_t yStar, uint16_t xEnd, uint16_t yEnd);
void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ST7789_Clear(uint16_t color);
void ST7789_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

// 显示ASCII字符和字符串
void ST7789_ShowChar(uint16_t x, uint16_t y, char num, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode);
void ST7789_ShowString(uint16_t x, uint16_t y, const char *p, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode);

// 显示图片
void ST7789_ShowImage(uint16_t x, uint16_t y, const ImageDef *img);

#endif /* __ST7789_H */

