#include "photosensitive.h"
#include "my_config.h" 

static volatile u16 photosensitive_scan_period_cnt = 0;

// ===================================================================
// 滑动平均：
#define SAMPLE_COUNT 20 // 样本计数
static volatile u16 photosensitive_adc_val_samples[SAMPLE_COUNT] = {0};
static volatile u8 photosensitive_adc_val_sample_index = 0;

void photosensitive_data_init(u16 adc_val)
{
	u8 i = 0;
	for (i = 0; i < SAMPLE_COUNT; i++)
	{
		photosensitive_adc_val_samples[i] = adc_val;
	}
}

void photosensitive_data_put(u16 adc_val)
{
	static u8 is_initialized = 0;
	if (0 == is_initialized)
	{
		is_initialized = 1;
		photosensitive_data_init(adc_val);
		return;
	}

	photosensitive_adc_val_samples[photosensitive_adc_val_sample_index] =
		adc_val;
	photosensitive_adc_val_sample_index++;
	if (photosensitive_adc_val_sample_index >= SAMPLE_COUNT)
	{
		photosensitive_adc_val_sample_index = 0;
	}
}

u16 photosensitive_data_get(void)
{
	u8 i = 0;
	u32 ret = 0;
	for (i = 0; i < SAMPLE_COUNT; i++)
	{
		ret += photosensitive_adc_val_samples[i];
	}

	ret /= SAMPLE_COUNT;

	return ret;
}
// ===================================================================

void photosensitive_scan_time_add(void)
{
	if (photosensitive_scan_period_cnt < ((u16)-1))
	{
		photosensitive_scan_period_cnt++;
	}
}

void __photosensitive_handle__(void)
{
	u16 adc_val = 0;
	u16 check_point_voltage = 0; // 检测点电压
	u8 brightness;

	adc_val = photosensitive_data_get();
	check_point_voltage = PHOTOSENSITIVE_CHECK_POINT_VOLTAGE(adc_val);

#if 0
		// 测试时使用:
		{
			static u16 cnt = 0;

			check_point_voltage = 100 * cnt;

			cnt++;
			if (cnt >= 31) // 31 * 100 mV
			{
				cnt = 0;
			}
		}
#endif

#if USER_DEBUG_ENABLE
	// printf("adc_val == %u\n", adc_val);
	// printf("check point vol == %u\n",
	// 	   check_point_voltage);
#endif

	/*
		将检测点电压划分
		0.8V ~ 2.0V 映射为 20% ~ 100%
	*/
#define VOLTAGE_DEAD_ZONE 100 // 100mV 的死区电压
	if (check_point_voltage >= 800 && check_point_voltage <= 2000)
	{
		/*
			将 800mV ~ 2000mV 的电压划分为 10份
			(2000 - 800) / 10 == 1200 / 10 == 120
			每份对应的电压值 120 mV，从2000mV开始，亮度从20%开始，
			每减去120mV， 亮度加 8 % （20% ~ 100%之间，划分10份，每份8%）

			(检测点电压 - 800 mV) / 每份对应的电压值 == 检测点电压对应的亮度等级

			亮度百分比 == 检测点电压对应的亮度等级 * 8
			由于电压值越低，亮度越高,应该改成：
			亮度百分比 == 100 - 检测点电压对应的亮度等级 * 8
		*/
		brightness = 100 - ((u32)check_point_voltage - 800) * 8 / ((2000 - 800) / 10);
	}
	else if (check_point_voltage >= 2000)
	{
		// 电压大于等于2000mV，设置最小亮度 20%
		brightness = 20;
	}
	else if (check_point_voltage <= 800)
	{
		// 电压小于等于800mV，设置最大亮度 100%
		brightness = 100;
	}

#if USER_DEBUG_ENABLE
	// printf("brightness == %u\n",
	// 	   (u16)brightness);
#endif
	aip3368h_module_set_brightness(brightness);
}

void photosensitive_scan(void)
{
	// 扫描时间到来
	if (photosensitive_scan_period_cnt >= PHOTOSENSITIVE_SCAN_PERIOD)
	{
		photosensitive_scan_period_cnt = 0;
		__photosensitive_handle__();
	}
}

void photosensitive_init(void)
{
	// 直接调用一次处理函数，设置亮度
	__photosensitive_handle__();
}