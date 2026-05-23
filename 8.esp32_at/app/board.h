#ifndef __BOARD_H_
#define __BOARD_H_

//#include "led.h"
//#include "botton.h"
#include <stddef.h>
//#include "24c02.h"
#include "usart1.h"

//extern led_desc_t led9;
//extern led_desc_t led10;
//extern botton_desc_t key0;
//extern at24c02_handle_t at24c02;
extern usart1_handle_t usart1;

void board_lowlevel_init(void);

#endif /* __BOARD_H_ */
