#include "speed_scan.h"

#if SPEED_SCAN_ENABLE

static volatile bit flag_is_speed_scan_over_time = 0; // 速度检测是否一直没有脉冲到来，导致超时

static volatile u32 speed_pulse_cnt = 0;    // 记录脉冲个数，在定时器中断累加
static volatile u32 speed_scan_time_ms = 0; // 记录扫描时间，在定时器中断累加

static volatile u32 cur_speed_scan_time = 0;  // 当前检测时速所用时间
static volatile u32 cur_speed_scan_pulse = 0; // 当前检测时速所用时间内检测到的脉冲个数

// 计数器，计数满一段时间后，更新显示
static volatile u8 aip3368h_display_speed_refresh_time_cnt = 0;

// 滑动平局数组
#define SPEED_FILTER_ARRAY_SIZE (40)
// #define SPEED_FILTER_ARRAY_SIZE (20)
static volatile u8 speed_filter_array[SPEED_FILTER_ARRAY_SIZE] = {0};
static volatile u8 speed_filter_index = 0;

void speed_filter_init(u8 speed)
{
    u8 i;
    for (i = 0; i < SPEED_FILTER_ARRAY_SIZE; i++)
    {
        speed_filter_array[i] = speed;
    }

    speed_filter_index = 0;
}

void speed_filter_add(u8 speed)
{
    speed_filter_array[speed_filter_index] = speed;
    speed_filter_index++;
    if (speed_filter_index >= SPEED_FILTER_ARRAY_SIZE)
    {
        speed_filter_index = 0;
    }
}

u8 speed_filter_get_speed(void)
{
#if 1
    u8 i;
    u32 sum = 0;
    u8 max_speed = 0;
    u8 min_speed = (u8)-1;

    // for (i = 0; i < SPEED_FILTER_ARRAY_SIZE; i++)
    // {
    //     printf("speed[%u] == %u\n", (u16)i, (u16)speed_filter_array[i]);
    // }
    // printf("\n===================\n");

    for (i = 0; i < SPEED_FILTER_ARRAY_SIZE; i++)
    {
        sum += speed_filter_array[i];

        if (max_speed < speed_filter_array[i])
        {
            max_speed = speed_filter_array[i];
        }

        if (min_speed > speed_filter_array[i])
        {
            min_speed = speed_filter_array[i];
        }
    }

    sum -= (max_speed + min_speed);

    return sum / (SPEED_FILTER_ARRAY_SIZE - 2);
#endif

    // u16 i;
    // u8 count[256] = {0}; // 速度值范围是 0-255，索引值对应这个速度值
    // u8 max_count = 0;
    // u8 max_count_val = 0; // 记录出现次数最多的数值

    // // 统计每个值出现的次数
    // for (i = 0; i < SPEED_FILTER_ARRAY_SIZE; i++)
    // {
    //     count[speed_filter_array[i]]++;

    //     printf("speed[%u] == %u\n", (u16)i, (u16)speed_filter_array[i]);
    // }
    // printf("\n===================\n");

    // // 找出出现次数最多的值
    // for (i = 0; i < 256; i++) // 0 ~ 255
    // {
    //     if (count[i] > max_count)
    //     {
    //         max_count = count[i];
    //         max_count_val = i;
    //     }
    // }

    // return max_count_val;
}

// 时速扫描的配置
void speed_scan_config(void)
{
    // 使用定时器扫描IO电平变化来计算脉冲
    P1_MD1 &= ~GPIO_P15_MODE_SEL(0x3); // 输入模式
    P1_PU |= GPIO_P15_PULL_UP(0x1);    // 配置为上拉
}

void speed_scan_update_data(void) // 更新检测时速的数据
{
    cur_speed_scan_time += speed_scan_time_ms;
    speed_scan_time_ms = 0;
    cur_speed_scan_pulse += speed_pulse_cnt;
    speed_pulse_cnt = 0;
}

void speed_scan_timer_50us_isr(void)
{
    static volatile bit last_speed_scan_level = 0; // 记录上一次检测到的引脚电平（时速检测脚）
    static volatile u16 cnt = 0;                   // 记录时速扫描的时间
    cnt++;
    if (cnt >= 20) // 每1ms进入一次
    {
        cnt = 0;
        // 更新扫描时间（如果一直没有脉冲到来给它清零，这里会一直累加）
        speed_scan_time_ms++;

        if (speed_scan_time_ms >= SPEED_SCAN_OVER_TIME &&
            flag_is_speed_scan_over_time == 0)
        {
            speed_scan_time_ms = 0;
            flag_is_speed_scan_over_time = 1; // 说明超时，脉冲计数一直没有加一
        }
    }

    if (SPEED_SCAN_PIN) // 检测时速的引脚
    {
        if (0 == last_speed_scan_level)
        {
            speed_pulse_cnt++;
            speed_scan_update_data();
        }

        last_speed_scan_level = 1;
    }
    else
    {
        // 如果现在检测到低电平
        last_speed_scan_level = 0;
    }
}

