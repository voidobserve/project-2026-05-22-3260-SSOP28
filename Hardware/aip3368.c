#include "aip3368.h"

#include "string.h" // memset
#include "include.h"

#if 1 // AIP3368H_MODULE

static volatile u16 aip3368h_refresh_cnt = 0;

// 显存
volatile u16 aip3368h_display_buff[AIP3368H_DEV_NUM];

// 放在1ms的定时器中
void aip3368h_refresh_time_add(void)
{
    // 防止计数溢出
    if (aip3368h_refresh_cnt < ((u16)-1))
    {
        aip3368h_refresh_cnt++;
    }
}

static void aip3368h_module_send_data_to_one_dev(u16 dat)
{
    volatile u16 i;
    // PDM = 1;

    for (i = 0; i < 16; i++)
    {
        DIO = dat & (u16)0x8000 ? 1 : 0;

        aip3368h_delay();
        DCK = 1;
        aip3368h_delay();
        dat <<= 1;
        DCK = 0;
        aip3368h_delay();
    }
    // PDM = 0;
}

static void aip3368h_module_send_data_to_all_dev(u16 *buff, u8 len)
{
    volatile u8 i;

    // EA = 0;
    // 开始
    DCK = 0;
    LAT = 0;
    // PDM = 1;
    aip3368h_delay();

    // 一帧完整数据
    for (i = 0; i < len; i++)
    {
        aip3368h_module_send_data_to_one_dev(buff[i]);
    }

    // 结束
    // PDM = 1;
    LAT = 1;
    aip3368h_delay();
    LAT = 0;
    // PDM = 0;
    aip3368h_delay();
    DIO = 0;

    // EA = 1;
}

#define AIP3368H_FLASH_TEST_ENABLE 0
// 根据显存中的数据，更新显示
void aip3368h_module_display(void)
{
    // if (!systimer_flag_is_valid(SYSTIME_FLAG_50MS))
    //     return;

    // 刷新间隔 单位：ms
    if (aip3368h_refresh_cnt < 25)
    {
        return;
    }
    else
    {
        aip3368h_refresh_cnt = 0;
    }

#if AIP3368H_FLASH_TEST_ENABLE
    // 闪烁测试
    if (aip3368h_display_buff[0] == 0x0000)
        memset(aip3368h_display_buff, 0xFF, sizeof(aip3368h_display_buff));
    else
        memset(aip3368h_display_buff, 0x00, sizeof(aip3368h_display_buff));
#endif

    aip3368h_module_send_data_to_all_dev(aip3368h_display_buff, AIP3368H_DEV_NUM);
}

// void aip3368h_module_clear(void)
// {
//     memset(aip3368h_display_buff, 0x00, sizeof(aip3368h_display_buff));
//     aip3368h_module_send_data_to_all_dev(aip3368h_display_buff, AIP3368H_DEV_NUM);
// }

void aip3368h_module_init(void)
{
    // memset(aip3368h_display_buff, 0xF0, sizeof(aip3368h_display_buff));
    // 显示驱动芯片有记忆功能（数据锁存），每次上电应该清空显存
    memset(aip3368h_display_buff, 0x00, sizeof(aip3368h_display_buff));

    // DCK
    P1_MD0 &= ~GPIO_P11_MODE_SEL(0x03);
    P1_MD0 |= GPIO_P11_MODE_SEL(0x01);
    FOUT_S11 = GPIO_FOUT_AF_FUNC;
    // DIO
    P1_MD0 &= ~GPIO_P12_MODE_SEL(0x03);
    P1_MD0 |= GPIO_P12_MODE_SEL(0x01);
    FOUT_S12 = GPIO_FOUT_AF_FUNC;
    // LAT
    P0_MD0 &= ~GPIO_P03_MODE_SEL(0x03);
    P0_MD0 |= GPIO_P03_MODE_SEL(0x01);
    FOUT_S03 = GPIO_FOUT_AF_FUNC;
    // PDM
    P0_MD0 &= ~GPIO_P00_MODE_SEL(0x03);
    P0_MD0 |= GPIO_P00_MODE_SEL(0x01);
    FOUT_S00 = GPIO_FOUT_AF_FUNC;

    DIO = 0;
    DCK = 0;
    LAT = 0;
    PDM = 0;
    aip3368h_module_send_data_to_all_dev(aip3368h_display_buff, AIP3368H_DEV_NUM);
}

// void aip3368h_module_uninit(void)
// {
//     aip3368h_module_clear();    // 清屏
//     DIO = 0;
//     DCK = 0;
//     LAT = 0;
// }

#endif
