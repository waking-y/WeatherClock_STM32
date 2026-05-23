#ifndef __ST7789_DESC_H__
#define __ST7789_DESC_H__

#include "stm32f4xx.h"

/* ================== 引脚定义 ================== */
// CLK —— PB13
// MOSI —— PC3
// MISO —— PC2
// CS —— PE2
// RESET —— PE3
// DC —— PE6
// BL —— PE5
#define ST7789_CS_PORT       GPIOE
#define ST7789_CS_PIN        GPIO_Pin_2
#define ST7789_CS_CLK        RCC_AHB1Periph_GPIOE

#define ST7789_RES_PORT      GPIOE
#define ST7789_RES_PIN       GPIO_Pin_3
#define ST7789_RES_CLK       RCC_AHB1Periph_GPIOE

#define ST7789_DC_PORT       GPIOE
#define ST7789_DC_PIN        GPIO_Pin_6
#define ST7789_DC_CLK        RCC_AHB1Periph_GPIOE

#define ST7789_BL_PORT       GPIOE
#define ST7789_BL_PIN        GPIO_Pin_5
#define ST7789_BL_CLK        RCC_AHB1Periph_GPIOE

// SPI2 引脚 (CLK-PB13, MISO-PC2, MOSI-PC3)
#define ST7789_SPI           SPI2
#define ST7789_SPI_CLK       RCC_APB1Periph_SPI2

#define ST7789_SCK_PORT      GPIOB
#define ST7789_SCK_PIN       GPIO_Pin_13
#define ST7789_SCK_CLK       RCC_AHB1Periph_GPIOB
#define ST7789_SCK_AF        GPIO_AF_SPI2
#define ST7789_SCK_SOURCE    GPIO_PinSource13

#define ST7789_MISO_PORT     GPIOC
#define ST7789_MISO_PIN      GPIO_Pin_2
#define ST7789_MISO_CLK      RCC_AHB1Periph_GPIOC
#define ST7789_MISO_AF       GPIO_AF_SPI2
#define ST7789_MISO_SOURCE   GPIO_PinSource2

#define ST7789_MOSI_PORT     GPIOC
#define ST7789_MOSI_PIN      GPIO_Pin_3
#define ST7789_MOSI_CLK      RCC_AHB1Periph_GPIOC
#define ST7789_MOSI_AF       GPIO_AF_SPI2
#define ST7789_MOSI_SOURCE   GPIO_PinSource3

/* ================== 引脚操作宏 ================== */
#define ST7789_CS(x)         ((x) ? GPIO_SetBits(ST7789_CS_PORT, ST7789_CS_PIN) : GPIO_ResetBits(ST7789_CS_PORT, ST7789_CS_PIN))
#define ST7789_RES(x)        ((x) ? GPIO_SetBits(ST7789_RES_PORT, ST7789_RES_PIN) : GPIO_ResetBits(ST7789_RES_PORT, ST7789_RES_PIN))
#define ST7789_DC(x)         ((x) ? GPIO_SetBits(ST7789_DC_PORT, ST7789_DC_PIN) : GPIO_ResetBits(ST7789_DC_PORT, ST7789_DC_PIN))
#define ST7789_BL(x)         ((x) ? GPIO_SetBits(ST7789_BL_PORT, ST7789_BL_PIN) : GPIO_ResetBits(ST7789_BL_PORT, ST7789_BL_PIN))

#endif /* __ST7789_DESC_H */
