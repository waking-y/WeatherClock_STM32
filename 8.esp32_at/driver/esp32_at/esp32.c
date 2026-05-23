#include "esp32.h"
#include "esp32_desc.h"
#include "cpu_delay.h"
#include <string.h>
#include <stdio.h>

// 静态匹配列表，用于在串口中断中异步抓取状态机的关键ACK握手
typedef struct {
    ESP32_Ack_t ack;
    const char *string;
} ESP32_AckMatch_t;

static const ESP32_AckMatch_t ack_matches[] = {
    {AT_ACK_OK, "OK\r\n"},
    {AT_ACK_ERROR, "ERROR\r\n"},
    {AT_ACK_BUSY, "busy p...\r\n"},
    {AT_ACK_READY, "ready\r\n"},
};

static void esp32_usart_init(void);
static uint8_t weekday_str_to_num(const char *str);
static uint8_t month_str_to_num(const char *str);

// 面向对象具体实现
static bool esp32_init_impl(ESP32_Dev_t *self);
static bool esp32_wait_ready_impl(ESP32_Dev_t *self, uint32_t timeout_ms);
static bool esp32_write_cmd_impl(ESP32_Dev_t *self, const char *cmd, uint32_t timeout_ms);
static const char* esp32_get_response_impl(ESP32_Dev_t *self);
static bool esp32_wifi_init_impl(ESP32_Dev_t *self);
static bool esp32_connect_wifi_impl(ESP32_Dev_t *self, const char *ssid, const char *pwd, const char *mac);
static bool esp32_get_wifi_info_impl(ESP32_Dev_t *self, ESP32_WifiInfo_t *info);
static bool esp32_wifi_is_connected_impl(ESP32_Dev_t *self);
static bool esp32_sntp_init_impl(ESP32_Dev_t *self);
static bool esp32_sntp_get_time_impl(ESP32_Dev_t *self, ESP32_DateTime_t *date);
static const char* esp32_http_get_impl(ESP32_Dev_t *self, const char *url);

// 绑定全局单例实例的方法表
ESP32_Dev_t ESP32_Device = {
    .rx_index = 0,
    .rx_completed = false,
    .current_ack = AT_ACK_NONE,
    .init = esp32_init_impl,
    .wait_ready = esp32_wait_ready_impl,
    .write_cmd = esp32_write_cmd_impl,
    .get_response = esp32_get_response_impl,
    .wifi_init = esp32_wifi_init_impl,
    .connect_wifi = esp32_connect_wifi_impl,
    .get_wifi_info = esp32_get_wifi_info_impl,
    .wifi_is_connected = esp32_wifi_is_connected_impl,
    .sntp_init = esp32_sntp_init_impl,
    .sntp_get_time = esp32_sntp_get_time_impl,
    .http_get = esp32_http_get_impl
};

static bool esp32_init_impl(ESP32_Dev_t *self)
{
    self->rx_index = 0;
    self->rx_completed = false;
    self->current_ack = AT_ACK_NONE;
    memset(self->rx_buffer, 0, ESP32_RX_BUF_SIZE);
    
    // 初始化串口硬件底层及中断管理器
    esp32_usart_init();
    
    // AT 握手信号
    self->write_cmd(self, "AT", 200);
    if (!self->write_cmd(self, "AT", 500)) {
        return false;
    }
    
    // 执行出厂设定恢复 (重启模组)
    if (!self->write_cmd(self, "AT+RESTORE", 2000)) {
        return false;
    }
    
    // 等待模组发出 "ready" 标识
    return self->wait_ready(self, 5000);
}

static bool esp32_wait_ready_impl(ESP32_Dev_t *self, uint32_t timeout_ms)
{
    uint32_t elapsed = 0;
    while (elapsed < timeout_ms) {
        if (self->current_ack == AT_ACK_READY) {
            return true;
        }
        cpu_delay(1000); // 延时 1ms
        elapsed++;
    }
    return false;
}

