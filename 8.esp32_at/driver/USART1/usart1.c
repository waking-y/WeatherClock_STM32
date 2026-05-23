#include "usart1_desc.h"
#include "usart1.h"

/*
 * Function: usart1_init
 * Description: Initialize USART1 peripheral
 */
void usart1_init(usart1_handle_t usart1)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // 1. 配置引脚复用映射 
    GPIO_PinAFConfig(usart1->TX_Port, usart1->TX_PinSource, usart1->GPIO_AF);
    GPIO_PinAFConfig(usart1->RX_Port, usart1->RX_PinSource, usart1->GPIO_AF);

    // 2. 初始化 TX 引脚为复用推挽输出
    GPIO_InitStructure.GPIO_Pin = usart1->TX_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(usart1->TX_Port, &GPIO_InitStructure);

    // 3. 初始化 RX 引脚为复用模式 
    GPIO_InitStructure.GPIO_Pin = usart1->RX_Pin;
    GPIO_Init(usart1->RX_Port, &GPIO_InitStructure);

    // 4. 配置 USART 参数
    USART_InitStructure.USART_BaudRate = usart1->BaudRate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(usart1->USARTx, &USART_InitStructure);

    // 5. 使能 USART 硬件
    USART_Cmd(usart1->USARTx, ENABLE);
}

/*
 * Function: usart1_send_byte
 * Description: Send a byte of data over USART1
 */
void usart1_send_byte(usart1_handle_t usart1, uint8_t data)
{
    USART_SendData(usart1->USARTx, data);
    // 等待发送完成标志位 (TC) 设置
    while (USART_GetFlagStatus(usart1->USARTx, USART_FLAG_TC) == RESET)
    {
    }
}
