#include "io_key.h"

extern u8 io_key_get_key_id(void);
// volatile key_driver_para_t ad_key_para = {
volatile struct key_driver_para io_key_para = {
	// 编译器不支持指定成员赋值的写法，会报错
	IO_KEY_SCAN_CIRCLE_TIMES,
	0,
	// NO_KEY,
	0,

	0,
	0,
	3,

	IO_KEY_LONG_PRESS_TIME_THRESHOLD_MS / IO_KEY_SCAN_CIRCLE_TIMES,
	(IO_KEY_LONG_PRESS_TIME_THRESHOLD_MS + IO_KEY_HOLD_PRESS_TIME_THRESHOLD_MS) / IO_KEY_SCAN_CIRCLE_TIMES,
	0,

	0,
	0,
	// 200 / IO_KEY_SCAN_CIRCLE_TIMES,
	0,
	// NO_KEY,
	0,
	KEY_TYPE_TOUCH, // 触摸按键，实际上是检测触摸ic传递过来的电平信号，可以当作io来检测
	io_key_get_key_id,

	IO_KEY_ID_NONE,
	KEY_EVENT_NONE,
};

#define IO_KEY_EFFECT_EVENT_NUMS 4
static const u8 io_key_event_table[][IO_KEY_EFFECT_EVENT_NUMS + 1] = {
	IO_KEY_ID_VALID,
	IO_KEY_EVENT_CLICK,
	IO_KEY_EVENT_LONG,
	IO_KEY_EVENT_HOLD,
	IO_KEY_EVENT_LOOSE,
};

void io_key_config(void)
{
	P0_PU |= GPIO_P05_PULL_UP(0x01);	  // 上拉
	P0_MD1 &= ~(GPIO_P05_MODE_SEL(0x03)); // 输入模式
}

u8 io_key_get_key_id(void)
{
	if (IO_KEY_PIN == 0)
	{
		// 引脚配置为输入上拉，低电平表示按键按下
		return IO_KEY_ID_VALID;
	}

	return NO_KEY;
}

u8 __io_key_get_event__(u8 key_val, u8 key_event)
{
	u8 ret_key_event = IO_KEY_EVENT_NONE;
	u8 i = 0;
	u8 index = 0;

	if (key_event == KEY_EVENT_CLICK)
	{
		index = 1;
	}
	else if (key_event == KEY_EVENT_LONG)
	{
		index = 2;
	}
	else if (key_event == KEY_EVENT_HOLD)
	{
		index = 3;
	}
	else if (key_event == KEY_EVENT_UP)
	{
		index = 4;
	}

	for (; i < ARRAY_SIZE(io_key_event_table); i++)
	{
		if (key_val == io_key_event_table[i][0])
		{
			ret_key_event = io_key_event_table[i][index];
			break;
		}
	}

	return ret_key_event;
}

void io_key_handle(void)
{
	u8 io_key_event = IO_KEY_EVENT_NONE;

	if (io_key_para.latest_key_val == IO_KEY_ID_NONE)
	{
		return;
	}

	io_key_event = __io_key_get_event__(io_key_para.latest_key_val, io_key_para.latest_key_event);
	io_key_para.latest_key_val = IO_KEY_ID_NONE;
	io_key_para.latest_key_event = KEY_EVENT_NONE;
 
	switch (io_key_event)
	{
	case KEY_EVENT_CLICK:
#if USER_DEBUG_ENABLE
		// printf("click\n");
#endif

		beep_play(117);

		if (UI_STATE_NORMAL == ui_manager.state)
		{
			instrument.save_info.is_display_total_mileage =
				!instrument.save_info.is_display_total_mileage;
			
			instrument_info_save();
		}
		else if (UI_STATE_SETTING_DISTANCE_UNIT_TYPE ==
				 ui_manager.state)
		{
			// 如果正在设置当前要显示的单位类型，km/h 或 mph

			// 切换 单位类型
			if (DISTANCE_UNIT_TYPE_METRIC ==
				instrument.save_info.distance_unit_type)
			{
				instrument.save_info.distance_unit_type =
					DISTANCE_UNIT_TYPE_IMPERIAL;
			}
			else
			{
				instrument.save_info.distance_unit_type =
					DISTANCE_UNIT_TYPE_METRIC;
			}
		}
		else if (UI_STATE_SETTING_WHEEL_CIRCUMFERENCE ==
				 ui_manager.state)
		{
			// 如果正在设置车轮周长

			instrument.save_info.whell_circumference += 5;
			if (instrument.save_info.whell_circumference > 180)
			{
				instrument.save_info.whell_circumference = 50;
			} 
		}

		ui_display_refresh();
		break;
	case IO_KEY_EVENT_LONG:
#if USER_DEBUG_ENABLE
		// printf("Long\n");
#endif
		beep_play(117);

		if (UI_STATE_NORMAL == ui_manager.state)
		{
			// 如果正在显示里程
			if (1 == instrument.save_info.is_display_total_mileage)
			{
				/*
					如果显示的是 TOTAL 里程，切换到设置要显示的单位类型
				*/
				ui_set_state(UI_STATE_SETTING_DISTANCE_UNIT_TYPE);
			}
			else
			{
				// 如果显示的是 TRIP 里程，清空它
				instrument.save_info.subtotal_mileage = 0; 
				instrument_info_save();
			} 
		}
		else if (UI_STATE_SETTING_DISTANCE_UNIT_TYPE ==
				 ui_manager.state)
		{
			// 从 设置单位类型 -> 设置车轮周长
			ui_manager.state = UI_STATE_SETTING_WHEEL_CIRCUMFERENCE; 
			instrument_info_save(); // 退出单位类型设置后，保存
		}
		else if (UI_STATE_SETTING_WHEEL_CIRCUMFERENCE ==
				 ui_manager.state)
		{
			// 从 设置车轮周长 -> 正常显示
			ui_manager.state = UI_STATE_NORMAL; 
			instrument_info_save(); // 退出 车轮周长设置 后，保存
		}

		ui_display_refresh();

		break;

	default:
		break;
	}
}
