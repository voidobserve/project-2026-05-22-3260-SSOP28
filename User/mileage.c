// encoding UTF-8
// mileage.c
#include "mileage.h"

volatile u16 mileage_save_time_cnt; // 里程扫描所需的计数值,每隔一定时间将里程写入flash
volatile u32 distance;              // 存放每次扫描时走过的路程（单位：毫米）-->用于里程表的计数

volatile u16 mileage_update_time_cnt; // 里程更新的时间计数,每隔一段时间更新一次当前里程（负责控制发送里程的周期）

// 刷新显示的里程（TOTAL 或 TRIP），不包括单位
void aip3368h_display_mileage_refresh(void)
{
    // 刷新总里程
    if (instrument.save_info.is_display_total_mileage)
    { 
        if (instrument.save_info.distance_unit_type ==
            DISTANCE_UNIT_TYPE_METRIC)
        {
            // 使用 公制 单位
            aip3368h_display_mileage(
                instrument.save_info.total_mileage / 1000,
                1);
        }
        else if (instrument.save_info.distance_unit_type ==
                 DISTANCE_UNIT_TYPE_IMPERIAL)
        {
            // 使用 英制 单位
            // 1km == 0.621427mile
            // 0.621427 mile * 1610 == 1000.49747 m
            aip3368h_display_mileage(
                instrument.save_info.total_mileage / 1610,
                1);
        }
    }
    else
    {
        // 刷新 小计里程(TRIP)

        if (instrument.save_info.distance_unit_type ==
            DISTANCE_UNIT_TYPE_METRIC)
        {
            // 使用 公制 单位
            aip3368h_display_mileage(
                instrument.save_info.subtotal_mileage / 100,
                0);
        }
        else if (instrument.save_info.distance_unit_type ==
                 DISTANCE_UNIT_TYPE_IMPERIAL)
        {
            // 使用 英制 单位
            // 1km == 0.621427mile
            // 0.621427 mile * 1610 == 1000.49747 m
            aip3368h_display_mileage(
                instrument.save_info.subtotal_mileage / 161,
                0);
        }

        // aip3368h_display_mileage(
        //     instrument.save_info.subtotal_mileage / 100, 0);
    }
}

// 总里程扫描
void mileage_scan(void)
{
    // static u8 is_initialized = 0;

    /*
        是否有里程数据需要保存的标志变量，
        0--没有里程变化，不需要保存，
        1--有里程变化，需要保存
        目前每过1m就会置位一次，保存之后清零
    */
    static volatile bit flag_is_any_mileage_save;

    // 每过1s，且里程有变化，就保存一次；这个里程变化的条件最好大于10m，否则会经常写入eeprom
    if ((mileage_save_time_cnt >= (u16)5 * 1000) && /* xx ms后 */
        flag_is_any_mileage_save)          /* 里程有变化，需要保存 */
    {
        instrument_info_save();
        flag_is_any_mileage_save = 0;
        mileage_save_time_cnt = 0;

        // printf("mile save\n");

        // printf("total_mileage %lu\n", instrument.save_info.total_mileage);
        // printf("sub_total_mileage %lu\n", instrument.save_info.subtotal_mileage);
        // printf("sub_total_mileage_2 %lu\n", instrument.save_info.subtotal_mileage_2);
    }

    if (distance >= 1000) // 1000mm -- 1m
    {
        // 如果走过的距离超过了1m，再进行保存（保存到变量）
        if (instrument.save_info.total_mileage < (u32)(999999 * 1000)) // 99 9999 KM
        {
            instrument.save_info.total_mileage++; // +1m
        }

        if (instrument.save_info.subtotal_mileage < (u32)(999999 * 100)) // 99999.9 KM
        {
            instrument.save_info.subtotal_mileage++; // +1m
        }

        distance -= 1000; // 剩下的、未保存的、不满1m的数据留到下一次再保存

        {
            static u8 cnt = 0;
            cnt++;
            // if (cnt >= 10) // cnt >= 10，说明走过了10m
            if (cnt >= 100) 
            {
                cnt = 0;
                flag_is_any_mileage_save = 1; // 表示需要把里程输入写入到flash
            }
        }
    }

    // if (0 == is_initialized || mileage_update_time_cnt >= MILEAGE_UPDATE_TIME_MS)
    // {
    //     // 每隔一段时间，发送大小里程，
    //     // 因为最后大计里程在999999km,小计里程在999.9km之后，就不更新了，
    //     // 要再刷新一次，才会发送1000000km和1000.0km的大小里程
    //     mileage_update_time_cnt = 0;

    //     if (0 == is_initialized)
    //     {
    //         aip3368h_display_mileage_unit_type(
    //             instrument.save_info.distance_unit_type);
    //         is_initialized = 1;
    //     }

    //     aip3368h_display_mileage_refresh();
    // }
}

void aip3368h_display_mileage_handle(void)
{
    static u8 is_initialized = 0;

    if (0 == is_initialized)
    {
        is_initialized = 1;

        // 显示单位
        aip3368h_display_mileage_unit_type(
            instrument.save_info.distance_unit_type);
        aip3368h_display_mileage_refresh();
    }

    if (mileage_update_time_cnt >= MILEAGE_UPDATE_TIME_MS)
    {
        // 每隔一段时间，更新里程
        mileage_update_time_cnt = 0; 
        aip3368h_display_mileage_refresh();
    }
}
