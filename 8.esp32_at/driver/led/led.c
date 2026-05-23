#include <stdbool.h>
#include <stdint.h>
#include "led_desc.h"
#include "led.h"

void led_init(led_desc_t led)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // Configure the GPIO pin for the LED
    GPIO_InitStruct.GPIO_Pin = led->Pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(led->Port, &GPIO_InitStruct);

    GPIO_WriteBit(led->Port, led->Pin, led->OffBit); // Ensure LED is off initially
}

void led_on(led_desc_t led)
{
    GPIO_WriteBit(led->Port, led->Pin, led->OnBit);
}

void led_off(led_desc_t led)
{
    GPIO_WriteBit(led->Port, led->Pin, led->OffBit);
}

void led_toggle(led_desc_t led)
{
    GPIO_ToggleBits(led->Port, led->Pin);
}
