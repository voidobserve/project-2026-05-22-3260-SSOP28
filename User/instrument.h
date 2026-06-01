#ifndef __INSTRUMENT_H__
#define __INSTRUMENT_H__

#include "include.h" // 使用芯片官方提供的头文件
#include "aip3368h_display.h"
#include "my_config.h" // 包含自定义的头文件

// 挡位的定义
enum
{
    GEAR_NEUTRAL = 0x00, // 空挡
    GEAR_FIRST = 0x01,   // 一档
    GEAR_SECOND = 0x02,  // 二档
    GEAR_THIRD = 0x03,   // 三档
    GEAR_FOURTH = 0x04,  // 四档
    GEAR_FIFTH = 0x05,   // 五档
    GEAR_SIXTH = 0x06,   // 六档
    /*
        未知，如果 GEAR_NEUTRAL ~ GEAR_SIXTH 都没有检测到，
        则返回 GEAR_UNKNOWN ， 让显示屏中档位对应的图标空着
    */
    GEAR_UNKNOWN = 0xFF,
};
typedef u8 gear_t;

// 定义存储在flash中的数据
typedef struct
{
    // 总里程表（单位：m，使用英制单位时，只需要再发送时进行转换）
    // （大计里程，范围：0 ~ 999999 KM）
    u32 total_mileage;
    // 短距离里程表(单位：m，使用英制单位时，只需要再发送时进行转换)
    // （小计里程，范围：0 ~ 99999.9 KM）
    u32 subtotal_mileage;

    u8 is_display_total_mileage; // 0：显示总里程，1：显示短距离里程
    distance_unit_type_t tmp;    // 要显示的时速单位类型，km/h 或 mph

    u8 is_save_data_valid;
} save_info_t;

typedef struct
{
    save_info_t save_info;
    u32 engine_speed; // 发动机的转速（单位：rpm）
    u8 speed;         // 时速(单位：km/h，使用英制单位时，需要进行转换)
    u8 fuel;          // 油量(单位：百分比)

    // 标志位，是否处于发动机转速过高的报警状态
    u8 flag_is_engine_speed_warning_enable;
    // 标志位，是否处于低电压报警状态
    u8 flag_is_in_warning_of_low_voltage;
    // 标志位，是否处于低油量报警
    u8 flag_is_in_warning_of_low_fuel;
 
    gear_t gear; 

} instrument_t;
extern volatile instrument_t instrument;

void instrument_info_init(void);
void instrument_info_save(void);

#endif
