#ifndef __BOTTON_DESC_H__
#define __BOTTON_DESC_H__

#include <stdint.h>
#include "stm32f4xx.h"

struct botton_desc
{
    GPIO_TypeDef *Port;
    uint32_t Pin;
    GPIOPuPd_TypeDef PuPd;
    EXTITrigger_TypeDef Trigger;

    uint32_t EXTI_Line;
    uint8_t EXTI_PortSource;
    uint8_t EXTI_PinSource;
    uint8_t NVIC_IRQn;

    void (*callback)(void* arg);
    void *arg;
};

#endif /* __BOTTON_DESC_H__ */
