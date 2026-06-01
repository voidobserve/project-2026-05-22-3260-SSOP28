// my_config.h -- 自定义的配置文件
#ifndef __MY_CONFIG_H__
#define __MY_CONFIG_H__

#include "include.h" // 使用芯片官方提供的头文件
// #include "typedef_struct.h" // 包含结构体类型和别名的定义

#define ARRAY_SIZE(arry) (sizeof(arry) / sizeof(arry[0]))

#define USER_DEBUG_ENABLE 1
#if USER_DEBUG_ENABLE
#define DEBUG_PIN P04
#endif

#define TOUCH_KEY_ENABLE 0 // 是否使能触摸按键检测功能
#define AD_KEY_ENABLE 0	   // 是否使能ad按键检测功能
#define IO_KEY_ENABLE 1
#define SPEED_SCAN_ENABLE 1			// 是否使能速度检测功能
#define ENGINE_SPEED_SCAN_ENABLE 1	// 是否使能发动机转速检测功能
#define BATTERY_SCAN_ENABLE 1		// 是否使能电池检测功能
#define FUEL_CAPACITY_SCAN_ENABLE 1 // 是否使能油量检测功能

#define PIN_LEVEL_SCAN_ENABLE 1 // 是否使能引脚电平检测功能

#define TEMP_OF_WATER_SCAN_ENABLE 0 // 是否使能水温检测

#define USE_INTERNAL_FLASH_SAVE_DATA 1

#define KEY_UP_VOL_UP ((u16)209)
#define KEY_DOWN_VOL_DOWN ((u16)210)
#define KEY_LEFT_PRE ((u16)207)
#define KEY_RIGHT_NEXT ((u16)208)
#define KEY_ENTER_MENU ((u16)127)

// ======================================================
// 水温检测配置:
#if TEMP_OF_WATER_SCAN_ENABLE
// 发送水温报警/解除水温报警的时间:(单位：ms，每隔xxms发送一次当前水温报警的状态)
#define TEMP_OF_WATER_UPDATE_TIME_MS (1000)

// 水温检测的累计时间(单位：ms)
#define TEMP_OF_WATER_ACCUMULATE_TIEM_MS (10000) // 10s
// 水温检测对应的报警ad值，满足该值且超过累计时间时，触发报警
#define TEMP_OF_WATER_WARNING_AD_VAL (3015)
// 水温检测对应的解除报警ad值，满足该值且超过累计时间时，解除报警
#define TEMP_OF_WATER_CANCEL_WARNING_AD_VAL (2048)

// 注意:水温报警和解除报警对应的ad值不能一致，差值也不能过小
#endif

// 水温检测配置
// ======================================================

// 里程的配置:
// 大小里程的更新时间(单位:ms):
#define MILEAGE_UPDATE_TIME_MS (1000)

// 主函数完成一次循环所需的时间，单位：ms (0--说明每次调用该函数的时间很短，可以忽略不计)(注意不能大于变量类型的大小)
// 功能全部开放后，每一轮的时间可能都不一样，不在时间要求高的场合使用(目前最短的一个周期是8ms)
// #define ONE_CYCLE_TIME_MS (8)

#include <stdio.h>	//
#include <string.h> // memset() 
// #include "tk_set.h" // 包含触摸按键的初始化接口
 
#include "key_driver.h"

#if USER_DEBUG_ENABLE
#include "uart0.h" // 测试时使用
#endif
#include "tmr1.h"			   // 用于 扫描发动机转速、扫描时速、定时将里程写入flash 的定时器
#include "tmr2.h"			   // 用于定时扫描脉冲个数(时速、发动机转速)

#include "pin_level_scan.h"	   // 扫描引脚电平状态
#include "adc.h"			   // adc
#include "speed_scan.h"		   // 速度（时速）扫描
#include "engine_speed_scan.h" // 发动机转速扫描
#include "mileage.h"		   // 里程表的相关接口
// #include "ad_key.h"			   // 检测触摸IC发送过来的ad信号
#include "fuel_capacity.h" // 油量检测
#include "battery.h"	   // 电池电量检测
// #include "touch_key.h"		   // 触摸按键
#include "io_key.h"

#include "user_flash.h"

#include "aip3368.h"
#include "aip3368h_display.h"

#include "instrument.h"		   // 存放各个功能的状态信息

#endif // end file
