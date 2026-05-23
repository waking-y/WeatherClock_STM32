#ifndef __USART1_DESC_H__
#define __USART1_DESC_H__

#include <stdint.h>
#include "stm32f4xx.h"

// USART1 硬件“身份证”
struct usart1_desc
{
    USART_TypeDef* USARTx;      //  USART1
    
    GPIO_TypeDef* TX_Port;     // TX 发送引脚端口
    uint32_t       TX_Pin;      // TX 发送引脚
    uint8_t        TX_PinSource;// TX 复用引脚源 (STM32F4 特有)
    
    GPIO_TypeDef* RX_Port;     // RX 接收引脚端口
    uint32_t       RX_Pin;      // RX 接收引脚
    uint8_t        RX_PinSource;// RX 复用引脚源
    
    uint8_t        GPIO_AF;     // 复用功能映射 (如 GPIO_AF_USART1)
    uint32_t       BaudRate;    // 波特率 (如 115200)
};

#endif /* __USART1_DESC_H__ */
