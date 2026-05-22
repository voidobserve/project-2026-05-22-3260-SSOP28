#ifndef __BATTERY_H__
#define __BATTERY_H__

#include "include.h"   // 使用芯片官方提供的头文件
#include "my_config.h" // 包含自定义的头文件

#if BATTERY_SCAN_ENABLE
 
// 选用检测电池电压对应的通道时，adc的参考电压，单位：mV
#define ADC_REF_OF_BATTERY (2000)
// 检测脚外部上拉电阻阻值，单位：K
#define BAT_PULL_UP_RESISTOR_VALUE ((u8)68)
// 检测脚外部下拉电阻阻值，单位：K
#define BAT_PULL_DOWN_RESISTOR_VALUE ((u8)10)

/*
	将采集到的ad值转换成实际的电池电压值
	(电池电压 - 0.2V二极管的压降) * 电阻分压系数 == 检测脚的电压值
	采集的ad值 * adc参考电压 / 4096 == 检测脚的电压值

	电池电压 = 采集的ad值 * adc参考电压 / 4096 / 电阻分压系数 + 0.2V二极管的压降

	实际计算出来比原来的电压大了100mV，这里在末尾减去100mV
*/ 
#define ADC_VAL_TO_BAT_VOLTAGE(adc_val)                                \
	((u32)adc_val * ADC_REF_OF_BATTERY *                               \
		 (BAT_PULL_UP_RESISTOR_VALUE + BAT_PULL_DOWN_RESISTOR_VALUE) / \
		 BAT_PULL_DOWN_RESISTOR_VALUE /                                \
		 4096 +                                                        \
	 (200 - 100))

// 低电压报警阈值，单位：mV
#define BAT_LOW_VOLTAGE_WARNING_THRESHOLD ((u16)10 * 1000)
// 取消低电压报警阈值，单位：mV
#define BAT_CANCEL_LOW_VOLTAGE_WARNING_THRESHOLD ((u16)10300)


// 电池电压扫描周期，单位：ms
#define BAT_SCAN_PERIOD 200

extern volatile u16 battery_scan_time_cnt; // 电池扫描时间计时

void bat_adc_val_samples_update(u16 adc_val);
// u16 bat_adc_val_get(void);

void bat_scan_time_add(void);
void battery_scan(void);

#endif // BATTERY_SCAN_ENABLE
#endif