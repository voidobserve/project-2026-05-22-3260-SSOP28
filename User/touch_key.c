#include "touch_key.h"

#if TOUCH_KEY_ENABLE
/*
    标志位，触摸按键的扫描周期是否到来
    由定时器中断置位，扫描函数中判断该标志位的状态，再决定是否要进行按键扫描
*/
// volatile bit flag_is_touch_key_scan_circle_arrived;

// 定义触摸按键的按键事件
enum TOUCH_KEY_EVENT
{
    TOUCH_KEY_EVENT_NONE = 0,
    TOUCH_KEY_EVENT_ID_1_CLICK,
    TOUCH_KEY_EVENT_ID_1_DOUBLE,
    TOUCH_KEY_EVENT_ID_1_LONG,
    TOUCH_KEY_EVENT_ID_1_HOLD,
    TOUCH_KEY_EVENT_ID_1_LOOSE,

    TOUCH_KEY_EVENT_ID_2_CLICK,
    TOUCH_KEY_EVENT_ID_2_DOUBLE,
    TOUCH_KEY_EVENT_ID_2_LONG,
    TOUCH_KEY_EVENT_ID_2_HOLD,
    TOUCH_KEY_EVENT_ID_2_LOOSE,
};

#define TOUCH_KEY_EFFECT_EVENT_NUMS (5) // 单个触摸按键的有效按键事件个数
// 将按键id和按键事件绑定起来，在 xx 函数中，通过查表的方式得到按键事件
static const u8 touch_key_event_table[][TOUCH_KEY_EFFECT_EVENT_NUMS + 1] = {
    // [0]--按键对应的id号，用于查表，[1]、[2]、[3]...--用于与 key_driver.h 中定义的按键事件KEY_EVENT绑定关系(一定要一一对应)
    {TOUCH_KEY_ID_1, TOUCH_KEY_EVENT_ID_1_CLICK, TOUCH_KEY_EVENT_ID_1_DOUBLE, TOUCH_KEY_EVENT_ID_1_LONG, TOUCH_KEY_EVENT_ID_1_HOLD, TOUCH_KEY_EVENT_ID_1_LOOSE}, //
    {TOUCH_KEY_ID_2, TOUCH_KEY_EVENT_ID_2_CLICK, TOUCH_KEY_EVENT_ID_2_DOUBLE, TOUCH_KEY_EVENT_ID_2_LONG, TOUCH_KEY_EVENT_ID_2_HOLD, TOUCH_KEY_EVENT_ID_2_LOOSE}, //
};

extern u8 touch_key_get_key_id(void);
volatile struct key_driver_para touch_key_para = {
    // 编译器不支持指定成员赋值的写法，会报错：
    // .scan_times = 10,   // 扫描频率，单位：ms
    // .last_key = NO_KEY, // 上一次得到的按键键值，初始化为无效的键值
    // // .filter_value = NO_KEY, // 按键消抖期间得到的键值(在key_driver_scan()函数中使用)，初始化为 NO_KEY
    // // .filter_cnt = 0, // 按键消抖期间的累加值(在key_driver_scan()函数中使用)，初始化为0
    // .filter_time = 3,       // 按键消抖次数，与扫描频率有关
    // .long_time = 50,        // 判定按键是长按对应的数量，与扫描频率有关
    // .hold_time = (75 + 15), // 判定按键是HOLD对应的数量，与扫描频率有关
    // // .press_cnt = 0, // 与long_time和hold_time对比, 判断长按事件和HOLD事件
    // // .click_cnt = 0,
    // .click_delay_time = 20, // 按键抬起后，等待连击的数量，与扫描频率有关
    // // .notify_value = 0,
    // .key_type = KEY_TYPE_AD, // 按键类型为ad按键
    // .get_value = ad_key_get_key_id,

    // .latest_key_val = AD_KEY_ID_NONE,
    // .latest_key_event = KEY_EVENT_NONE,

    TOUCH_KEY_SCAN_CIRCLE_TIMES,
    0,
    // NO_KEY,
    0,

    0,
    0,
    3,

    TOUCH_KEY_LONG_PRESS_TIME_THRESHOLD_MS / TOUCH_KEY_SCAN_CIRCLE_TIMES,
    (TOUCH_KEY_LONG_PRESS_TIME_THRESHOLD_MS + TOUCH_KEY_HOLD_PRESS_TIME_THRESHOLD_MS) / TOUCH_KEY_SCAN_CIRCLE_TIMES,
    0,

    0,
    0,
    // 200 / TOUCH_KEY_SCAN_CIRCLE_TIMES,
    0,
    // NO_KEY,
    0,
    KEY_TYPE_TOUCH,
    touch_key_get_key_id,

    TOUCH_KEY_ID_NONE,
    KEY_EVENT_NONE,
};

