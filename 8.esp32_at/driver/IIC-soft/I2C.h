#ifndef __I2C_H__
#define __I2C_H__   

#include <stdint.h>

struct i2c_desc;
typedef struct i2c_desc *i2c_handle_t;

void i2c_init(i2c_handle_t i2c);
void i2c_start(i2c_handle_t i2c);
void i2c_stop(i2c_handle_t i2c);
uint8_t i2c_wait_ack(i2c_handle_t i2c);
void i2c_ack(i2c_handle_t i2c);
void i2c_nack(i2c_handle_t i2c);
void i2c_send_byte(i2c_handle_t i2c, uint8_t byte);
uint8_t i2c_read_byte(i2c_handle_t i2c, uint8_t ack);

#endif /* __I2C_H__ */
