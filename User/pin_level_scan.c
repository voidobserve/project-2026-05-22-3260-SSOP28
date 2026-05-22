#include "pin_level_scan.h"

#if PIN_LEVEL_SCAN_ENABLE
void pin_level_scan_config(void)
{
    // 检测 故障 的引脚:
    P2_PU |= GPIO_P20_PULL_UP(0x01);      // 上拉
    P2_MD0 &= ~(GPIO_P20_MODE_SEL(0x03)); // 输入模式    
}

// 引脚电平扫描
void pin_level_scan(void)
{
    if (0 == PIN_DETECT_BREAKDOWN)
    {
        aip3368h_display_err_icon(1);
    }
    else
    {
        aip3368h_display_err_icon(0);
    }
}

#endif
