#include "battery.h"

#if BATTERY_SCAN_ENABLE

// static volatile u16 adc_val_of_battery; // 电池电压对应的ad值

volatile u16 battery_scan_time_cnt; // 电池扫描时间计时（在定时器中累加）

// 滑动平均：
#define SAMPLE_COUNT 20 // 样本计数
static volatile u16 bat_adc_val_samples[SAMPLE_COUNT] = {0};
static volatile u8 bat_adc_val_sample_index = 0;

// 初始化 滑动平均 数组
static void __bat_adv_val_samples_init__(u16 adc_val)
{
    u8 i;
    for (i = 0; i < SAMPLE_COUNT; i++)
    {
        bat_adc_val_samples[i] = adc_val;
    }
}

// 将数据放入滑动平均数组
void bat_adc_val_samples_update(u16 adc_val)
{
    static u8 is_initialized = 0;
    if (0 == is_initialized)
    {
        __bat_adv_val_samples_init__(adc_val);
        is_initialized = 1;
        return; // 初始化数组之后，直接退出，下一次得到新数据才执行下面的操作
    }

    bat_adc_val_samples[bat_adc_val_sample_index] = adc_val;
    bat_adc_val_sample_index++;
    if (bat_adc_val_sample_index >= SAMPLE_COUNT)
    {
        bat_adc_val_sample_index = 0;
    }
}

// 从滑动平均数组中读出数据
u16 bat_adc_val_get(void)
{
    u8 i;
    u32 sum = 0;
    for (i = 0; i < SAMPLE_COUNT; i++)
    {
        sum += bat_adc_val_samples[i];
    }
    sum /= SAMPLE_COUNT;

    return sum;
}

void bat_scan_time_add(void)
{
    if (battery_scan_time_cnt < ((u16)-1))
    {
        battery_scan_time_cnt++;
    }
}

void battery_scan(void)
{
    u16 adc_val;
    u16 voltage;

    if (battery_scan_time_cnt >= BAT_SCAN_PERIOD)
    {
        battery_scan_time_cnt = 0;
    }
    else
    {
        return;
    }

    // 这里必须要等滑动平均的数组初始化完成，再获取ad值。否则会得到错误的ad值
    adc_val = bat_adc_val_get();
    voltage = ADC_VAL_TO_BAT_VOLTAGE(adc_val);
    if (voltage >= BAT_CANCEL_LOW_VOLTAGE_WARNING_THRESHOLD)
    {
        instrument.flag_is_in_warning_of_low_voltage = 0;
        // 取消报警之后，需要立即更新显示
        aip3368h_display_bat_err_icon(0);
    }
    else if (voltage < BAT_LOW_VOLTAGE_WARNING_THRESHOLD)
    {
        instrument.flag_is_in_warning_of_low_voltage = 1;
    }
    else
    {
        /*
            如果检测到的电压在 低电压报警阈值 和 取消低电压报警阈值之间，
            不做处理，保持之前的显示
        */
    }

    // printf("adc_val == %u\n", adc_val);
    // printf("voltage == %u\n", voltage);
}

#endif // BATTERY_SCAN_ENABLE