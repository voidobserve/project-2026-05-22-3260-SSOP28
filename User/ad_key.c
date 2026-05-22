#include "ad_key.h"

#if AD_KEY_ENABLE

// 自定义ad按键的键值:
// enum
// {
//     AD_KEY_ID_ONE_LEFT = 0x01,
//     AD_KEY_ID_TWO_LEFT,
//     AD_KEY_ID_THREE_LEFT,
//     AD_KEY_ID_ONE_RIGHT,
//     AD_KEY_ID_TWO_RIGHT,
//     AD_KEY_ID_THREE_RIGHT,
// };

// 定义按键扫描函数中，各个扫描状态：
// enum
// {
//     AD_KEY_SCAN_STATUS_NONE,                  // 空状态，检测是否有按键按下
//     AD_KEY_SCAN_STATUS_IS_DETECT_LOOG_PRESS,  // 正在检测是否为长按（要跟检测长按作区分）
//     AD_KEY_SCAN_STATUS_IS_HANDLE_LONG_PRESS,  // 正在处理长按
//     AD_KEY_SCAN_STATUS_IS_HANDLE_HOLD_PRESS,  // 正在处理长按持续（不松手）
//     AD_KEY_SCAN_STATUS_IS_HANDLE_SHORT_PRESS, // 正在处理短按

//     AD_KEY_SCAN_STATUS_IS_WAIT_SHORT_PRESS_RELEASE, // 正在等待短按松开
//     AD_KEY_SCAN_STATUS_IS_WAIT_LONG_PRESS_RELEASE,  // 正在等待长按松开
//     AD_KEY_SCAN_STATUS_IS_END,                      // 收尾处理
// };
// static volatile u8 ad_key_scan_status = 0; // 非阻塞的按键扫描函数中，使用的状态机

// 存放按键对应的ad值:
static const u16 ad_key_scan_table[][2] = {
    // [][0]按键对应的标号,在判断按键键值时使用   [][1]按键对应的ad值
    // {AD_KEY_ID_ONE_LEFT, AD_KEY_ONE_LEFT_VAL},
    // {AD_KEY_ID_TWO_LEFT, AD_KEY_TWO_LEFT_VAL},
    // {AD_KEY_ID_THREE_LEFT, AD_KEY_THREE_LEFT_VAL},
    // {AD_KEY_ID_ONE_RIGHT, AD_KEY_ONE_RIGHT_VAL},
    // {AD_KEY_ID_TWO_RIGHT, AD_KEY_TWO_RIGHT_VAL},
    // {AD_KEY_ID_THREE_RIGHT, AD_KEY_THREE_RIGHT_VAL},

    {AD_KEY_ID_1, AD_KEY_ID_1_VAL}, // 339
    {AD_KEY_ID_2, AD_KEY_ID_2_VAL}, // 1140
    {AD_KEY_ID_3, AD_KEY_ID_3_VAL}, // 1875
    {AD_KEY_ID_4, AD_KEY_ID_4_VAL}, // 2333
    {AD_KEY_ID_5, AD_KEY_ID_5_VAL}, // 3309
};

// 定义ad按键的按键事件
enum AD_KEY_EVENT
{
    AD_KEY_EVENT_NONE,
    AD_KEY_EVENT_ID_1_CLICK,
    AD_KEY_EVENT_ID_1_DOUBLE,
    AD_KEY_EVENT_ID_1_LONG,
    AD_KEY_EVENT_ID_1_HOLD,
    AD_KEY_EVENT_ID_1_LOOSE,

    AD_KEY_EVENT_ID_2_CLICK,
    AD_KEY_EVENT_ID_2_DOUBLE,
    AD_KEY_EVENT_ID_2_LONG,
    AD_KEY_EVENT_ID_2_HOLD,
    AD_KEY_EVENT_ID_2_LOOSE,

