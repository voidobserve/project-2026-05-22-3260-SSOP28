#ifndef __SPEED_SCAN_H__
#define __SPEED_SCAN_H__

#include "include.h"   // 使用芯片官方提供的头文件
#include "my_config.h" // 包含自定义的头文件

#if SPEED_SCAN_ENABLE
// ======================================================
// 检测时速所需的配置：
#define SPEED_SCAN_PIN P15 // 检测时速的引脚
// 检测到 多少个脉冲 表示 车轮走过一圈（仅供计算时使用，并不代表真实的车轮数据）
#define SPEED_SCAN_PULSE_PER_TURN (3)
// 车轮的一圈对应多少毫米（仅供计算时使用，并不代表真实的车轮数据）
#define SPEED_SCAN_MM_PER_TURN (1070) // 一圈 xx 毫米
  
/*
    时速扫描的超时时间，单位：ms
    如果超过这个时间还没有检测到脉冲个数，则认为时速为0

    至少要大于500ms，才能检测到1Hz的信号
*/
#define SPEED_SCAN_OVER_TIME (600) // == 0 , 每600ms发送一次
// 时速的更新时间，单位：ms
#define SPEED_SCAN_UPDATE_TIME (500)
// #define SPEED_SCAN_UPDATE_TIME (400) // 时间太短，时速会经常跳动（提供1Hz信号，会在0km/h、3km/h这两个值来回切换）

// 更新时速显示的时间周期，单位：ms
#define AIP3368H_DISPLAY_SPEED_REFRESH_TIME (75)

void speed_scan_timer_50us_isr(void);

void speed_scan_pulse_50us_isr(void);
// void speed_scan_time_add(void);

void speed_scan_config(void);
void speed_scan(void);


void aip3368h_display_speed_refresh_time_add(void);
void aip3368h_display_speed_handle(void);

#endif // SPEED_SCAN_ENABLE
#endif
