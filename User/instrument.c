#include <string.h> // memset()

#include "instrument.h"
#include "my_config.h"

volatile instrument_t instrument = {0};

void instrument_info_init(void)
{
    save_info_t save_info;
    // 从flash中读出数据
    flash_read(FLASH_START_ADDR,
               (u8 *)&save_info,
               sizeof(save_info_t));
    if (USER_FLASH_DATA_VALID_VAL == save_info.is_save_data_valid)
    {
        // 如果保存在flash中的数据有效
        instrument.save_info.total_mileage = save_info.total_mileage;
        instrument.save_info.subtotal_mileage = save_info.subtotal_mileage;
        instrument.save_info.is_display_total_mileage = save_info.is_display_total_mileage;
    }
    else
    {
        // 如果保存在flash中的数据无效，全局变量 instrument 中的元素 默认全部为0

        instrument.save_info.is_display_total_mileage = 1; // 默认显示大计里程
        instrument.save_info.is_save_data_valid = USER_FLASH_DATA_VALID_VAL;
        instrument.save_info.distance_unit_type = DISTANCE_UNIT_TYPE_METRIC;
        // instrument.save_info.distance_unit_type = DISTANCE_UNIT_TYPE_IMPERIAL;

        instrument_info_save(); // 将数据写回flash
    }
}

void instrument_info_save(void)
{
#if USE_INTERNAL_FLASH_SAVE_DATA

    instrument.save_info.is_save_data_valid = USER_FLASH_DATA_VALID_VAL; // 表示数据有效，让下一次上电读出数据时，验证该标志位
    // 先擦除扇区再写入
    flash_erase_sector(FLASH_START_ADDR);
    flash_program(FLASH_START_ADDR,
                  (u8 *)&instrument.save_info,
                  sizeof(save_info_t));
#endif
}

// 控制设置界面对应的设置项目以一定周期进行闪烁
static volatile u16 aip3368h_display_setting_item_time_cnt = 0;
// 进入设置界面后，一段时间没有操作时，退出设置：
static volatile u16 aip3368h_display_setting_item_exit_time_cnt;

void aip3368h_display_setting_item_time_add(void)
{
    if (aip3368h_display_setting_item_time_cnt < ((u16)-1))
    {
        aip3368h_display_setting_item_time_cnt++;
    }
}

// 清除设置界面对应设置项目的闪烁时间，让它重新开始计时并闪烁
void aip3368h_display_setting_item_time_clear(void)
{
    aip3368h_display_setting_item_time_cnt = 0;
}

// USER_TO_DO 目前还没有添加自动退出功能
void aip3368h_display_setting_item_exit_time_add(void)
{
    if (aip3368h_display_setting_item_exit_time_cnt < ((u16)-1))
    {
        aip3368h_display_setting_item_exit_time_cnt++;
    }
}

void aip3368h_display_setting_item_exit_time_clear(void)
{
    aip3368h_display_setting_item_exit_time_cnt = 0;
}

void aip3368h_display_setting_item_handle(void)
{
    if (aip3368h_display_setting_item_time_cnt >= 500)
    {
        aip3368h_display_setting_item_time_cnt = 0;
    }
    else
    {
        return;
    }

    switch (instrument.cur_sel_setting_item)
    {
    // case SETTING_ITEM_IS_DISPLAY_TOTAL_MILEAGE:

    //     break;
    case SETTING_ITEM_DISTANCE_UNIT_TYPE:
        // 让 时速 和 里程 的单位都进行闪烁显示

        if (DISTANCE_UNIT_TYPE_METRIC ==
            instrument.save_info.distance_unit_type)
        {
            // 如果当前 设置的项目 是公制单位

            // 直接读取显存，判断有没有点亮对应的指示灯
            if ((aip3368h_display_buff[5] >> 10) & 0x01)
            {
                // 如果是点亮的，改为熄灭
                __aip3368h_display_speed_unit_type__(DISTANCE_UNIT_TYPE_METRIC, 0);
                __aip3368h_display_mileage_unit_type__(DISTANCE_UNIT_TYPE_METRIC, 0);
            }
            else
            {
                // 如果当前是熄灭的，点亮它
                __aip3368h_display_speed_unit_type__(DISTANCE_UNIT_TYPE_METRIC, 1);
                __aip3368h_display_mileage_unit_type__(DISTANCE_UNIT_TYPE_METRIC, 1);
            }
        }
        else if (DISTANCE_UNIT_TYPE_IMPERIAL ==
                 instrument.save_info.distance_unit_type)
        {
            // 直接读取显存，判断有没有点亮对应的指示灯
            if ((aip3368h_display_buff[5] >> 11) & 0x01)
            {
                // 如果是点亮的，改为熄灭
                __aip3368h_display_speed_unit_type__(DISTANCE_UNIT_TYPE_IMPERIAL, 0);
                __aip3368h_display_mileage_unit_type__(DISTANCE_UNIT_TYPE_IMPERIAL, 0);
            }
            else
            {
                // 如果当前是熄灭的，点亮它
                __aip3368h_display_speed_unit_type__(DISTANCE_UNIT_TYPE_IMPERIAL, 1);
                __aip3368h_display_mileage_unit_type__(DISTANCE_UNIT_TYPE_IMPERIAL, 1);
            }
        }
        break;
    case SETTING_ITEM_WHELL_CIRCUMFERENCE:
        // 正在设置车轮周长
        /*
            设置范围 ： 50 ~ 180，每次调节步长为5，
            可以直接判断时速第2位的A段数码管有没有点亮，
            来控制闪烁
        */
        if ((aip3368h_display_buff[6] >> 1) & 0x01)
        {
            // 如果是点亮的，改为熄灭
            __aip3368h_display_speed_bit_x_clear__(0);
            __aip3368h_display_speed_bit_x_clear__(1);
            __aip3368h_display_speed_bit_x_clear__(2);
        }
        else
        {
            // 如果当前是熄灭的，点亮它

            // 通过调用显示时速的接口来显示对应的数字
            // aip3368h_display_speed();
        }

        break;

    default:
        break;
    }
}