static u8 touch_key_get_key_id(void)
{
    unsigned long tk_key_val = 0;
    u8 ret = NO_KEY;

    // delay_ms(1); // 延时并不能解决刚上点就检测到长按的问题

    // printf("touch key scan\n");

    tk_key_val = __tk_key_flag;
#if 1
    if (TK_CH9_VALIB == __tk_key_flag)
    {
        ret = TOUCH_KEY_ID_1;
        // return TOUCH_KEY_ID_1;
    }
    else if (TK_CH10_VALIB == __tk_key_flag)
    {
        ret = TOUCH_KEY_ID_2;
        // return TOUCH_KEY_ID_2;
    }
    else
    {
        // return TOUCH_KEY_ID_NONE;
        // return NO_KEY;
    }
#endif

#if 0
    if (TK_CH10_VALIB == __tk_key_flag)
    {
        ret = TOUCH_KEY_ID_2;
        // return TOUCH_KEY_ID_1;
    }
    else if (TK_CH9_VALIB == __tk_key_flag)
    {
        ret = TOUCH_KEY_ID_1;
        // return TOUCH_KEY_ID_2;
    }
    else
    {
        // return TOUCH_KEY_ID_NONE;
        // return NO_KEY;
    }
#endif

    tk_key_val = 0;

    // return __tk_key_flag;        // __tk_key_flag 单次按键标志
    return ret;
}

/**
 * @brief 将按键值和key_driver_scan得到的按键事件转换成触摸按键的事件
 *
 * @param key_val 触摸按键键值
 * @param key_event 在key_driver_scan得到的按键事件 KEY_EVENT
 * @return u8 在touch_key_event_table中找到的对应的按键事件，如果没有则返回 TOUCH_KEY_EVENT_NONE
 */
static u8 __touch_key_get_event(const u8 key_val, const u8 key_event)
{
    volatile u8 ret_key_event = TOUCH_KEY_EVENT_NONE;
    u8 i = 0;
    for (; i < ARRAY_SIZE(touch_key_event_table); i++)
    {
        // 如果往 KEY_EVENT 枚举中添加了新的按键事件，这里查表的方法就会失效，需要手动修改
        if (key_val == touch_key_event_table[i][0])
        {
            ret_key_event = touch_key_event_table[i][key_event];
            break;
        }
    }

    return ret_key_event;
}

void touch_key_handle(void)
{
    u8 touch_key_event = TOUCH_KEY_EVENT_NONE;

    if (touch_key_para.latest_key_val == TOUCH_KEY_ID_NONE)
    {
        return;
    }

    touch_key_event = __touch_key_get_event(touch_key_para.latest_key_val, touch_key_para.latest_key_event);
    touch_key_para.latest_key_val = TOUCH_KEY_ID_NONE;
    touch_key_para.latest_key_event = KEY_EVENT_NONE;

    switch (touch_key_event)
    {
    case TOUCH_KEY_EVENT_ID_1_CLICK:
        // printf("touch key 1 click\n");
        send_data(SEND_TOUCH_KEY_STATUS, (u32)TOUCH_KEY_CLICK_MSG << 16 | TOUCH_KEY_SEND_INFO_1); // DEBUG 测试时屏蔽，应该恢复

        // DEBUG
        // if (fuel_adc_val < 4095 - 100)
        // {
        //     fuel_adc_val += 100;
        // }

        break;

    case TOUCH_KEY_EVENT_ID_1_DOUBLE:
        // printf("touch key 1 double\n");
        break;

    case TOUCH_KEY_EVENT_ID_1_LONG:
        // printf("touch key 1 long\n");
        send_data(SEND_TOUCH_KEY_STATUS, (u32)TOUCH_KEY_LONG_MSG << 16 | TOUCH_KEY_SEND_INFO_1);
        break;

    case TOUCH_KEY_EVENT_ID_1_HOLD:
        // printf("touch key 1 hold\n");
        send_data(SEND_TOUCH_KEY_STATUS, (u32)TOUCH_KEY_HOLD_MSG << 16 | TOUCH_KEY_SEND_INFO_1);
        break;

    case TOUCH_KEY_EVENT_ID_1_LOOSE:
        // printf("touch key 1 loose\n");
        send_data(SEND_TOUCH_KEY_STATUS, (u32)TOUCH_KEY_LOOSE_MSG << 16 | TOUCH_KEY_SEND_INFO_1);
        break;
    case TOUCH_KEY_EVENT_ID_2_CLICK:
        // printf("touch key 2 click\n");
        send_data(SEND_TOUCH_KEY_STATUS, (u32)TOUCH_KEY_CLICK_MSG << 16 | TOUCH_KEY_SEND_INFO_2); // DEBUG 测试时屏蔽，应该恢复

        // DEBUG
        // if (fuel_adc_val > 100)
        // {
        //     fuel_adc_val -= 100;
        // }
        break;

    case TOUCH_KEY_EVENT_ID_2_DOUBLE:
        // printf("touch key 2 double\n");
        break;

    case TOUCH_KEY_EVENT_ID_2_LONG:
        // printf("touch key 2 long\n");
        send_data(SEND_TOUCH_KEY_STATUS, (u32)TOUCH_KEY_LONG_MSG << 16 | TOUCH_KEY_SEND_INFO_2);
        break;

    case TOUCH_KEY_EVENT_ID_2_HOLD:
        // printf("touch key 2 hold\n");
        send_data(SEND_TOUCH_KEY_STATUS, (u32)TOUCH_KEY_HOLD_MSG << 16 | TOUCH_KEY_SEND_INFO_2);
        break;

    case TOUCH_KEY_EVENT_ID_2_LOOSE:
        // printf("touch key 2 loose\n");
        send_data(SEND_TOUCH_KEY_STATUS, (u32)TOUCH_KEY_LOOSE_MSG << 16 | TOUCH_KEY_SEND_INFO_2);
        break;

    default:
        break;
    }
}

#endif // (TOUCH_KEY_ENABLE)
