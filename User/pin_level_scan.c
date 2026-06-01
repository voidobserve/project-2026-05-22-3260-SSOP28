#include "pin_level_scan.h"

#if PIN_LEVEL_SCAN_ENABLE
void pin_level_scan_config(void)
{
    // 检测 故障 的引脚:
    P2_PU |= GPIO_P26_PULL_UP(0x01);      // 上拉
    P2_MD1 &= ~(GPIO_P26_MODE_SEL(0x03)); // 输入模式

    // N 档 检测脚：
    P0_PU |= GPIO_P06_PULL_UP(0x01); // 上拉
    P0_MD1 &= ~GPIO_P06_MODE_SEL(0x03);

    // 1 档 检测脚：
    P0_PU |= GPIO_P07_PULL_UP(0x01); // 上拉
    P0_MD1 &= ~GPIO_P07_MODE_SEL(0x03);

    // 2 档 检测脚
    P1_PU |= GPIO_P10_PULL_UP(0x01);
    P1_MD0 &= ~GPIO_P10_MODE_SEL(0x03);

    // 3 档 检测脚
    P1_PU |= GPIO_P13_PULL_UP(0x01);
    P1_MD0 &= ~GPIO_P13_MODE_SEL(0x03);

    // 4 档 检测脚
    P1_PU |= GPIO_P14_PULL_UP(0x01);
    P1_MD1 &= ~GPIO_P14_MODE_SEL(0x03);

    // 5 档 检测脚
    P3_PU |= GPIO_P30_PULL_UP(0x01);
    P3_MD0 &= ~GPIO_P30_MODE_SEL(0x03);

    // 6 档 检测脚
    P2_PU |= GPIO_P27_PULL_UP(0x01);
    P2_MD1 &= ~GPIO_P27_MODE_SEL(0x03);

    // 小灯（近光灯） 检测脚
    P2_MD0 &= ~GPIO_P23_MODE_SEL(0x03);

    // 大灯（远光灯） 检测脚
    P2_MD0 &= ~GPIO_P21_MODE_SEL(0x03);

    // 左转向 检测脚
    P2_MD1 &= ~GPIO_P25_MODE_SEL(0x03);

    // 右转向 检测脚
    P2_MD0 &= ~GPIO_P22_MODE_SEL(0x03);
}

// 引脚电平扫描
void pin_level_scan(void)
{
    // 扫描挡位时，以检测到的最低挡位为优先
    if (0 == PIN_DETECT_GEAR_N)
    {
        instrument.gear = GEAR_NEUTRAL;
    }
    else if (0 == PIN_DETECT_GEAR_1)
    {
        instrument.gear = GEAR_FIRST;
    }
    else if (0 == PIN_DETECT_GEAR_2)
    {
        instrument.gear = GEAR_SECOND;
    }
    else if (0 == PIN_DETECT_GEAR_3)
    {
        instrument.gear = GEAR_THIRD;
    }
    else if (0 == PIN_DETECT_GEAR_4)
    {
        instrument.gear = GEAR_FOURTH;
    }
    else if (0 == PIN_DETECT_GEAR_5)
    {
        instrument.gear = GEAR_FIFTH;
    }
    else if (0 == PIN_DETECT_GEAR_6)
    {
        instrument.gear = GEAR_SIXTH;
    }
    else
    {
        // 如果 N档 、 1档 ~ 6档 都没有检测到，说明挡位没有接线
        instrument.gear = GEAR_UNKNOWN;
    }

    aip3368h_display_gear(instrument.gear);
    aip3368h_display_gear_light(1);

    if (0 == PIN_DETECT_BREAKDOWN)
    {
        aip3368h_display_err_light(1);
    }
    else
    {
        aip3368h_display_err_light(0);
    }

    // 引脚检测到高电平，点亮对应的指示灯，不用闪烁
    if (PIN_DETECT_LFFT_TURN)
    {
        aip3368h_display_left_turn_light(1);
    }
    else
    {
        aip3368h_display_left_turn_light(0);
    }

    // 引脚检测到高电平，点亮对应的指示灯，不用闪烁
    if (PIN_DETECT_RIGHT_TURN)
    {
        aip3368h_display_right_turn_light(1);
    }
    else
    {
        aip3368h_display_right_turn_light(0);
    }

    // 引脚检测到高电平，点亮对应的指示灯
    if (PIN_DETECT_LOW_BEAM)
    {
        aip3368h_display_low_beam_indicator_light(1);
    }
    else
    {
        aip3368h_display_low_beam_indicator_light(0);
    }

    // 引脚检测到高电平，点亮对应的指示灯
    if (PIN_DETECT_HIGH_BEAM)
    {
        aip3368h_display_high_beam_indicator_light(1);
    }
    else
    {
        aip3368h_display_high_beam_indicator_light(0);
    }
}

#endif
