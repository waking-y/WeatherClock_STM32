#ifndef __24C02_H__
#define __24C02_H__

#include <stdint.h>

struct at24c02_desc;
typedef struct at24c02_desc *at24c02_handle_t;

void at24c02_init(at24c02_handle_t dev);
uint8_t at24c02_read_byte(at24c02_handle_t dev, uint8_t read_addr);
void at24c02_write_byte(at24c02_handle_t dev, uint8_t write_addr, uint8_t data);
void at24c02_write_page(at24c02_handle_t dev, uint8_t write_addr, uint8_t *pBuffer, uint8_t num_to_write);
void at24c02_read_buffer(at24c02_handle_t dev, uint16_t read_addr, uint8_t *pBuffer, uint16_t num_to_read);


#endif /* __24C02_H__ */
