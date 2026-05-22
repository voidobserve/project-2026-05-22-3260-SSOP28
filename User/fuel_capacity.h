#ifndef __FUEL_CAPACITY_H__
#define __FUEL_CAPACITY_H__

#include "include.h"   // 使用芯片官方提供的头文件
#include "my_config.h" // 包含自定义的头文件

#if FUEL_CAPACITY_SCAN_ENABLE 

// 将电压值转换为油量检测点对应的ad值，单位：mV
#define FUEL_VOLTAGE_TO_ADC_VAL(voltage) ((u16)((u32)(voltage) * 4096 / 2 / 1000))

// USER_TO_DO 如果把满油量和没有油量对应的电压值进行线性划分
/*
    1.43 ~ 0
    1430 / 5 ==

*/
// 满油量对应的电压
#define FUEL_FULL_VOLTAGE (220)
// 没有油量对应的电压
#define FUEL_EMPTY_VOLTAGE (1450)
// 油量挡位
#define FUEL_LEVEL_MAX (5)
// 线性划分的各个油量挡位对应的电压值：
enum
{
    FUEL_LEVEL_0_VOLTAGE = FUEL_EMPTY_VOLTAGE,
    FUEL_LEVEL_1_VOLTAGE = FUEL_EMPTY_VOLTAGE -
                           (FUEL_EMPTY_VOLTAGE - FUEL_FULL_VOLTAGE) *
                               1 / FUEL_LEVEL_MAX,
    FUEL_LEVEL_2_VOLTAGE = FUEL_EMPTY_VOLTAGE -
                           (FUEL_EMPTY_VOLTAGE - FUEL_FULL_VOLTAGE) *
                               2 / FUEL_LEVEL_MAX,
    FUEL_LEVEL_3_VOLTAGE = FUEL_EMPTY_VOLTAGE -
                           (FUEL_EMPTY_VOLTAGE - FUEL_FULL_VOLTAGE) *
                               3 / FUEL_LEVEL_MAX,
    FUEL_LEVEL_4_VOLTAGE = FUEL_EMPTY_VOLTAGE -
                           (FUEL_EMPTY_VOLTAGE - FUEL_FULL_VOLTAGE) *
                               4 / FUEL_LEVEL_MAX,
    FUEL_LEVEL_5_VOLTAGE = FUEL_EMPTY_VOLTAGE -
                           (FUEL_EMPTY_VOLTAGE - FUEL_FULL_VOLTAGE) *
                               5 / FUEL_LEVEL_MAX, 
}; 

// 每次计算油量的时间周期，单位：ms
#define FUEL_SCAN_TIME ((u16)100)
// 更新油量的时间，单位：ms
#define FUEL_UPDATE_TIME ((u16)5000)

// 油量检测配置
// ======================================================

extern u32 fuel_capacity_scan_cnt; // 扫描时间计数，在1ms定时器中断中累加

void fuel_capacity_scan_time_add(void);
void fuel_lev_update_time_add(void);
void fuel_capacity_adc_val_samples_update(u16 adc_val);
u16 fuel_capacity_adc_val_get(void);

void fuel_capacity_scan(void);

#endif //
#endif
