#include "ui.h"
#include "my_config.h"

volatile ui_manager_t ui_manager;

void ui_manager_init(void)
{
	ui_manager.state = UI_STATE_NORMAL;

	ui_display_refresh();
}

/**
 * @brief ui定时器处理
 *
 * @attention 在定时器调用，负责所有ui相关的计数器处理
 *
 */
void ui_timer_handle_isr(void)
{
	aip3368h_refresh_time_add(); // 控制将显存数据刷新到屏幕驱动ic的周期
	// 递增 AIP3368H 显示 速度 刷新时间计数
	aip3368h_display_speed_refresh_time_add();
	// 递增 AIP3368H 显示 发动机转速 刷新时间计数
	aip3368h_display_engine_speed_refresh_time_add();
	// 控制设置界面对应的设置项目以一定周期进行闪烁
	// aip3368h_display_setting_item_time_add();

	if (ui_manager.state == UI_STATE_SETTING_DISTANCE_UNIT_TYPE ||
		ui_manager.state == UI_STATE_SETTING_WHEEL_CIRCUMFERENCE)
	{
		if (ui_manager.blink_timer_cnt < ((u16)-1))
		{
			ui_manager.blink_timer_cnt++;
		}

		if (ui_manager.auto_exit_setting_time_cnt < ((u16)-1))
		{
			ui_manager.auto_exit_setting_time_cnt++;
		}
	}
	else
	{
		ui_manager.blink_timer_cnt = 0;
		ui_manager.auto_exit_setting_time_cnt = 0;
	}
}

// 设置ui状态（切换ui）
void ui_set_state(ui_state_t state)
{
	ui_manager.state = state;

	// switch (state)
	// {
	// case UI_STATE_NORMAL:
	// 	// 正常显示

	// 	break;
	// case UI_STATE_SETTING_DISTANCE_UNIT_TYPE:
	// 	// 设置 要显示的单位类型 km/h 或 mph

	// 	break;
	// case UI_STATE_SETTING_WHEEL_CIRCUMFERENCE:
	// 	// 设置 车轮的周长
	// 	break;
	// }

	// ui_display_refresh();
}

// 显示处理
void ui_display_handle(void)
{
	switch (ui_manager.state)
	{
	case UI_STATE_NORMAL:
		// 正常显示
		break;
	case UI_STATE_SETTING_DISTANCE_UNIT_TYPE:
		// 设置 要显示的单位类型 km/h 或 mph

		if (ui_manager.blink_timer_cnt >= UI_SETTING_BLINK_PERIOD)
		{
			ui_manager.blink_timer_cnt = 0;

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
		}

		break;
	case UI_STATE_SETTING_WHEEL_CIRCUMFERENCE:
		// 设置 车轮的周长

		if (ui_manager.blink_timer_cnt >= UI_SETTING_BLINK_PERIOD)
		{
			ui_manager.blink_timer_cnt = 0;

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
				aip3368h_display_speed(
					instrument.save_info.whell_circumference);
			}
		}

		break;
	}

	if (ui_manager.auto_exit_setting_time_cnt >= UI_SETTING_TIME_OUT_CNT)
	{
		ui_manager.auto_exit_setting_time_cnt = 0;
		ui_set_state(UI_STATE_NORMAL);
		ui_display_refresh();

		// 自动退出设置界面后，保存相关数据
		instrument_info_save(); 
	}

	aip3368h_display_speed_handle();		// 显示时速
	aip3368h_display_mileage_handle();		// 显示里程
	aip3368h_display_engine_speed_handle(); // 显示发动机转速
	aip3368h_module_display();
}

/**
 * @brief
 * 		清空所有计时
 * 		立即显示内容
 *
 */
void ui_display_refresh(void)
{
	ui_manager.blink_timer_cnt = 0;
	ui_manager.auto_exit_setting_time_cnt = 0;

	// aip3368h_display_speed_refresh(); // 显示时速
	// aip3368h_display_mileage_refresh(); // 显示里程

	// aip3368h_module_display();

	switch (ui_manager.state)
	{
	case UI_STATE_NORMAL:
		// 正常显示

		// 显示时速
		aip3368h_display_speed_by_unit_type(instrument.speed_of_lag);

		break;
	case UI_STATE_SETTING_DISTANCE_UNIT_TYPE:
		// 设置 要显示的单位类型 km/h 或 mph

		break;
	case UI_STATE_SETTING_WHEEL_CIRCUMFERENCE:
		// 设置 车轮的周长
		aip3368h_display_speed(instrument.save_info.whell_circumference);
		break;
	}

	aip3368h_display_mileage_refresh(); // 立即显示里程

	// 立即显示单位类型
	if (DISTANCE_UNIT_TYPE_METRIC ==
		instrument.save_info.distance_unit_type)
	{
		aip3368h_display_speed_unit_type(DISTANCE_UNIT_TYPE_METRIC);
		aip3368h_display_mileage_unit_type(DISTANCE_UNIT_TYPE_METRIC);
	}
	else
	{
		aip3368h_display_speed_unit_type(DISTANCE_UNIT_TYPE_IMPERIAL);
		aip3368h_display_mileage_unit_type(DISTANCE_UNIT_TYPE_IMPERIAL);
	}
}