    AD_KEY_EVENT_ID_3_CLICK,
    AD_KEY_EVENT_ID_3_DOUBLE,
    AD_KEY_EVENT_ID_3_LONG,
    AD_KEY_EVENT_ID_3_HOLD,
    AD_KEY_EVENT_ID_3_LOOSE,

    AD_KEY_EVENT_ID_4_CLICK,
    AD_KEY_EVENT_ID_4_DOUBLE,
    AD_KEY_EVENT_ID_4_LONG,
    AD_KEY_EVENT_ID_4_HOLD,
    AD_KEY_EVENT_ID_4_LOOSE,

    AD_KEY_EVENT_ID_5_CLICK,
    AD_KEY_EVENT_ID_5_DOUBLE,
    AD_KEY_EVENT_ID_5_LONG,
    AD_KEY_EVENT_ID_5_HOLD,
    AD_KEY_EVENT_ID_5_LOOSE,
};

#define AD_KEY_EFFECT_EVENT_NUMS (5) // 单个ad按键的有效按键事件个数
// 将按键id和按键事件绑定起来，在 xx 函数中，通过查表的方式得到按键事件
static const u8 ad_key_event_table[][AD_KEY_EFFECT_EVENT_NUMS + 1] = {
    // [0]--按键对应的id号，用于查表，[1]、[2]、[3]...--用于与 key_driver.h 中定义的按键事件KEY_EVENT绑定关系(一定要一一对应)
    {AD_KEY_ID_1, AD_KEY_EVENT_ID_1_CLICK, AD_KEY_EVENT_ID_1_DOUBLE, AD_KEY_EVENT_ID_1_LONG, AD_KEY_EVENT_ID_1_HOLD, AD_KEY_EVENT_ID_1_LOOSE}, //
    {AD_KEY_ID_2, AD_KEY_EVENT_ID_2_CLICK, AD_KEY_EVENT_ID_2_DOUBLE, AD_KEY_EVENT_ID_2_LONG, AD_KEY_EVENT_ID_2_HOLD, AD_KEY_EVENT_ID_2_LOOSE}, //
    {AD_KEY_ID_3, AD_KEY_EVENT_ID_3_CLICK, AD_KEY_EVENT_ID_3_DOUBLE, AD_KEY_EVENT_ID_3_LONG, AD_KEY_EVENT_ID_3_HOLD, AD_KEY_EVENT_ID_3_LOOSE}, //
    {AD_KEY_ID_4, AD_KEY_EVENT_ID_4_CLICK, AD_KEY_EVENT_ID_4_DOUBLE, AD_KEY_EVENT_ID_4_LONG, AD_KEY_EVENT_ID_4_HOLD, AD_KEY_EVENT_ID_4_LOOSE}, //
    {AD_KEY_ID_5, AD_KEY_EVENT_ID_5_CLICK, AD_KEY_EVENT_ID_5_DOUBLE, AD_KEY_EVENT_ID_5_LONG, AD_KEY_EVENT_ID_5_HOLD, AD_KEY_EVENT_ID_5_LOOSE}, //
};

extern u8 ad_key_get_key_id(void);
// volatile key_driver_para_t ad_key_para = {
volatile struct key_driver_para ad_key_para = {
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

    AD_KEY_SCAN_CIRCLE_TIMES,
    0,
    // NO_KEY,
    0,

    0,
    0,
    3,

    AD_KEY_LONG_PRESS_TIME_THRESHOLD_MS / AD_KEY_SCAN_CIRCLE_TIMES,
    (AD_KEY_LONG_PRESS_TIME_THRESHOLD_MS + AD_KEY_HOLD_PRESS_TIME_THRESHOLD_MS) / AD_KEY_SCAN_CIRCLE_TIMES,
    0,

    0,
    0,
    // 200 / AD_KEY_SCAN_CIRCLE_TIMES,
    0,
    // NO_KEY,
    0,
    KEY_TYPE_AD,
    ad_key_get_key_id,

    AD_KEY_ID_NONE,
    KEY_EVENT_NONE,
};

