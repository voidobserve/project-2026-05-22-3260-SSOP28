#ifndef __AIP3368H_DISPLAY_H__
#define __AIP3368H_DISPLAY_H__

// #include "my_config.h"
#include "typedef.h"

#define AIP3368H_DISPLAY_TEST_ENABLE 1

// 用于建立指示灯和显存的映射关系：
typedef struct
{
    u8 buff_index; // aip3368h_display_buff[] 中对应元素索引
    u8 bit_offset; // aip3368h_display_buff[] 中对应元素中的第 x 位
} aip3368h_display_mapping_t;

// 数码管 A ~ G 段索引值
enum
{
    SEG_INDEX_A = 0x00,
    SEG_INDEX_B,
    SEG_INDEX_C,
    SEG_INDEX_D,
    SEG_INDEX_E,
    SEG_INDEX_F,
    SEG_INDEX_G,
};
typedef u8 seg_index_t;

// 单位类型：公制单位 或 英制单位
enum
{
    DISTANCE_UNIT_TYPE_METRIC,   // 公制单位，时速用 km/h ，里程用 km
    DISTANCE_UNIT_TYPE_IMPERIAL, // 英制单位，时速用 mph ，里程用 mile
};
typedef u8 distance_unit_type_t; // 距离相关的单位类型

enum
{
    AIP3368H_DISPLAY_FUEL_LEVEL_EMPTY = 0,
    AIP3368H_DISPLAY_FUEL_LEVEL_0,
    AIP3368H_DISPLAY_FUEL_LEVEL_1,
    AIP3368H_DISPLAY_FUEL_LEVEL_2,
    AIP3368H_DISPLAY_FUEL_LEVEL_3,
    AIP3368H_DISPLAY_FUEL_LEVEL_4,
    AIP3368H_DISPLAY_FUEL_LEVEL_5, // FULL
};
typedef u8 aip3368h_display_fuel_level_t;

/*
    开机动画：
    时速
    背光刻度条
    整个面板各个指示灯从左到右依次点亮
*/
enum
{
    BOOT_ANIMATION_PHASE_SPEED = 0,
    BOOT_ANIMATION_PHASE_BACKLIGHT,
    BOOT_ANIMATION_PHASE_LEFT_TO_RIGHT,
    BOOT_ANIMATION_PAHSE_END, // 结束
};
typedef u8 boot_animation_phase_t;

typedef struct
{
    u8 is_in_boot_animation;         // 是否处于启动动画中
    u8 boot_animation_time_add_flag; // 给开机动画处理函数提供时基
    u16 boot_animation_time_cnt;     // 记录整个开机动画的时间

    boot_animation_phase_t boot_animation_phase; // 开机动画的各个阶段

} aip3368h_display_obj_t;
extern volatile aip3368h_display_obj_t aip3368h_display_obj;

void aip3368h_display_left_turn_light(u8 is_enable);           // 左转灯
void aip3368h_display_right_turn_light(u8 is_enable);          // 右转灯
void aip3368h_display_err_light(u8 is_enable);                 // 故障 指示
void aip3368h_display_low_beam_indicator_light(u8 is_enable);  // 显示 小灯（近光灯） 指示灯
void aip3368h_display_high_beam_indicator_light(u8 is_enable); // 显示 大灯（远光灯） 指示灯
void aip3368h_display_back_light_scale_bar(u8 level);
void aip3368h_display_engine_speed_scale_bar(u8 level);
void aip3368h_display_x1000rpm_light(u8 is_enable);
void aip3368h_display_engine_speed_digit_scale(u8 num);
void aip3368h_display_gear_light(u8 is_enable); // 显示 挡位 "GEAR" 字样指示灯（白）
void __aip3368h_display_gear_n_light__(u8 is_enable);
void __aip3368h_display_gear_digit__(u8 level);
void aip3368h_display_gear(u8 level);

// 时速单位类型：
void __aip3368h_display_speed_unit_type__(distance_unit_type_t type, u8 is_enable);
void aip3368h_display_speed_unit_type(distance_unit_type_t type);
// 时速：
void __aip3368h_display_speed_seg__(u8 bit_x, seg_index_t seg, u8 is_enable);
void __aip3368h_display_speed_bit_x__(u8 bit_x, u8 number);
void aip3368h_display_speed(u8 speed);
// 里程单位类型：
void __aip3368h_display_mileage_unit_type__(distance_unit_type_t type, u8 is_enable);
void aip3368h_display_mileage_unit_type(distance_unit_type_t type);
// 里程
void __aip3368h_display_mileage_bit_x__(u8 bit_x, u8 number);
void __aip3368h_display_mileage_total_light__(u8 is_enable);
void __aip3368h_display_mileage_trip_light__(u8 is_enable);
void __aip3368h_display_mileage_point__(u8 is_enable);
void aip3368h_display_mileage(u32 mileage, u8 is_displaying_total_mileage);

// 油量图标
void __aip3368h_display_fuel_icon__(u8 is_enable);
// 油量"E"字样 指示灯
void __aip3368h_display_fuel_empty_light__(u8 is_enable);
// 油量"F"字样 指示灯
void __aip3368h_display_fuel_full_light__(u8 is_enable);
void aip3368h_display_fuel_level(aip3368h_display_fuel_level_t level);

// void aip3368h_display_engine_speed_back_light(void);
// void aip3368h_display_exclamation_point(u8 is_enable);
// void aip3368h_display_engine_speed_scale_bar(u8 level);
// void aip3368h_display_bat_err_icon(u8 is_enable);
// void aip3368h_display_err_icon(u8 is_enable);
// void aip3368h_display_fuel_level(aip3368h_display_fuel_level_t level);
// void aip3368h_display_mileage_km_icon(u8 is_enable);

// void aip3368h_display_speed_km_icon(u8 is_enable);

// void aip3368h_display_speed_scale_bar(u8 level);

// void aip3368h_display_boot_animation_1ms_isr(void);
void aip3368h_display_boot_animation_time_add(void);
void aip3368h_display_boot_animation_handle(void);

void aip3368h_display_err_handle_time_add(void);
void aip3368h_display_err_handle(void);

#if AIP3368H_DISPLAY_TEST_ENABLE

void aip3368h_display_test_light_blink_1ms_isr(void);

void aip3368h_display_test_back_light_scale_bar(void);
void aip3368h_display_test_engine_speed_scale_bar(void);
void aip3368h_display_test_engine_speed_digit_scale(void);
void aip3368h_display_test_gear(void);
void aip3368h_display_test_speed(void);
void aip3368h_display_test_mileage(void);
void aip3368h_display_test_fuel(void);

void aip3368h_display_test(void);
#endif

#endif