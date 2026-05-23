#ifndef __24C02_DESC_H__
#define __24C02_DESC_H__

#include <stdint.h>
#include "I2C.h"

struct at24c02_desc
{
    i2c_handle_t i2c_bus;
    uint8_t device_addr;
};

#endif /* __24C02_DESC_H__ */
