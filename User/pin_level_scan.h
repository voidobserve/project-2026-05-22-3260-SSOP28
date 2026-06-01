#ifndef __PIN_LEVEL_SCAN_H__
#define __PIN_LEVEL_SCAN_H__

#include "include.h"   // 使用芯片官方提供的头文件
#include "my_config.h" // 包含自定义的头文件

#if PIN_LEVEL_SCAN_ENABLE

#define PIN_DETECT_GEAR_N (P06) // N 档 检测脚
#define PIN_DETECT_GEAR_1 (P07) // 1 档 检测脚
#define PIN_DETECT_GEAR_2 (P10) // 2 档 检测脚
#define PIN_DETECT_GEAR_3 (P13) // 3 档 检测脚
#define PIN_DETECT_GEAR_4 (P14) // 4 档 检测脚
#define PIN_DETECT_GEAR_5 (P30) // 5 档 检测脚
#define PIN_DETECT_GEAR_6 (P27) // 6 档 检测脚

#define PIN_DETECT_BREAKDOWN (P26)	// 检测故障的引脚
#define PIN_DETECT_LFFT_TURN (P25)	// 左转向 检测脚
#define PIN_DETECT_RIGHT_TURN (P22) // 右转向 检测脚
#define PIN_DETECT_LOW_BEAM (P23)	// 小灯（近光灯）检测脚
#define PIN_DETECT_HIGH_BEAM (P21)	// 大灯（远光灯）检测脚

extern u16 pin_level_scan_time_cnt;

void pin_level_scan_config(void); // 扫描引脚的配置（初始化）
void pin_level_scan(void);		  // 扫描引脚的电平
#endif

#endif
