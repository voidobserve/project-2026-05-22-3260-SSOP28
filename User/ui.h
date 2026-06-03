#ifndef __UI_H__
#define __UI_H__

#include "include.h"

#define UI_SETTING_TIME_OUT_CNT ((u16)2 * 12 * 400)
#define UI_SETTING_BLINK_PERIOD ((u16)400)

enum
{
	UI_STATE_NORMAL,
	UI_STATE_SETTING_DISTANCE_UNIT_TYPE,  // 设置 要显示的单位类型 km/h 或 mph
	UI_STATE_SETTING_WHEEL_CIRCUMFERENCE, // 设置 车轮的周长
};
typedef u8 ui_state_t;

typedef struct
{
	u16 blink_timer_cnt;
	u16 state_timeout_cnt;
	u16 auto_exit_setting_time_cnt; // 自动退出设置界面的计时器


	ui_state_t state;

} ui_manager_t;

extern volatile ui_manager_t ui_manager;

void ui_manager_init(void);
void ui_timer_handle_isr(void);

void ui_set_state(ui_state_t state);

void ui_display_handle(void);
void ui_display_refresh(void);

#endif