static bool esp32_write_cmd_impl(ESP32_Dev_t *self, const char *cmd, uint32_t timeout_ms)
{
    self->rx_index = 0;
    self->rx_completed = false;
    self->current_ack = AT_ACK_NONE;
    memset(self->rx_buffer, 0, ESP32_RX_BUF_SIZE);

    // 阻塞式纯硬件串口写字符流
    const char *p = cmd;
    while (*p) {
        USART_SendData(ESP32_USART, *p++);
        while (USART_GetFlagStatus(ESP32_USART, USART_FLAG_TXE) == RESET);
    }
    // 补充输入标准的结束换行符
    USART_SendData(ESP32_USART, '\r'); while (USART_GetFlagStatus(ESP32_USART, USART_FLAG_TXE) == RESET);
    USART_SendData(ESP32_USART, '\n'); while (USART_GetFlagStatus(ESP32_USART, USART_FLAG_TXE) == RESET);

    // 采用非阻塞的中断方式捕获异步反馈 ACK 状态
    uint32_t elapsed = 0;
    while (elapsed < timeout_ms) {
        if (self->current_ack == AT_ACK_OK) {
            return true;
        }
        if (self->current_ack == AT_ACK_ERROR || self->current_ack == AT_ACK_BUSY) {
            return false;
        }
        cpu_delay(1000); // 延时 1ms
        elapsed++;
    }
    return false;
}

static const char* esp32_get_response_impl(ESP32_Dev_t *self)
{
    return (const char*)self->rx_buffer;
}

static bool esp32_wifi_init_impl(ESP32_Dev_t *self)
{
    // 配置为 Station 模式
    return self->write_cmd(self, "AT+CWMODE=1", 2000);
}

static bool esp32_connect_wifi_impl(ESP32_Dev_t *self, const char *ssid, const char *pwd, const char *mac)
{
    static char cmd_buf[256];
    if (mac && strlen(mac) > 0) {
        snprintf(cmd_buf, sizeof(cmd_buf), "AT+CWJAP=\"%s\",\"%s\",\"%s\"", ssid, pwd, mac);
    } else {
        snprintf(cmd_buf, sizeof(cmd_buf), "AT+CWJAP=\"%s\",\"%s\"", ssid, pwd);
    }
    return self->write_cmd(self, cmd_buf, 15000);
}

static bool esp32_get_wifi_info_impl(ESP32_Dev_t *self, ESP32_WifiInfo_t *info)
{
    memset(info, 0, sizeof(ESP32_WifiInfo_t));
    
    if (!self->write_cmd(self, "AT+CWJAP?", 3000)) {
        info->connected = false;
        return false;
    }
    
    const char *resp = self->get_response(self);
    resp = strstr(resp, "+CWJAP:");
    if (!resp) {
        info->connected = false;
        return true; // 未报错，仅说明未连上热点
    }
    
    // 解析 SSID 标识及信号强度 RSSI
    if (sscanf(resp, "+CWJAP:\"%63[^\"]\",\"%17[^\"]\",%d,%d", 
               info->ssid, info->bssid, &info->channel, &info->rssi) == 4) {
        info->connected = true;
        return true;
    }
    return false;
}

static bool esp32_wifi_is_connected_impl(ESP32_Dev_t *self)
{
    ESP32_WifiInfo_t info;
    if (self->get_wifi_info(self, &info)) {
        return info.connected;
    }
    return false;
}

static bool esp32_sntp_init_impl(ESP32_Dev_t *self)
{
    // 配置高精度SNTP时区（东八区）和稳定的国内服务器
    return self->write_cmd(self, "AT+CIPSNTPCFG=1,8,\"ntp.aliyun.com\",\"cn.ntp.org.cn\"", 3000);
}

