#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "weather.h"

static const char* json_get_val_str(const char *json, const char *key, char *out, size_t max_len)
{
    const char *p = strstr(json, key);
    if (!p) return NULL;
    p += strlen(key);
    while (*p == ' ' || *p == ':' || *p == '"') {
        p++;
    }
    size_t i = 0;
    while (*p && *p != '"' && *p != ',' && *p != '}' && i < max_len - 1) {
        out[i++] = *p++;
    }
    out[i] = '\0';
    return p;
}

static const char* json_get_val_int(const char *json, const char *key, int *out)
{
    const char *p = strstr(json, key);
    if (!p) return NULL;
    p += strlen(key);
    while (*p == ' ' || *p == ':' || *p == '"') {
        p++;
    }
    *out = atoi(p);
    return p;
}

static const char* json_get_val_float(const char *json, const char *key, float *out)
{
    const char *p = strstr(json, key);
    if (!p) return NULL;
    p += strlen(key);
    while (*p == ' ' || *p == ':' || *p == '"') {
        p++;
    }
    *out = (float)atof(p);
    return p;
}

bool parse_seniverse_response(const char *response, weather_info_t *info)
{
    if (response == NULL || info == NULL) {
        return false;
    }

    memset(info, 0, sizeof(weather_info_t));

    // 搜索结果集
    const char *results = strstr(response, "\"results\":");
    if (results == NULL) {
        return false;
    }

    // 1. 安全定位并解析 Location 节点
    const char *location = strstr(results, "\"location\":");
    if (location) {
        json_get_val_str(location, "\"name\"", info->city, sizeof(info->city));
        json_get_val_str(location, "\"path\"", info->loaction, sizeof(info->loaction));
    }

    // 2. 安全定位并解析 Now 节点
    const char *now = strstr(results, "\"now\":");
    if (now) {
        json_get_val_str(now, "\"text\"", info->weather, sizeof(info->weather));
        json_get_val_int(now, "\"code\"", &info->weather_code);
        json_get_val_float(now, "\"temperature\"", &info->temperature);
        return true;
    }

    return false;
}
