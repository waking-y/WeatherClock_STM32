#include "board.h"
//#include "led_desc.h"
//#include "botton_desc.h"
//#include "I2C_desc.h"
//#include "24c02_desc.h"
#include "usart1_desc.h"
#include <stdio.h>

void board_lowlevel_init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF 
                            | RCC_AHB1Periph_GPIOE 
                            | RCC_AHB1Periph_GPIOB
                            | RCC_AHB1Periph_GPIOA, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
}

//// -------------------LED Description------------------
//static struct led_desc _led9= {GPIOF, GPIO_Pin_9, Bit_RESET, Bit_SET};
//static struct led_desc _led10 = {GPIOF, GPIO_Pin_10, Bit_RESET, Bit_SET};
//led_desc_t led9 = &_led9;
//led_desc_t led10 = &_led10;


//// -------------------Botton Description------------------
//static struct botton_desc _key0 = {
//    GPIOE, GPIO_Pin_4, GPIO_PuPd_DOWN, EXTI_Trigger_Rising,
//    EXTI_Line4, EXTI_PortSourceGPIOE, GPIO_PinSource4, EXTI4_IRQn,
//    NULL, NULL
//};
//botton_desc_t key0 = &_key0;

//// -------------------IRQ Handler------------------
//void EXTI4_IRQHandler(void) 
//{
//    botton_irq_handler(key0); // 将硬件中断信号，导向你的面向对象处理框架
//}

//// -------------------I2C Description------------------
//static struct i2c_desc _i2c1 = {
//    GPIOB, GPIO_Pin_8,//SCL
//    GPIOB, GPIO_Pin_9//SDA
//};

//// -------------------24C02 Description------------------
//static struct at24c02_desc _24c02 = {
//    &_i2c1,// I2C总线
//    0xA0// 24C02的I2C地址
//};
//at24c02_handle_t at24c02 = &_24c02;

// -------------------USART1 Description------------------
static struct usart1_desc _debug_usart1 = {
    USART1,
    GPIOA, GPIO_Pin_9,  GPIO_PinSource9,   // TX: PA9
    GPIOA, GPIO_Pin_10, GPIO_PinSource10,  // RX: PA10
    GPIO_AF_USART1,                        // F4 复用通道
    115200                                 // 波特率设置
};
usart1_handle_t usart1 = &_debug_usart1;

// -------------------Retargeting printf to USART1------------------
int fputc(int ch, FILE *f)
{
    usart1_send_byte(usart1, (uint8_t)ch);
    return ch;
}