static bool esp32_sntp_get_time_impl(ESP32_Dev_t *self, ESP32_DateTime_t *date)
{
    if (!self->write_cmd(self, "AT+CIPSNTPTIME?", 3000)) return false;
    
    const char *resp = self->get_response(self);
    resp = strstr(resp, "+CIPSNTPTIME:");
    if (!resp) return false;

    char weekday_str[8] = {0};
    char month_str[8] = {0};
    
    if (sscanf(resp, "+CIPSNTPTIME:%3s %3s %hhu %hhu:%hhu:%hhu %hu", 
               weekday_str, month_str, &date->day, &date->hour, 
               &date->minute, &date->second, &date->year) != 7) {
        return false;
    }
    
    date->weekday = weekday_str_to_num(weekday_str);
    date->month = month_str_to_num(month_str);
    return true;
}

static const char* esp32_http_get_impl(ESP32_Dev_t *self, const char *url)
{
    // 为避免 memset 破坏输入参数或本身的数据缓存，独立采用静态发送区
    static char cmd_buf[512];
    // 使用 application/json 及 HTTPS（传输安全配置选择2）
    snprintf(cmd_buf, sizeof(cmd_buf), "AT+HTTPCLIENT=2,1,\"%s\",,,2", url);
    if (self->write_cmd(self, cmd_buf, 10000)) {
        return self->get_response(self);
    }
    return NULL;
}

static void esp32_usart_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(ESP32_USART_CLK, ENABLE);
    RCC_AHB1PeriphClockCmd(ESP32_GPIO_CLK, ENABLE);

    // 引脚映射 AF
    GPIO_PinAFConfig(ESP32_TX_PORT, ESP32_TX_SOURCE, ESP32_USART_AF);
    GPIO_PinAFConfig(ESP32_RX_PORT, ESP32_RX_SOURCE, ESP32_USART_AF);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = ESP32_TX_PIN; GPIO_Init(ESP32_TX_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = ESP32_RX_PIN; GPIO_Init(ESP32_RX_PORT, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(ESP32_USART, &USART_InitStructure);

    USART_ITConfig(ESP32_USART, USART_IT_RXNE, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = ESP32_USART_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(ESP32_USART, ENABLE);
}

// 串口中断回调解析入口
void ESP32_USART_IRQHandler(void)
{
    if (USART_GetITStatus(ESP32_USART, USART_IT_RXNE) != RESET) {
        char data = (char)USART_ReceiveData(ESP32_USART);

        if (ESP32_Device.rx_index < ESP32_RX_BUF_SIZE - 1) {
            ESP32_Device.rx_buffer[ESP32_Device.rx_index++] = data;
            ESP32_Device.rx_buffer[ESP32_Device.rx_index] = '\0'; 

            for (uint8_t i = 0; i < sizeof(ack_matches)/sizeof(ack_matches[0]); i++) {
                uint16_t len = strlen(ack_matches[i].string);
                if (ESP32_Device.rx_index >= len) {
                    if (strcmp(&ESP32_Device.rx_buffer[ESP32_Device.rx_index - len], ack_matches[i].string) == 0) {
                        ESP32_Device.current_ack = ack_matches[i].ack;
                        ESP32_Device.rx_completed = true;
                        break;
                    }
                }
            }
        } else {
            ESP32_Device.rx_index = 0; 
        }
    }
}

static uint8_t weekday_str_to_num(const char *str)
{
    if (strcmp(str, "Mon") == 0) return 1;
    if (strcmp(str, "Tue") == 0) return 2;
    if (strcmp(str, "Wed") == 0) return 3;
    if (strcmp(str, "Thu") == 0) return 4;
    if (strcmp(str, "Fri") == 0) return 5;
    if (strcmp(str, "Sat") == 0) return 6;
    if (strcmp(str, "Sun") == 0) return 7; // 对齐主业务循环判断
    return 0;
}

static uint8_t month_str_to_num(const char *str)
{
    const char *months[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    for (uint8_t i = 0; i < 12; i++) {
        if (strcmp(str, months[i]) == 0) return i + 1;
    }
    return 1;
}
