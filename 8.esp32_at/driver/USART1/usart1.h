#ifndef __USART1_H__
#define __USART1_H__

#include <stdint.h>

struct usart1_desc;
typedef struct usart1_desc *usart1_handle_t;

void usart1_init(usart1_handle_t usart1);
void usart1_send_byte(usart1_handle_t usart1, uint8_t data);

#endif /* __USART1_H__ */
