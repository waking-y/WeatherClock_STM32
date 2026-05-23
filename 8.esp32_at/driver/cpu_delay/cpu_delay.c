#include <stdint.h>
#include <string.h>
#include "stm32f4xx.h"

void cpu_delay(uint32_t us)
{
    while (us >= 1000)
    {
        SysTick->LOAD = SystemCoreClock / 1000;
        SysTick->VAL = 0;
        SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);
        SysTick->CTRL = ~SysTick_CTRL_ENABLE_Msk;
        us -= 1000;
    }
    
    if (us > 0)
    {
        SysTick->LOAD = us * SystemCoreClock / 1000 / 1000;
        SysTick->VAL = 0;
        SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);
        SysTick->CTRL = ~SysTick_CTRL_ENABLE_Msk;
    }
}
