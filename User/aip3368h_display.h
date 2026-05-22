#ifndef __AIP3368H_DISPLAY_H__
#define __AIP3368H_DISPLAY_H__

#include "my_config.h"

#define AIP3368H_DISPLAY_TEST_ENABLE 1

enum
{
    AIP3368H_DISPLAY_FUEL_LEVEL_EMPTY = 0,
    AIP3368H_DISPLAY_FUEL_LEVEL_0,
    AIP3368H_DISPLAY_FUEL_LEVEL_1,
    AIP3368H_DISPLAY_FUEL_LEVEL_2,
    AIP3368H_DISPLAY_FUEL_LEVEL_3,
    AIP3368H_DISPLAY_FUEL_LEVEL_4,
};
typedef u8 aip3368h_display_fuel_level_t;

typedef struct
{
    u8 is_in_boot_animiation; // 是否处于启动动画中
} aip3368h_display_obj_t;
extern volatile aip3368h_display_obj_t aip3368h_display_obj;


void aip3368h_display_engine_speed_back_light(void);
void aip3368h_display_exclamation_point(u8 is_enable);
void aip3368h_display_engine_speed_scale_bar(u8 level);
void aip3368h_display_bat_err_icon(u8 is_enable);
void aip3368h_display_err_icon(u8 is_enable);
void aip3368h_display_fuel_level(aip3368h_display_fuel_level_t level);
void aip3368h_display_mileage_km_icon(u8 is_enable);
void aip3368h_display_mileage(u32 mileage, u8 is_displaying_total_mileage);
void aip3368h_display_speed_km_icon(u8 is_enable);
void aip3368h_display_speed(u8 speed);
void aip3368h_display_speed_scale_bar(u8 level);

// void aip3368h_display_boot_animation_1ms_isr(void);
void aip3368h_display_boot_animation_time_add(void);
void aip3368h_display_boot_animation_handle(void);

void aip3368h_display_err_handle_time_add(void);
void aip3368h_display_err_handle(void);

#if AIP3368H_DISPLAY_TEST_ENABLE
void aip3368h_display_test_engine_speed_scale_bar_1ms_isr(void);
void aip3368h_display_test_fuel_level_1ms_isr(void);
void aip3368h_display_test_mileage_1ms_isr(void);
void aip3368h_display_test_speed_1ms_isr(void);
void aip3368h_display_test_speed_scale_bar_1ms_isr(void);

void aip3368h_display_test(void);
#endif

#endif