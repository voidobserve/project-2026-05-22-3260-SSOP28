#include "instrument.h"
#include <string.h> // memset()

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
