#ifndef __I2C_DESC_H__
#define __I2C_DESC_H__

#include <stdint.h>
#include "stm32f4xx.h"

struct i2c_desc
{
    GPIO_TypeDef* SCL_Port;
    uint32_t SCL_Pin;
    
    GPIO_TypeDef* SDA_Port;
    uint32_t SDA_Pin;
};

#endif /* __I2C_DESC_H__ */ 
