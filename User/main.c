/**
 ******************************************************************************
 * @file    main.c
 * @author  HUGE-IC Application Team
 * @version V1.0.0
 * @date    05-11-2022
 * @brief   Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2021 HUGE-IC</center></h2>
 *
 * 版权说明后续补上
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "include.h"
#include "my_config.h"
#include "string.h"

#if USER_DEBUG_ENABLE
void debug_pin_init(void)
{
    P2_MD0 &= ~GPIO_P21_MODE_SEL(0x03);
    P2_MD0 |= GPIO_P21_MODE_SEL(0x01); // 输出模式
    FOUT_S21 = GPIO_FOUT_AF_FUNC;
    DEBUG_PIN = 0;
}
#endif

void user_init(void)
{
#if USER_DEBUG_ENABLE
    debug_pin_init();
    uart0_debug_init();
    printf("sys reset\n");
#endif

#if PIN_LEVEL_SCAN_ENABLE
    // pin_level_scan_config();
#endif

#if IO_KEY_ENABLE
    // io_key_config(); // 按键的配置
#endif

#if SPEED_SCAN_ENABLE
    // speed_scan_config(); // 时速扫描的配置
#endif

#if ENGINE_SPEED_SCAN_ENABLE
    // engine_speed_scan_config(); // 发动机转速扫描的配置
#endif

#if (BATTERY_SCAN_ENABLE || FUEL_CAPACITY_SCAN_ENABLE)
    // adc_config();
#endif

    instrument_info_init(); // 初始化仪表信息
    aip3368h_module_init();

    tmr1_config(); //
    tmr2_config(); // 扫描脉冲(电平变化)的定时器

    delay_ms(1); // 等待系统稳定
                 // delay_ms(2000); // 等待系统稳定
}

void main(void)
{
    // 看门狗默认打开, 复位时间2s
    system_init();

    // WDT_KEY = WDT_KEY_VAL(0xDD); //  关闭看门狗

    // 关闭HCK和HDA的调试功能
    WDT_KEY = 0x55;  // 解除写保护
    IO_MAP &= ~0x01; // 清除这个寄存器的值，实现关闭HCK和HDA引脚的调试功能（解除映射）
    WDT_KEY = 0xBB;

    /* 用户代码初始化接口 */
    user_init();

    // USER_TO_DO
    // 上电之后，需要先跑一遍开机动画，再继续主循环
    // aip3368h_display_boot_animation_handle();

    // USER_TO_DO 测试时使用
    aip3368h_display_test();
    // aip3368h_display_mileage(123456, 0);
    

    /* 系统主循环 */
    while (1)
    {
        // printf("main circle\n");
        // DEBUG_PIN = ~DEBUG_PIN;

#if 1
        WDT_KEY = WDT_KEY_VAL(0xAA); // 喂狗并清除 wdt_pending

#if PIN_LEVEL_SCAN_ENABLE
        // pin_level_scan();
#endif

#if IO_KEY_ENABLE
        // key_driver_scan(&io_key_para);
        // io_key_handle(); // io按键处理函数
#endif

#if SPEED_SCAN_ENABLE
        // speed_scan(); // 检测时速
        // aip3368h_display_speed_handle();
#endif

        // mileage_scan(); // 检测大计里程和小计里程

#if ENGINE_SPEED_SCAN_ENABLE
        // engine_speed_scan(); // 检测发动机转速
        // aip3368h_display_engine_speed_handle();
#endif

#if FUEL_CAPACITY_SCAN_ENABLE
        // fuel_capacity_scan(); // 油量检测
#endif

#if BATTERY_SCAN_ENABLE
        // battery_scan(); // 电池电量检测
#endif

#endif //

        // memset(aip3368h_display_buff, 0x00, sizeof(aip3368h_display_buff));
        // aip3368h_module_display();
        // delay_ms(500);

        // memset(aip3368h_display_buff, 0xFF, sizeof(aip3368h_display_buff));
        aip3368h_module_display();
        // delay_ms(500);
    }
}

/**
 * @}
 */

/*************************** (C) COPYRIGHT 2022 HUGE-IC ***** END OF FILE *****/
