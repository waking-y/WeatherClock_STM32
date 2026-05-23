#ifndef __WEAHTER_H__
#define __WEAHTER_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    char city[32];
    char loaction[128]; // 对齐您上传的代码中带“oa”微调错拼的 loaction，保障业务编译兼容性
    char weather[16];   // 气象文本
    int weather_code;   // 气象标识代码
    float temperature;  // 浮点温度
} weather_info_t;

bool parse_seniverse_response(const char *response, weather_info_t *info);

#endif /* __WEAHTER_H__ */