// 将采集到的ad值转换成自定义的键值
static u16 __conver_cur_ad_to_ad_key(const u16 cur_ad_key)
{
    u8 i = 0;
    // u16 ad_key_id = AD_KEY_ID_NONE;
    u16 ad_key_id = NO_KEY;

    // ARRAY_SIZE(ad_key_scan_table) 这里是求出数组中存放的按键个数
    for (i = 0; i < ARRAY_SIZE(ad_key_scan_table); i++)
    {
        // if (cur_ad_key > (ad_key_scan_table[i][1] - AD_KEY_INTERVAL) &&
        //     cur_ad_key < (ad_key_scan_table[i][1] + AD_KEY_INTERVAL))
        // {
        //     // 如果之前未检测到按键，现在检测到按键按下
        //     ad_key_id = ad_key_scan_table[i][0]; // 获取ad值对应的键值
        //     break;
        // }

        if (cur_ad_key < ad_key_scan_table[i][1])
        {
            ad_key_id = ad_key_scan_table[i][0];
            break;
        }
    }

    return ad_key_id;
}

//
/**
 * @brief 将按键值和key_driver_scan得到的按键事件转换成ad按键的事件
 *
 * @param key_val ad按键键值
 * @param key_event 在key_driver_scan得到的按键事件 KEY_EVENT
 * @return u8 在ad_key_event_table中找到的对应的按键事件，如果没有则返回 AD_KEY_EVENT_NONE
 */
static u8 __ad_key_get_event(const u8 key_val, const u8 key_event)
{
    u8 ret_key_event = AD_KEY_EVENT_NONE;
    u8 i = 0;
    for (; i < ARRAY_SIZE(ad_key_event_table); i++)
    {
        if (key_val == ad_key_event_table[i][0])
        {
            // 如果往 KEY_EVENT 枚举中添加了新的按键事件，这里查表的方法就会失效，需要手动修改
            ret_key_event = ad_key_event_table[i][key_event];

            // if (KEY_EVENT_CLICK == key_event)
            // {
            //     ret_key_event = ad_key_event_table[i][1];
            // }
            // else if (KEY_EVENT_DOUBLE_CLICK == key_event)
            // {
            //     ret_key_event = ad_key_event_table[i][2];
            // }

            break;
        }
    }

    return ret_key_event;
}

u8 ad_key_get_key_id(void)
{
    volatile u16 ad_key_id = 0;       // 单次按键标志
    adc_sel_pin(ADC_PIN_KEY);         // 内部至少占用1ms
    ad_key_id = adc_single_convert(); // 直接用单次转换,不取平均值,防止识别不到按键
    // printf("ad key val %u \n", ad_key_id);
    // printf("ad_key_id val %u\n", ad_key_id);
    ad_key_id = __conver_cur_ad_to_ad_key(ad_key_id); // 将采集到的ad值转换成自定义的键值

    // 测试用：
    // if (ad_key_id != 0)
    // {
    //     printf("ad key id %u\n", ad_key_id);
    // }

    return ad_key_id;
}