// void speed_scan_pulse_50us_isr(void)
// {
//     static volatile bit last_speed_scan_level = 0; // 记录上一次检测到的引脚电平（时速检测脚）

//     static volatile u32 speed_pulse_cnt = 0;
//     static volatile u8 cnt = 0;
//     static volatile u16 speed_scan_time_cnt = 0;
//     static volatile u16 speed_scan_over_time_cnt = 0;

//     if (SPEED_SCAN_PIN) // 检测时速的引脚
//     {
//         if (0 == last_speed_scan_level)
//         {
//             // 每有一个上升沿，计数++
//             speed_pulse_cnt++;
//         }

//         last_speed_scan_level = 1;
//     }
//     else
//     {
//         // 如果现在检测到低电平
//         last_speed_scan_level = 0;
//     }

//     cnt++;
//     if (cnt >= 20)
//     {
//         cnt = 0;

//         speed_scan_time_cnt++;
//         if (speed_pulse_cnt !=0 && speed_scan_time_cnt >= SPEED_SCAN_UPDATE_TIME)
//         {
//             cur_speed_scan_time += speed_scan_time_cnt;
//             speed_scan_time_cnt = 0;
//             cur_speed_scan_pulse += speed_pulse_cnt;
//             speed_pulse_cnt = 0;
//         }
//         // else if (speed_scan_time_cnt >= SPEED_SCAN_OVER_TIME)
//         // {

//         // }

//         if (cur_speed_scan_pulse == 0)
//         {
//             cur_speed_scan_time = 0;
//             speed_scan_over_time_cnt++;
//             if (speed_scan_over_time_cnt >= SPEED_SCAN_OVER_TIME)
//             {
//                 speed_scan_over_time_cnt = 0;
//                 flag_is_speed_scan_over_time = 1;
//             }
//         }
//     }
// }

// void speed_scan_time_add(void)
// {
//     static volatile u16 speed_scan_time_ms = 0;
//     static volatile u16 speed_scan_over_time_cnt = 0;

//     // if (speed_pulse_cnt == 0)
//     // {
//     //     speed_scan_over_time_cnt++;
//     //     if (speed_scan_over_time_cnt >= SPEED_SCAN_OVER_TIME)
//     //     {
//     //         speed_scan_over_time_cnt = 0;
//     //         flag_is_speed_scan_over_time = 1;
//     //     }
//     // }
//     // else
//     // {
//     //     speed_scan_over_time_cnt = 0;
//     // }

//     speed_scan_time_ms++;
//     if (speed_scan_time_ms >= SPEED_SCAN_UPDATE_TIME)
//     {
//         // speed_scan_update_data();
//         cur_speed_scan_time += speed_scan_time_ms;
//         speed_scan_time_ms = 0;
//         cur_speed_scan_pulse += speed_pulse_cnt;
//         speed_pulse_cnt = 0;
//     }
// }

void speed_scan(void)
{
    volatile u32 cur_speed = 0;
    volatile u32 tmp = 0;
    // static u8 is_initiatlized = 0;

    if (cur_speed_scan_time >= SPEED_SCAN_UPDATE_TIME || flag_is_speed_scan_over_time)
    {
        /*
            采集到的脉冲个数 / 一圈对应的脉冲个数 * 车轮一圈对应走过的距离（单位：mm），
            计算得到 采集的脉冲个数对应走过的距离（单位：mm）
        */
        // u32 tmp = (cur_speed_scan_pulse * SPEED_SCAN_MM_PER_TURN / SPEED_SCAN_PULSE_PER_TURN);
        tmp = ((cur_speed_scan_pulse * SPEED_SCAN_MM_PER_TURN) / SPEED_SCAN_PULSE_PER_TURN);

        // 27,638 = (脉冲个数 * 1070) / 3;
        //             77

        // printf("cur_speed_scan_pulse %lu\n", cur_speed_scan_pulse);

        if (flag_is_speed_scan_over_time) // 超时，采集到的脉冲个数对应一直是0km/h，认为时速是0
        {
            // printf("scan over time\n");
            if (cur_speed_scan_pulse != 0)
            {
                // 如果采集的脉冲个数不为0
                cur_speed = 1;
            }
            else
            {
                cur_speed = 0;
            }
        }
        else // 未超时，计算采集到的脉冲个数对应走过的距离，再转换成以 km/h 的单位
        {
            /*
                采集的脉冲个数对应走过的距离（单位：mm）/ 采集所用的时间（单位：ms） == 速度（单位：mm/ms）
                1mm/ms == 1m/s
                因为 1mm/ms * 1000 == 1m/ms
                     1m/ms  / 1000 == 1m/s
                     先乘以1000再除以1000，那么这个操作就可以去掉，直接化简为 1mm/ms == 1m/s

                1m/s == 3.6km/h，那么根据得到的 以 m/s 为单位的速度，先乘以36再除以10，得到以km/h为单位的速度值

                cur_speed == 采集的脉冲个数对应走过的距离（单位：mm）/ 采集所用的时间（单位：ms）* 3.6
                换成单片机可以计算的格式：
                cur_speed == 采集的脉冲个数对应走过的距离（单位：mm） * 36 / 10 / 采集所用的时间（单位：ms）
            */
            cur_speed = tmp * 36 / 10 / cur_speed_scan_time;
        }

        // 防止时速为0时（有可能是推车，记录不到速度），记录不到里程
        distance += tmp;

        // printf("cur distace 2 %lu\n", distance);
        // printf("cur distace %lu\n", distance);
        // printf("cur speed == %lu \n", cur_speed);
        // printf("cur_speed_scan_pulse == %lu\n", cur_speed_scan_pulse);

        cur_speed_scan_pulse = 0;
        cur_speed_scan_time = 0;
        flag_is_speed_scan_over_time = 0;

        if (cur_speed > 0)
        {
            cur_speed = (u32)cur_speed * 102 / 100;
        }

        // 限制要发送的时速:
        if (cur_speed > 199)
        {
            cur_speed = 199;
        }

        // // USER_TO_DO 由于更新显示速度的时间很快，只能在这里加入稳定速度值的处理
        // /**/
        // if (is_initiatlized == 0)
        // {
        //     is_initiatlized = 1;
        //     speed_scan_filter_init(cur_speed);
        // }

        // speed_scan_filter_add(cur_speed);
        // cur_speed = speed_scan_filter_get_speed();
        instrument.speed = cur_speed;
    }
}

