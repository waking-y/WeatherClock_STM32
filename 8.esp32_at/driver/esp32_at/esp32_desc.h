#ifndef __ESP32_DESC_H__
#define __ESP32_DESC_H__

#include "stm32f4xx.h"

// ============================================================================
// ESP32-C3 串口通讯硬件描述符 (选用外设 USART3)
// ============================================================================
#define ESP32_USART                  USART3
#define ESP32_USART_CLK              RCC_APB1Periph_USART3
#define ESP32_USART_AF               GPIO_AF_USART3

#define ESP32_GPIO_CLK               RCC_AHB1Periph_GPIOB

// PB10 -> TXD (发送引脚)
#define ESP32_TX_PORT                GPIOB
#define ESP32_TX_PIN                 GPIO_Pin_10
#define ESP32_TX_SOURCE              GPIO_PinSource10

// PB11 -> RXD (接收引脚)
#define ESP32_RX_PORT                GPIOB
#define ESP32_RX_PIN                 GPIO_Pin_11
#define ESP32_RX_SOURCE              GPIO_PinSource11

// 中断服务程序映射管理
#define ESP32_USART_IRQn             USART3_IRQn
#define ESP32_USART_IRQHandler       USART3_IRQHandler

// 内部环形解析缓冲区尺寸限制 (足以容纳大尺寸 HTTP Client 负载)
#define ESP32_RX_BUF_SIZE            1024

#endif /* __ESP32_DESC_H__ */
