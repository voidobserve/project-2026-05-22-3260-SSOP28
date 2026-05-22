#ifndef __ENGINE_SPEED_SCAN_H__
#define __ENGINE_SPEED_SCAN_H__

#include "include.h"   // 使用芯片官方提供的头文件
#include "my_config.h" // 包含自定义的头文件

#if ENGINE_SPEED_SCAN_ENABLE

// ======================================================
// 检测发动机转速所需的配置：
#define ENGINE_SPEED_SCAN_PIN P02 // 检测发动机转速的引脚
// 检测到 多少个脉冲 表示 发动机转过一圈
// #define ENGINE_SPEED_SCAN_PULSE_PER_TURN (60)

// 检测到一个脉冲表示发动机转过多少圈
#if 1
#define ENGINE_SPEED_SCAN_A_PULSE_PER_TURNS (1)
// #define ENGINE_SPEED_SCAN_A_PULSE_PER_TURNS (15)
// #define ENGINE_SPEED_SCAN_A_PULSE_PER_TURNS (10)
// #define ENGINE_SPEED_SCAN_A_PULSE_PER_TURNS (99)
#elif

#endif
  
// 发动机转速的更新时间，单位：ms
#define ENGINE_SPEED_SCAN_UPDATE_TIME (500)
/*
    发动机转速的超时时间，单位：ms
    如果超时时间到来，还没有检测到脉冲，认为发动机转速为0
*/
#define ENGINE_SPEED_SCAN_OVER_TIME (600)


#define AIP3368H_DISPLAY_ENGINE_SPEED_REFRESH_TIME 200

// 检测发动机转速所需的配置
// ======================================================
extern volatile bit flag_is_engine_speed_scan_over_time; // 标志位，发动机转速检测是否超时
extern volatile u32 engine_speed_scan_cnt; // 检测到的脉冲个数，在定时器中断累加
extern volatile u16 engine_speed_scan_ms;


extern void engine_speed_scan_config(void); // 发动机转速扫描的配置
extern void engine_speed_scan(void);        // 发动机转速扫描

void engine_speed_scan_timer_50us_isr(void);

void aip3368h_display_engine_speed_refresh_time_add(void);
void aip3368h_display_engine_speed_handle(void);

#endif // #if ENGINE_SPEED_SCAN_ENABLE

#endif