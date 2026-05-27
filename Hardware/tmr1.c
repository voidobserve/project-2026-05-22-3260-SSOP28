#include "tmr1.h"

// 定时器定时周期 (单位:Hz)
// 周期值 = 系统时钟 / 定时器分频 / 频率 - 1
#define TMR1_PERIOD (SYSCLK / 128 / 1000 - 1) // 1000Hz,1ms

/**
 * @brief 配置定时器TMR1，配置完成后，定时器默认关闭
 */
void tmr1_config(void)
{
    __SetIRQnIP(TMR1_IRQn, TMR1_IQn_CFG); // 设置中断优先级（TMR1）
    __DisableIRQ(TMR1_IRQn);              // 禁用中断
    IE_EA = 1;                            // 打开总中断

    TMR_ALLCON = TMR1_CNT_CLR(0x1); // 清除计数值

    // USER_TO_DO 待优化
    TMR1_CONL &= ~TMR_PRESCALE_SEL(0x07); // 清除TMR1的预分频配置寄存器
    TMR1_CONL |= TMR_PRESCALE_SEL(0x07);  // 定时器预分频
    TMR1_CONL &= ~TMR_MODE_SEL(0x03);     // 清除TMR1的模式配置寄存器
    TMR1_CONL |= TMR_MODE_SEL(0x01);      // 配置TMR1的模式为计数器模式，最后对系统时钟的脉冲进行计数

    TMR1_CONH &= ~TMR_PRD_PND(0x01); // 清除TMR1的计数标志位，表示未完成计数
    TMR1_CONH |= TMR_PRD_IRQ_EN(1);  // 使能TMR1的计数中断

    // 配置TMR1的计数周期
    TMR1_PRH = TMR_PERIOD_VAL_H((TMR1_PERIOD >> 8) & 0xFF); // 周期值
    TMR1_PRL = TMR_PERIOD_VAL_L((TMR1_PERIOD >> 0) & 0xFF);

    // TMR1_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除TMR1的时钟源配置寄存器
    // TMR1_CONL |= TMR_SOURCE_SEL(0x05);    // 配置TMR1的时钟源，不用任何时钟

    // 重新给TMR1配置时钟
    TMR1_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除定时器的时钟源配置寄存器
    TMR1_CONL |= TMR_SOURCE_SEL(0x06);    // 配置定时器的时钟源，使用系统时钟

    __EnableIRQ(TMR1_IRQn); // 使能中断
    // IE_EA = 1;              // 打开总中断
}

// TMR1中断服务函数
void TIMR1_IRQHandler(void) interrupt TMR1_IRQn
{
    // 进入中断设置IP，不可删除
    __IRQnIPnPush(TMR1_IRQn);

    // ---------------- 用户函数处理 -------------------

    // 周期中断
    if (TMR1_CONH & TMR_PRD_PND(0x1))
    {
        TMR1_CONH |= TMR_PRD_PND(0x1); // 清除pending

        if (mileage_save_time_cnt < 65535)
        {
            mileage_save_time_cnt++;
        }

        if (io_key_para.cur_scan_times < 255)
        {
            io_key_para.cur_scan_times++;
        }

#if FUEL_CAPACITY_SCAN_ENABLE
        fuel_capacity_scan_time_add();
        fuel_lev_update_time_add();
#endif

        // if (mileage_update_time_cnt < 65535)
        {
            mileage_update_time_cnt++;
        }

#if BATTERY_SCAN_ENABLE
        // bat_scan_time_add();
#endif // BATTERY_SCAN_ENABLE

        // adc_channel_switch_by_isr();
        
        aip3368h_refresh_time_add();
        // aip3368h_display_speed_refresh_time_add();
        // aip3368h_display_engine_speed_refresh_time_add();
        // aip3368h_display_err_handle_time_add();

        // 修改显存的操作放到了主循环，这里仅用作计时
        // aip3368h_display_boot_animation_time_add();

        // USER_TO_DO 只在测试时只用： 
        // aip3368h_display_test_engine_speed_scale_bar_1ms_isr();
        // aip3368h_display_test_fuel_level_1ms_isr();
        // aip3368h_display_test_mileage_1ms_isr();
        // aip3368h_display_test_speed_1ms_isr();
        // aip3368h_display_test_speed_scale_bar_1ms_isr();
    }
 
    // 退出中断设置IP，不可删除
    __IRQnIPnPop(TMR1_IRQn);
}