#include <stddef.h>
#include "botton_desc.h"
#include "botton.h"

void botton_init(botton_desc_t botton)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = botton->Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = botton->PuPd;
    GPIO_Init(botton->Port, &GPIO_InitStructure);

    SYSCFG_EXTILineConfig(botton->EXTI_PortSource, botton->EXTI_PinSource);

    EXTI_InitStructure.EXTI_Line = botton->EXTI_Line;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = botton->Trigger;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel = botton->NVIC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    botton->callback = NULL;
}

void botton_set_callback(botton_desc_t botton, void (*callback)(void* arg), void *arg)
{
    botton->callback = callback;
    botton->arg = arg;
}

void botton_irq_handler(botton_desc_t botton)
{
    if(EXTI_GetITStatus(botton->EXTI_Line) != RESET)
    {
        if (botton->callback)
        {
            botton->callback(botton->arg);
        }
        EXTI_ClearITPendingBit(botton->EXTI_Line);
    }
}
