#ifndef __BOTTONN_H__
#define __BOTTONN_H__

struct botton_desc;
typedef struct botton_desc *botton_desc_t;

void botton_init(botton_desc_t botton);
void botton_set_callback(botton_desc_t botton, void (*callback)(void* arg), void *arg);
void botton_irq_handler(botton_desc_t botton);

#endif /* __BOTTONN_H__ */