void ad_key_handle(void)
{
    u8 ad_key_event = AD_KEY_EVENT_NONE;

    if (ad_key_para.latest_key_val == AD_KEY_ID_NONE)
    {
        return;
    }

    ad_key_event = __ad_key_get_event(ad_key_para.latest_key_val, ad_key_para.latest_key_event);
    ad_key_para.latest_key_val = AD_KEY_ID_NONE;
    ad_key_para.latest_key_event = KEY_EVENT_NONE;

    switch (ad_key_event)
    {
#if 1

    case AD_KEY_EVENT_ID_1_CLICK:
        // printf("key 1 click\n");
        send_data(SEND_TOUCH_KEY_STATUS, (u32)AD_KEY_CLICK_MSG << 16 | KEY_LEFT_PRE);
        break;

    case AD_KEY_EVENT_ID_1_DOUBLE:
        // printf("key 1 double\n");
        break;

    case AD_KEY_EVENT_ID_1_LONG:
        // printf("key 1 long\n");
        break;

    case AD_KEY_EVENT_ID_1_HOLD:
        // printf("key 1 hold\n");
        send_data(SEND_TOUCH_KEY_STATUS, (u32)AD_KEY_LONG_MSG << 16 | KEY_LEFT_PRE);
        break;

    case AD_KEY_EVENT_ID_1_LOOSE:
        // printf("key 1 loose\n");
        break;
    case AD_KEY_EVENT_ID_2_CLICK:
        // printf("key 2 click\n");
        send_data(SEND_TOUCH_KEY_STATUS, (u32)AD_KEY_CLICK_MSG << 16 | KEY_DOWN_VOL_DOWN);
        break;

    case AD_KEY_EVENT_ID_2_DOUBLE:
        // printf("key 2 double\n");
        break;

    case AD_KEY_EVENT_ID_2_LONG:
        // printf("key 2 long\n");
        break;

    case AD_KEY_EVENT_ID_2_HOLD:
        // printf("key 2 hold\n");
        send_data(SEND_TOUCH_KEY_STATUS, (u32)AD_KEY_LONG_MSG << 16 | KEY_DOWN_VOL_DOWN);
        break;

    case AD_KEY_EVENT_ID_2_LOOSE:
        // printf("key 2 loose\n");
        break;
    case AD_KEY_EVENT_ID_3_CLICK:
        // printf("key 3 click\n");

        send_data(SEND_TOUCH_KEY_STATUS, (u32)AD_KEY_CLICK_MSG << 16 | KEY_UP_VOL_UP);
        break;

    case AD_KEY_EVENT_ID_3_DOUBLE:
        // printf("key 3 double\n");
        break;

    case AD_KEY_EVENT_ID_3_LONG:
        // printf("key 3 long\n");
        break;

    case AD_KEY_EVENT_ID_3_HOLD:
        // printf("key 3 hold\n");
        send_data(SEND_TOUCH_KEY_STATUS, (u32)AD_KEY_LONG_MSG << 16 | KEY_UP_VOL_UP);
        break;

    case AD_KEY_EVENT_ID_3_LOOSE:
        // printf("key 3 loose\n");
        break;
    case AD_KEY_EVENT_ID_4_CLICK:
        // printf("key 4 click\n");
        send_data(SEND_TOUCH_KEY_STATUS, (u32)AD_KEY_CLICK_MSG << 16 | KEY_ENTER_MENU);
        break;

    case AD_KEY_EVENT_ID_4_DOUBLE:
        // printf("key 4 double\n");
        break;

    case AD_KEY_EVENT_ID_4_LONG:
        // printf("key 4 long\n");
        break;

    case AD_KEY_EVENT_ID_4_HOLD:
        // printf("key 4 hold\n");
        send_data(SEND_TOUCH_KEY_STATUS, (u32)AD_KEY_LONG_MSG << 16 | KEY_ENTER_MENU);
        break;

    case AD_KEY_EVENT_ID_4_LOOSE:
        // printf("key 4 loose\n");
        break;
    case AD_KEY_EVENT_ID_5_CLICK:
        // printf("key 5 click\n");
        send_data(SEND_TOUCH_KEY_STATUS, (u32)AD_KEY_CLICK_MSG << 16 | KEY_RIGHT_NEXT);
        break;

    case AD_KEY_EVENT_ID_5_DOUBLE:
        // printf("key 5 double\n");
        break;

    case AD_KEY_EVENT_ID_5_LONG:
        // printf("key 5 long\n");
        break;

    case AD_KEY_EVENT_ID_5_HOLD:
        // printf("key 5 hold\n");
        send_data(SEND_TOUCH_KEY_STATUS, (u32)AD_KEY_LONG_MSG << 16 | KEY_RIGHT_NEXT);
        break;

    case AD_KEY_EVENT_ID_5_LOOSE:
        // printf("key 5 loose\n");
        break;

    default:
        break;

#endif
    }
}

#endif//  AD_KEY_ENABLE
