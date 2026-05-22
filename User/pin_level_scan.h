#ifndef __PIN_LEVEL_SCAN_H__
#define __PIN_LEVEL_SCAN_H__

#include "include.h"   // 使用芯片官方提供的头文件
#include "my_config.h" // 包含自定义的头文件

#if PIN_LEVEL_SCAN_ENABLE
 

#define PIN_DETECT_BREAKDOWN (P20) // 检测故障的引脚
  
 

extern u16 pin_level_scan_time_cnt;

void pin_level_scan_config(void); // 扫描引脚的配置（初始化）
void pin_level_scan(void);        // 扫描引脚的电平
#endif                             

#endif
