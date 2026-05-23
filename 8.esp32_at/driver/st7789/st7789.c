#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "stm32f4xx.h"
#include "cpu_delay.h"
#include "st7789.h"
#include "st7789_desc.h"
#include "font.h"
#include "image.h"

#define delay_us(us)      cpu_delay(us)       // 微秒
#define delay_ms(ms)      cpu_delay((ms)*1000) // 毫秒 → 转成微秒再调用cpu_delay

// SPI2 写一个字节
static void SPI2_WriteByte(uint8_t data)
{
    while(SPI_I2S_GetFlagStatus(ST7789_SPI, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(ST7789_SPI, data);
    while(SPI_I2S_GetFlagStatus(ST7789_SPI, SPI_I2S_FLAG_RXNE) == RESET);
    SPI_I2S_ReceiveData(ST7789_SPI);
}

// 写命令
static void ST7789_WriteCmd(uint8_t cmd)
{
    ST7789_DC(0);// DC 0 表示命令
    ST7789_CS(0);// 拉低 CS 
    SPI2_WriteByte(cmd);
    ST7789_CS(1);// 释放 CS
}

static void ST7789_WriteData8(uint8_t data)
{
    ST7789_DC(1);// DC 1 表示数据
    ST7789_CS(0);// 拉低 CS 
    SPI2_WriteByte(data);
    ST7789_CS(1);// 释放 CS
}

static void ST7789_WriteData16(uint16_t data)
{
    ST7789_DC(1);// DC 1 表示数据
    ST7789_CS(0);// 拉低 CS 
    SPI2_WriteByte(data >> 8);
    SPI2_WriteByte(data & 0xFF);
    ST7789_CS(1);// 释放 CS
}

void ST7789_HW_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    // 1. 使能时钟
    RCC_AHB1PeriphClockCmd(ST7789_CS_CLK | ST7789_RES_CLK | ST7789_DC_CLK | ST7789_BL_CLK |
                           ST7789_SCK_CLK | ST7789_MISO_CLK | ST7789_MOSI_CLK, ENABLE);
    RCC_APB1PeriphClockCmd(ST7789_SPI_CLK, ENABLE);

    // 2. 初始化控制引脚 (推挽输出)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    
    GPIO_InitStructure.GPIO_Pin = ST7789_CS_PIN;
    GPIO_Init(ST7789_CS_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = ST7789_RES_PIN;
    GPIO_Init(ST7789_RES_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = ST7789_DC_PIN;
    GPIO_Init(ST7789_DC_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = ST7789_BL_PIN;
    GPIO_Init(ST7789_BL_PORT, &GPIO_InitStructure);

    // 3. 初始化SPI2引脚 (复用推挽)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    
    GPIO_InitStructure.GPIO_Pin = ST7789_SCK_PIN;
    GPIO_Init(ST7789_SCK_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = ST7789_MISO_PIN;
    GPIO_Init(ST7789_MISO_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = ST7789_MOSI_PIN;
    GPIO_Init(ST7789_MOSI_PORT, &GPIO_InitStructure);

    GPIO_PinAFConfig(ST7789_SCK_PORT, ST7789_SCK_SOURCE, ST7789_SCK_AF);
    GPIO_PinAFConfig(ST7789_MISO_PORT, ST7789_MISO_SOURCE, ST7789_MISO_AF);
    GPIO_PinAFConfig(ST7789_MOSI_PORT, ST7789_MOSI_SOURCE, ST7789_MOSI_AF);

    // 4. SPI2 参数配置
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High; // 模式3
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2; // 高速
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(ST7789_SPI, &SPI_InitStructure);
    
    SPI_Cmd(ST7789_SPI, ENABLE);
}

static void ST7789_Reset(void)
{
    ST7789_RES(0);
    delay_ms(20);
    ST7789_RES(1);
    delay_ms(120);
}

void ST7789_Init(void)
{
    ST7789_HW_Init();
    ST7789_Reset();
    ST7789_WriteCmd(0x11);     // Sleep out
    delay_ms(120);                //ms            

    ST7789_WriteCmd(0x36);     
    ST7789_WriteData8(0x00);   

    ST7789_WriteCmd(0x3A);     
    ST7789_WriteData8(0x55);   

    ST7789_WriteCmd(0xB2);     
    ST7789_WriteData8(0x0C);   
    ST7789_WriteData8(0x0C);   
    ST7789_WriteData8(0x00);   
    ST7789_WriteData8(0x33);   
    ST7789_WriteData8(0x33);   

    ST7789_WriteCmd(0xB7);     
    ST7789_WriteData8(0x46);   //VGH=13.65V,VGL=-11.38V

    ST7789_WriteCmd(0xBB);     
    ST7789_WriteData8(0x1B);   

    ST7789_WriteCmd(0xC0);     
    ST7789_WriteData8(0x2C);   

    ST7789_WriteCmd(0xC2);     
    ST7789_WriteData8(0x01);   

    ST7789_WriteCmd(0xC3);     
    ST7789_WriteData8(0x0F);   

    ST7789_WriteCmd(0xC4);     
    ST7789_WriteData8(0x20);   

    ST7789_WriteCmd(0xC6);     
    ST7789_WriteData8(0x0F);   

    ST7789_WriteCmd(0xD0);     
    ST7789_WriteData8(0xA4);   
    ST7789_WriteData8(0xA1);   

    ST7789_WriteCmd(0xD6);     
    ST7789_WriteData8(0xA1); 

    ST7789_WriteCmd(0xE0);
    ST7789_WriteData8(0xF0);
    ST7789_WriteData8(0x00);
    ST7789_WriteData8(0x06);
    ST7789_WriteData8(0x04);
    ST7789_WriteData8(0x05);
    ST7789_WriteData8(0x05);
    ST7789_WriteData8(0x31);
    ST7789_WriteData8(0x44);
    ST7789_WriteData8(0x48);
    ST7789_WriteData8(0x36);
    ST7789_WriteData8(0x12);
    ST7789_WriteData8(0x12);
    ST7789_WriteData8(0x2B);
    ST7789_WriteData8(0x34);

    ST7789_WriteCmd(0xE1);
    ST7789_WriteData8(0xF0);
    ST7789_WriteData8(0x0B);
    ST7789_WriteData8(0x0F);
    ST7789_WriteData8(0x0F);
    ST7789_WriteData8(0x0D);
    ST7789_WriteData8(0x26);
    ST7789_WriteData8(0x31);
    ST7789_WriteData8(0x43);
    ST7789_WriteData8(0x47);
    ST7789_WriteData8(0x38);
    ST7789_WriteData8(0x14);
    ST7789_WriteData8(0x14);
    ST7789_WriteCmd(0x2C);
    ST7789_WriteData8(0x32);

    ST7789_WriteCmd(0x21);

    ST7789_WriteCmd(0x29);      // Display on

    ST7789_BL(1); // 打开背光
}

// 在指定位置画一个像素点
void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    ST7789_SetWindow(x, y, x, y);
    ST7789_WriteData16(color);
}

void ST7789_Clear(uint16_t color)
{
    uint16_t i, j;
    ST7789_SetWindow(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    ST7789_DC(1);// 数据
    ST7789_CS(0);// 拉低 CS
    for (i = 0; i < LCD_WIDTH; i++)
    {
        for (j = 0; j < LCD_HEIGHT; j++)
        {
            ST7789_WriteData16(color);
        }
    }
    ST7789_CS(1);// 释放 CS
}

void ST7789_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    uint16_t i, j;
    ST7789_SetWindow(x, y, x + w - 1, y + h - 1);
    ST7789_DC(1);
    ST7789_CS(0);
    for(i = 0; i < w; i++)
    {
        for(j=0; j < h; j++)
        {
            ST7789_WriteData16(color);
        }
    }
    ST7789_CS(1);
}

void ST7789_ShowChar(uint16_t x, uint16_t y, char ch, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    uint8_t i, j, temp;
    uint16_t char_len = 0;
    uint16_t x0 = x;
    const uint8_t *font_buf = NULL;

    ch -= ' ';

    if(sizey == 16)
    {
        char_len = 16;
        font_buf = font16.model;
    }
    else if(sizey == 32)
    {
        char_len = 64;
        font_buf = font32.model;
    }
    else if(sizey == 48)
    {
        char_len = 144;
        font_buf = font48.model;
    }
    else
    {
        return; 
    }

    for(i = 0; i < char_len; i++)
    {
        temp = font_buf[ch * char_len + i];
        for(j = 0; j < 8; j++)
        {
            if(temp & 0x80)
                ST7789_DrawPixel(x, y, fc);
            else if(mode == 0)
                ST7789_DrawPixel(x, y, bc);

            temp <<= 1;
            x++;
            if((x - x0) == sizey / 2)
            {
                x = x0;
                y++;
                break;
            }
        }
    }
}

void ST7789_ShowString(uint16_t x, uint16_t y, const char *str, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    while(*str != '\0')
    {
        if(x > (LCD_WIDTH - sizey / 2))
        {
            x = 0;
            y += sizey;
        }
        if(y > (LCD_HEIGHT - sizey))
        {
            y = 0;
        }
        ST7789_ShowChar(x, y, *str, fc, bc, sizey, mode);
        x += sizey / 2;
        str++;
    }
}

// 设置窗口
void ST7789_SetWindow(uint16_t xStar, uint16_t yStar, uint16_t xEnd, uint16_t yEnd) {
    ST7789_CS(0); // 全程拉低片选

    // 1. 设置 X 坐标范围
    ST7789_DC(0); // 命令模式
    SPI2_WriteByte(0x2A);
    ST7789_DC(1); // 数据模式
    SPI2_WriteByte(xStar >> 8);
    SPI2_WriteByte(xStar & 0xFF);
    SPI2_WriteByte(xEnd >> 8);
    SPI2_WriteByte(xEnd & 0xFF);

    // 2. 设置 Y 坐标范围
    ST7789_DC(0); // 命令模式
    SPI2_WriteByte(0x2B);
    ST7789_DC(1); // 数据模式
    SPI2_WriteByte(yStar >> 8);
    SPI2_WriteByte(yStar & 0xFF);
    SPI2_WriteByte(yEnd >> 8);
    SPI2_WriteByte(yEnd & 0xFF);

    // 3. 发送开始写入内存命令 
    ST7789_DC(0); // 命令模式
    SPI2_WriteByte(0x2C); 
    
    ST7789_CS(1); // 设置完毕，释放片选
}

void ST7789_ShowImage(uint16_t x, uint16_t y, const ImageDef *image)
{
    uint32_t i;
    uint16_t w = image->width;
    uint16_t h = image->height;
    const uint8_t *p = image->data;
    uint16_t color;

    if(x + w > LCD_WIDTH)
        w = LCD_WIDTH - x;
    if(y + h > LCD_HEIGHT)
        h = LCD_HEIGHT - y;
    if(w == 0 || h == 0)
        return;

    ST7789_SetWindow(x, y, x + w - 1, y + h - 1);
    
    ST7789_DC(1);
    ST7789_CS(0);
    

    // 每次取2个字节，交换高低位
    for (i = 0; i < w * h; i++)
    {
        color  = (p[2*i + 1] << 8) | p[2*i]; // 交换字节顺序
        SPI2_WriteByte(color >> 8);
        SPI2_WriteByte(color & 0xFF);
    }

    ST7789_CS(1);
}

