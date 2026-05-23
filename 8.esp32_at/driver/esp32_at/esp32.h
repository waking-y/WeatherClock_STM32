#ifndef __ESP32_H__
#define __ESP32_H__

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    AT_ACK_NONE = 0,
    AT_ACK_OK,
    AT_ACK_ERROR,
    AT_ACK_BUSY,
    AT_ACK_READY,
} ESP32_Ack_t;

typedef struct {
    char ssid[64];
    char bssid[18];
    int channel;
    int rssi;
    bool connected;
} ESP32_WifiInfo_t;

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t weekday;
} ESP32_DateTime_t;

typedef struct esp32_dev {
    char rx_buffer[1024];
    volatile uint16_t rx_index;
    volatile bool rx_completed;
    volatile ESP32_Ack_t current_ack;

    // 面向对象成员方法指针绑定
    bool (*init)(struct esp32_dev *self);
    bool (*wait_ready)(struct esp32_dev *self, uint32_t timeout_ms);
    bool (*write_cmd)(struct esp32_dev *self, const char *cmd, uint32_t timeout_ms);
    const char* (*get_response)(struct esp32_dev *self);
    
    bool (*wifi_init)(struct esp32_dev *self);
    bool (*connect_wifi)(struct esp32_dev *self, const char *ssid, const char *pwd, const char *mac);
    bool (*get_wifi_info)(struct esp32_dev *self, ESP32_WifiInfo_t *info);
    bool (*wifi_is_connected)(struct esp32_dev *self);
    bool (*sntp_init)(struct esp32_dev *self);
    bool (*sntp_get_time)(struct esp32_dev *self, ESP32_DateTime_t *date);
    const char* (*http_get)(struct esp32_dev *self, const char *url);
} ESP32_Dev_t;

extern ESP32_Dev_t ESP32_Device;

#endif /* __ESP32_H__ */
