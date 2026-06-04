#ifndef __PHOTOSENSITIVE_H__
#define __PHOTOSENSITIVE_H__

#include "include.h"

// 扫描周期 (单位：ms)
#define PHOTOSENSITIVE_SCAN_PERIOD 2000

// ADC相关参数 (检测使用内部 3.0V 参考电压)
#define PHOTOSENSITIVE_ADC_REF_VOLTAGE 3000 // 内部参考电压 3.0 V

// 计算ADC值对应的电压 (单位：mV)
// ADC值 -> 实际电压 = ADC值 * (参考电压/4096) * 1000
#define PHOTOSENSITIVE_CHECK_POINT_VOLTAGE(adc_val) \
	((u32)(adc_val) * PHOTOSENSITIVE_ADC_REF_VOLTAGE / 4096)

void photosensitive_data_init(u16 adc_val);
void photosensitive_data_put(u16 adc_val);
u16 photosensitive_data_get(void);

void photosensitive_scan_time_add(void);
void photosensitive_scan(void);
void photosensitive_init(void);

#endif