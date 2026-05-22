#ifndef __TOUCH_KEY_H
#define __TOUCH_KEY_H

#include "my_config.h"

#if TOUCH_KEY_ENABLE
/*
    触摸按键的扫描周期，单位：ms
    用在定时器中断，注意不能超过变量的最大值
*/
#define TOUCH_KEY_SCAN_CIRCLE_TIMES (10)
// #define TOUCH_KEY_SCAN_CIRCLE_TIMES (20)
// #define TOUCH_KEY_SCAN_CIRCLE_TIMES (30)
// #define TOUCH_KEY_SCAN_CIRCLE_TIMES (50) // 加上外壳，会连短按也检测不到
#define TOUCH_KEY_FILTER_TIMES (3)         // 触摸按键消抖次数
// #define DETECT_DOUBLE_CLICK_INTERVAL (150) // 检测双击的时间间隔(单位：ms)
#define TOUCH_KEY_LONG_PRESS_TIME_THRESHOLD_MS (1000) // 长按时间阈值（单位：ms）
// #define HOLD_PRESS_TIME_THRESHOLD_MS (25) // 长按持续(不松手)的时间阈值(单位：ms)，每隔 xx 时间认为有一次长按持续事件
#define TOUCH_KEY_HOLD_PRESS_TIME_THRESHOLD_MS (750) // 长按持续(不松手)的时间阈值(单位：ms)，每隔 xx 时间认为有一次长按持续事件
// #define LOOSE_PRESS_CNT_MS (30)            // 松手计时，松开手多久，才认为是真的松手了

#define TOUCH_KEY_CLICK_MSG ((u8)0x00) // 按键短按时，要发送的按键状态数据
#define TOUCH_KEY_LONG_MSG ((u8)0x01)  // 检测到按键长按时，要发送的按键状态数据
#define TOUCH_KEY_HOLD_MSG ((u8)0x02)  // 检测到按键长按一直未松手时，要发送的按键状态数据
#define TOUCH_KEY_LOOSE_MSG ((u8)0x03) // 按键长按后松手时，要发送的按键状态数据

// 定义待发送的、触摸按键对应的数据
enum
{
    TOUCH_KEY_SEND_INFO_1 = 0x47,

    TOUCH_KEY_SEND_INFO_2 = 0x48,
};

enum TOUCH_KEY_ID
{
    TOUCH_KEY_ID_NONE = 0x00,
    TOUCH_KEY_ID_1,
    TOUCH_KEY_ID_2,
};

extern volatile struct key_driver_para touch_key_para;

/*
    标志位，触摸按键的扫描周期是否到来
    由定时器中断置位，扫描函数中判断该标志位的状态，再决定是否要进行按键扫描
*/
// extern volatile bit flag_is_touch_key_scan_circle_arrived;
// extern void touch_key_scan(void);

extern void touch_key_handle(void);

#endif // TOUCH_KEY_ENABLE

#endif
