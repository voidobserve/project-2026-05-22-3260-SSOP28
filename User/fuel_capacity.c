#include "fuel_capacity.h"

#if FUEL_CAPACITY_SCAN_ENABLE

static volatile u16 fuel_capacity_scan_time_cnt = 0;
static volatile u16 fuel_lev_update_time_cnt = 0;

// 滑动平均：
#define SAMPLE_COUNT 20 // 样本计数
static volatile u16 fuel_capacity_adc_val_samples[SAMPLE_COUNT] = {0};
static volatile u8 fuel_capacity_adc_val_sample_index = 0;

static void __fuel_capacity_adc_val_samples_init__(u16 adc_val)
{
    u8 i;
    for (i = 0; i < SAMPLE_COUNT; i++)
    {
        fuel_capacity_adc_val_samples[i] = adc_val;
    }
}

// 将数据放入油量检测的滑动平均数组
void fuel_capacity_adc_val_samples_update(u16 adc_val)
{
    static u8 is_initialized = 0;
    if (0 == is_initialized)
    {
        is_initialized = 1;
        __fuel_capacity_adc_val_samples_init__(adc_val);
        return; // 初始化数组之后，直接退出，下一次得到新数据才执行下面的操作
    }

    fuel_capacity_adc_val_samples[fuel_capacity_adc_val_sample_index] = adc_val;
    fuel_capacity_adc_val_sample_index++;
    if (fuel_capacity_adc_val_sample_index >= SAMPLE_COUNT)
    {
        fuel_capacity_adc_val_sample_index = 0;
    }
}

// 从油量检测的滑动平均数组中获取数据
u16 fuel_capacity_adc_val_get(void)
{
    u8 i;
    u32 sum = 0;
    for (i = 0; i < SAMPLE_COUNT; i++)
    {
        sum += fuel_capacity_adc_val_samples[i];
    }

    return (sum / SAMPLE_COUNT);
}

void fuel_capacity_scan_time_add(void)
{
    if (fuel_capacity_scan_time_cnt < ((u16)-1))
    {
        fuel_capacity_scan_time_cnt++;
    }
}

void fuel_lev_update_time_add(void)
{
    if (fuel_lev_update_time_cnt < ((u16)-1))
    {
        fuel_lev_update_time_cnt++;
    }
}

// 滑动平均：
#define FUEL_VOLTAGE_SAMPLE_COUNT 20 // 样本计数
static volatile u16 fuel_voltage_samples[FUEL_VOLTAGE_SAMPLE_COUNT] = {0};
static volatile u8 fuel_voltage_sample_index = 0;
void __fuel_voltage_samples_init__(u16 voltage)
{
    u8 i;
    for (i = 0; i < FUEL_VOLTAGE_SAMPLE_COUNT; i++)
    {
        fuel_voltage_samples[i] = voltage;
    }
}

void __fuel_voltage_samples_update__(u16 voltage)
{
    fuel_voltage_samples[fuel_voltage_sample_index] = voltage;
    fuel_voltage_sample_index++;
    if (fuel_voltage_sample_index >= FUEL_VOLTAGE_SAMPLE_COUNT)
    {
        fuel_voltage_sample_index = 0;
    }
}

u16 __fuel_voltage_samples_get__(void)
{
    u8 i;
    u32 sum = 0;
    for (i = 0; i < FUEL_VOLTAGE_SAMPLE_COUNT; i++)
    {
        sum += fuel_voltage_samples[i];
    }

    return (sum / FUEL_VOLTAGE_SAMPLE_COUNT);
}

// 将电压转换成油量等级
// 电压值越大，油量越小
u8 fuel_capacity_convert_voltage_to_lev(u16 voltage)
{
    u8 ret = 0; // 默认油量为0
    if (voltage >= FUEL_LEVEL_0_VOLTAGE)
    {
        ret = 0;
    }
    else if (voltage >= FUEL_LEVEL_1_VOLTAGE)
    {
        ret = 1;
    }
    else if (voltage >= FUEL_LEVEL_2_VOLTAGE)
    {
        ret = 2;
    }
    else if (voltage >= FUEL_LEVEL_3_VOLTAGE)
    {
        ret = 3;
    }
    else if (voltage >= FUEL_LEVEL_4_VOLTAGE)
    {
        ret = 4;
    }
    // else if (voltage >= FUEL_LEVEL_5_VOLTAGE)
    // {
    //     ret = 5;
    // }
    else
    {
        ret = 5;
    }

    return ret;
}

void fuel_capacity_scan(void)
{
    u16 fuel_adc_val;
    u16 fuel_voltage;

    static u8 is_initialized = 0;    // 初始化
    static u8 fuel_lev_of_lag = 0; // 延迟显示的油量等级
    u8 cur_fuel_lev = 0;           // 当前实际获取到的油量等级

    if (fuel_capacity_scan_time_cnt >= 200)
    {
        fuel_capacity_scan_time_cnt = 0;
    }
    else
    {
        // 没有到扫描时间周期，直接返回
        return;
    }

    // 获取ad值
    fuel_adc_val = fuel_capacity_adc_val_get();
    // 根据油量检测的ADC值，计算出对应的检测点的电压值
    fuel_voltage = (u32)fuel_adc_val * 2000 / 4096;
    // printf("fuel_adc_val == %u\n", fuel_adc_val);
    // printf("fuel_voltage == %u\n", fuel_voltage);

    if (is_initialized == 0)
    {
        is_initialized = 1;
        __fuel_voltage_samples_init__(fuel_voltage);
        cur_fuel_lev = fuel_capacity_convert_voltage_to_lev(fuel_voltage);
        fuel_lev_of_lag = cur_fuel_lev;

        if (fuel_lev_of_lag == 0)
        {
            // 打开低油量报警 
            if (instrument.flag_is_in_warning_of_low_fuel == 0)
            {
                // 如果之前没有进入低油量报警
                // aip3368h_display_fuel_level(0); // 清空油量显示
                instrument.flag_is_in_warning_of_low_fuel = 1;
            }
        }
        else
        {
            // 关闭低油量报警
            instrument.flag_is_in_warning_of_low_fuel = 0;
            // 正常显示油量
            // aip3368h_display_fuel_level(fuel_lev_of_lag);
        }
    }
    else
    {
        __fuel_voltage_samples_update__(fuel_voltage);
    }

    if (fuel_lev_update_time_cnt < FUEL_UPDATE_TIME)
    {
        // 没有到油量挡位更新时间，直接返回
        return;
    }
    fuel_lev_update_time_cnt = 0;

    // printf("fuel lev update\n");

    cur_fuel_lev = fuel_capacity_convert_voltage_to_lev(
        __fuel_voltage_samples_get__());

    if (fuel_lev_of_lag < cur_fuel_lev)
    {
        fuel_lev_of_lag++;
    }
    else if (fuel_lev_of_lag > cur_fuel_lev)
    {
        fuel_lev_of_lag--;
    }

    if (fuel_lev_of_lag == 0)
    {
        // 打开低油量报警

        if (instrument.flag_is_in_warning_of_low_fuel == 0)
        {
            // 如果之前没有进入低油量报警
            // aip3368h_display_fuel_level(0); // 清空油量显示
            instrument.flag_is_in_warning_of_low_fuel = 1;
        }
    }
    else
    {
        // 关闭低油量报警
        instrument.flag_is_in_warning_of_low_fuel = 0;
        // 正常显示油量
        // aip3368h_display_fuel_level(fuel_lev_of_lag);
    }
}

#endif