void aip3368h_display_speed_refresh_time_add(void)
{
    if (aip3368h_display_speed_refresh_time_cnt < ((u8)-1)) // 防止计数溢出
    {
        aip3368h_display_speed_refresh_time_cnt++;
    }
}

void aip3368h_display_speed_handle(void)
{
    static volatile u8 is_initialized = 0; // 是否初始化
    static volatile u8 speed_of_lag = 0;   // 延迟显示的时速
    volatile u8 cur_speed;                 //

    // α越小越平滑但响应慢,α越大响应快但平滑差
#define ALPHA 7 // 滤波系数，范围：0 ~ 10，推荐值 1 ~ 3
    static volatile u8 filtered_speed;
    u8 base_step;
    u8 speed_abs_diff;

    if (0 == is_initialized)
    {
        is_initialized = 1;

        speed_of_lag = instrument.speed; // 初始化，直接获取当前最新的速度值
        filtered_speed = instrument.speed;
        aip3368h_display_speed(speed_of_lag);

        speed_filter_init(instrument.speed);
    }

    if (aip3368h_display_speed_refresh_time_cnt >= AIP3368H_DISPLAY_SPEED_REFRESH_TIME)
    {
        aip3368h_display_speed_refresh_time_cnt = 0;

        // 如果当前显示的速度值和计算出来的速度值相差太大，需要进行快速逼近：
        if (speed_of_lag > instrument.speed)
        {
            speed_abs_diff = speed_of_lag - instrument.speed;
            if (speed_abs_diff >= 10)
            {
                // 根据速度插值，调节步长
                if (speed_abs_diff >= 50)
                {
                    base_step = 20;
                }
                else if (speed_abs_diff >= 20)
                {
                    base_step = 10;
                }
                else
                {
                    base_step = 5;
                }

                speed_of_lag -= base_step;
#if USER_DEBUG_ENABLE
                printf("speed_of_lag == %u\n", (u16)speed_of_lag);
#endif

                speed_filter_init(instrument.speed);
                filtered_speed = instrument.speed;
                aip3368h_display_speed(speed_of_lag);
                return;
            }
        }
        else if (speed_of_lag < instrument.speed)
        {
            speed_abs_diff = instrument.speed - speed_of_lag;
            if (speed_abs_diff >= 10)
            {
                // 根据速度插值，调节步长
                if (speed_abs_diff >= 50)
                {
                    base_step = 20;
                }
                else if (speed_abs_diff >= 20)
                {
                    base_step = 10;
                }
                else
                {
                    base_step = 5;
                }

                speed_of_lag += base_step;
#if USER_DEBUG_ENABLE
                printf("speed_of_lag == %u\n", (u16)speed_of_lag);
#endif

                speed_filter_init(instrument.speed);
                filtered_speed = instrument.speed;
                aip3368h_display_speed(speed_of_lag);
                return;
            }
        }

        speed_filter_add(instrument.speed);
        cur_speed = speed_filter_get_speed();
        /*
            如果当前速度值和过滤后的速度值都是10以下，需要注意
            不能再经过低通滤波器，会导致最后计算出的时速变为0
        */
        if (cur_speed < 10)
        {
            filtered_speed = cur_speed;
        }
        else
        {
            // 套用一阶低通滤波器计算公式：
            filtered_speed = ((u16)ALPHA * cur_speed + ((u16)10 - ALPHA) * filtered_speed) / 10;
        }

        cur_speed = filtered_speed;

        if (speed_of_lag > cur_speed)
        {
            speed_of_lag--;
        }
        else if (speed_of_lag < cur_speed)
        {
            speed_of_lag++;
        }

#if USER_DEBUG_ENABLE
        printf("speed_of_lag == %u\n", (u16)speed_of_lag);
#endif
        aip3368h_display_speed(speed_of_lag);
    }
}

#endif
