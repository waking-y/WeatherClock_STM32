#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "stm32f4xx.h"
#include "board.h"
#include "cpu_delay.h"
#include "st7789.h"
#include "esp32.h"
#include "esp32_desc.h"
#include "weather.h"

#define delay_us(us)      cpu_delay(us)       // 微秒
#define delay_ms(ms)      cpu_delay((ms)*1000) // 毫秒 → 转成微秒再调用cpu_delay

// Wifi 配置
#define WIFI_SSID           "MyWifi"     // WiFi 账户
#define WIFI_PASSWORD       "12345678"   // WiFi 密码

// 心知天气 API URL
static const char *weather_url = 
    "https://api.seniverse.com/v3/weather/now.json?key=SNoBTl1eIOboddVSl&location=Nanyang&language=en&unit=c";

static void update_status_bar(const char *status, uint16_t color)
{
    ST7789_FillRect(10, 280, 220, 20, BLACK);
    ST7789_ShowString(10, 280, status, color, BLACK, 16, 1);
}

static void draw_clock_ui(ESP32_DateTime_t *date)
{
    char text_buf[32];
    
    snprintf(text_buf, sizeof(text_buf), "%04d-%02d-%02d", date->year, date->month, date->day);
    ST7789_FillRect(30, 40, 180, 20, BLACK);
    ST7789_ShowString(30, 40, text_buf, CYAN, BLACK, 16, 1);
    
    snprintf(text_buf, sizeof(text_buf), "%02d:%02d:%02d", date->hour, date->minute, date->second);
    ST7789_FillRect(20, 70, 200, 40, BLACK);
    ST7789_ShowString(20, 70, text_buf, WHITE, BLACK, 32, 1);
    
    const char *weekdays[] = {"Unknown", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    const char *wk = (date->weekday >= 1 && date->weekday <= 7) ? weekdays[date->weekday] : weekdays[0];
    ST7789_FillRect(50, 120, 140, 20, BLACK);
    ST7789_ShowString(50, 120, wk, YELLOW, BLACK, 16, 1);
}

static void draw_weather_ui(weather_info_t *weather)
{
    char text_buf[64];
    
    ST7789_FillRect(10, 160, 220, 2, GRAY);
    
    snprintf(text_buf, sizeof(text_buf), "City: %s", weather->city);
    ST7789_FillRect(10, 180, 220, 20, BLACK);
    ST7789_ShowString(10, 180, text_buf, GRED, BLACK, 16, 1);
    
    snprintf(text_buf, sizeof(text_buf), "Weather: %s", weather->weather);
    ST7789_FillRect(10, 210, 220, 20, BLACK);
    ST7789_ShowString(10, 210, text_buf, GBLUE, BLACK, 16, 1);
    
    snprintf(text_buf, sizeof(text_buf), "Temp: %.1f C", weather->temperature);
    ST7789_FillRect(10, 240, 220, 20, BLACK);
    ST7789_ShowString(10, 240, text_buf, MAGENTA, BLACK, 16, 1);
}

int main(void)
{
    ESP32_DateTime_t current_date = {0};
    weather_info_t current_weather = {0};
    bool sntp_synced = false;
    const char *http_response = NULL;
    uint32_t seconds_elapsed = 0;
    uint8_t i = 0;

    // 3.1 系统底层初始化
    board_lowlevel_init();
    
    // 3.2 调试串口初始化
    usart1_init(usart1);
    printf("\r\n[SYS] Smart Weather Station Starting up...\r\n");
    
    // 3.3 LCD 初始化
    ST7789_Init();
    delay_ms(5000);
    ST7789_Clear(BLACK);
    delay_ms(3000);
    
    ST7789_ShowString(40, 10, "Smart Clock Station", GREEN, BLACK, 16, 1);
    update_status_bar("Booting Wifi...", YELLOW);
    delay_ms(3000); // 等待模组稳定
    
    // 3.4 ESP32 初始化
    if (!ESP32_Device.init(&ESP32_Device)) {
        printf("[ERR] ESP32 AT Interface Initialize Failed!\r\n");
        update_status_bar("ESP32 Init Error", RED);
        goto err_halt;
    }
    printf("[SYS] ESP32 AT Client Init Success!\r\n");
    
    delay_ms(1000); // 等待模组稳定
    // 3.5 WiFi 配置与连接
    if (!ESP32_Device.wifi_init(&ESP32_Device)) {
        printf("[ERR] Config Station Mode Failed!\r\n");
        update_status_bar("WiFi Mode Error", RED);
        goto err_halt;
    }
    
    printf("[SYS] Connecting to Access Point [%s]...\r\n", WIFI_SSID);
    if (!ESP32_Device.connect_wifi(&ESP32_Device, WIFI_SSID, WIFI_PASSWORD, NULL)) {
        printf("[ERR] Connect to AP %s Failed!\r\n", WIFI_SSID);
        update_status_bar("WiFi Conn Error", RED);
        goto err_halt;
    }
    printf("[SYS] WiFi Connect Success!\r\n");
    update_status_bar("WiFi Connected", GREEN);
    
    // 3.6 SNTP 校时
    printf("[SYS] Starting SNTP Client...\r\n");
    if (!ESP32_Device.sntp_init(&ESP32_Device)) {
        printf("[ERR] SNTP Client Config Failed!\r\n");
        update_status_bar("SNTP Init Error", RED);
        goto err_halt;
    }
    
    printf("[SYS] Syncing Network Time...\r\n");
    update_status_bar("Time Syncing...", YELLOW);
    
    sntp_synced = false;
    for (i = 0; i < 5; i++) {
        delay_ms(2000); // 等待2秒后重试
        if (ESP32_Device.sntp_get_time(&ESP32_Device, &current_date)) {
            if (current_date.year >= 2026) {
                sntp_synced = true;
                break;
            }
        }
    }
    
    if (!sntp_synced) {
        printf("[WARN] Cloud Time Sync Timeout! Utilizing Local Backup...\r\n");
        update_status_bar("Time Sync Failed", RED);
        current_date.year = 2026;
        current_date.month = 5;
        current_date.day = 22;
        current_date.hour = 20;
        current_date.minute = 14;
        current_date.second = 0;
        current_date.weekday = 5;
    } else {
        printf("[SYS] Cloud Time Sync Completed!\r\n");
        update_status_bar("Time Sync Success", GREEN);
    }
    
    // 3.7 初次天气获取
    printf("[SYS] Requesting Seniverse Weather Data...\r\n");
    update_status_bar("Fetching Weather...", YELLOW);
    
    http_response = ESP32_Device.http_get(&ESP32_Device, weather_url);
    if (http_response && parse_seniverse_response(http_response, &current_weather)) {
        printf("[WEATHER] %s, %s, Temp: %.1f C\r\n", 
               current_weather.city, current_weather.weather, current_weather.temperature);
        draw_weather_ui(&current_weather);
        update_status_bar("System OK", GREEN);
    } else {
        printf("[WARN] Initial Weather Fetch Failed!\r\n");
        update_status_bar("Weather Fetch Error", RED);
    }
    
    seconds_elapsed = 0;

    // ============================================================================
    // 4. 主循环
    // ============================================================================
    while (1)
    {
        delay_ms(1000); // 每秒更新一次
        seconds_elapsed++;
        
        // 本地时间自增
        current_date.second++;
        if (current_date.second >= 60) {
            current_date.second = 0;
            current_date.minute++;
            if (current_date.minute >= 60) {
                current_date.minute = 0;
                current_date.hour++;
                if (current_date.hour >= 24) {
                    current_date.hour = 0;
                    current_date.day++;
                }
            }
        }
        
        // 刷新界面
        draw_clock_ui(&current_date);
        printf("[CLOCK] %04d-%02d-%02d %02d:%02d:%02d\r\n", 
               current_date.year, current_date.month, current_date.day, 
               current_date.hour, current_date.minute, current_date.second);
        
        // 1小时同步时间
        if (seconds_elapsed % 3600 == 0) {
            printf("[SYS] Periodical SNTP Time Calibrating...\r\n");
            ESP32_DateTime_t temp_date;
            if (ESP32_Device.sntp_get_time(&ESP32_Device, &temp_date)) {
                if (temp_date.year >= 2026) {
                    current_date = temp_date;
                    printf("[SYS] SNTP Calibration Completed.\r\n");
                }
            }
        }
        
        // 15分钟更新天气
        if (seconds_elapsed % 900 == 0) {
            printf("[SYS] Requesting Periodical Weather Refresh...\r\n");
            update_status_bar("Updating Weather...", YELLOW);
            
            http_response = ESP32_Device.http_get(&ESP32_Device, weather_url);
            if (http_response && parse_seniverse_response(http_response, &current_weather)) {
                printf("[WEATHER] Updated: %s, %s, Temp: %.1f C\r\n", 
                       current_weather.city, current_weather.weather, current_weather.temperature);
                draw_weather_ui(&current_weather);
                update_status_bar("System OK", GREEN);
            } else {
                printf("[WARN] Periodical Weather Sync Failed!\r\n");
                update_status_bar("Weather Sync Fail", RED);
            }
        }
    }

err_halt:
    while (1) {
        printf("[SYS] System Halted in Error State.\r\n");
        delay_ms(2000); // 等待2秒后重试
    }
}
