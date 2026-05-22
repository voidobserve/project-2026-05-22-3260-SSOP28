#include "engine_speed_scan.h"

#if ENGINE_SPEED_SCAN_ENABLE

volatile u32 engine_speed_scan_cnt; // 检测到的脉冲个数，在定时器中断累加
volatile u16 engine_speed_scan_ms;  // 在定时器中断累加

static volatile u32 cur_engine_speed_scan_cnt;
static volatile u32 cur_engine_speed_scan_ms;

volatile bit flag_is_engine_speed_scan_over_time; // 标志位，检测是否超时

// 计数器，计数满一段时间后，更新显示
static volatile u16 aip3368h_display_engine_speed_refresh_time_cnt = 0;
static volatile u16 aip3368h_display_exclamation_point_refresh_time_cnt = 0;

// 发动机转速的相关配置
void engine_speed_scan_config(void)
{
    // 使用定时器扫描IO电平的方式
    P0_MD0 &= ~GPIO_P02_MODE_SEL(0x3); // 输入模式
    P0_PU |= GPIO_P02_PULL_UP(0x01);   // 上拉
}

void update_engine_speed_scan_data(void) // 更新检测发动机转速的数据
{
    cur_engine_speed_scan_cnt += engine_speed_scan_cnt;
    engine_speed_scan_cnt = 0;
    cur_engine_speed_scan_ms += engine_speed_scan_ms;
    engine_speed_scan_ms = 0;
}

void engine_speed_scan_timer_50us_isr(void)
{
    static volatile bit last_engine_speed_scan_level = 0; // 记录上一次检测到的引脚电平（发送机转速检测脚）

    // 记录发动机转速扫描的时间
    static u8 cnt = 0;
    cnt++;
    if (cnt >= 20) // 每1ms进入一次
    {
        cnt = 0;
        engine_speed_scan_ms++;

        if (engine_speed_scan_ms >= ENGINE_SPEED_SCAN_OVER_TIME &&
            flag_is_engine_speed_scan_over_time == 0)
        {
            engine_speed_scan_ms = 0;
            flag_is_engine_speed_scan_over_time = 1; // 说明超时，脉冲计数一直没有加一
        }
    }

    if (ENGINE_SPEED_SCAN_PIN) // 检测发动机转速的引脚
    {
        if (0 == last_engine_speed_scan_level)
        {
            // 如果之前检测到低电平，现在检测到高电平，说明有上升沿，对脉冲计数加一
            // if (detect_engine_pulse_cnt[0] < 4294967295) // 防止计数溢出
            // {
            //     detect_engine_pulse_cnt[0]++;
            // }
            engine_speed_scan_cnt++;
            update_engine_speed_scan_data();
        }

        last_engine_speed_scan_level = 1;
    }
    else
    {
        // 如果现在检测到低电平
        last_engine_speed_scan_level = 0;
    }
}

// 发动机转速扫描
void engine_speed_scan(void)
{
#define CONVER_ONE_MINUTE_TO_MS (60000UL) // 将1min转换成以ms为单位的数据
    volatile u32 rpm;                     // 由下面的语句赋值，这里为了节省程序空间，没有赋初始值

    if (cur_engine_speed_scan_ms >= ENGINE_SPEED_SCAN_UPDATE_TIME || flag_is_engine_speed_scan_over_time)
    // if (cur_engine_speed_scan_ms >= ENGINE_SPEED_SCAN_UPDATE_TIME )
    {
        // printf("cur_engine_speed_scan_ms:%lu\n", cur_engine_speed_scan_ms);
        if (flag_is_engine_speed_scan_over_time)
        {
            flag_is_engine_speed_scan_over_time = 0;
            rpm = 0;
        }
        else
        {
            /*
                (1min / 1min转过的圈数) == (扫描时间 / 扫描时间内的转过的圈数)
                1min转过的圈数 == 1min * 扫描时间内转过的圈数 / 扫描时间
                1min转过的圈数 == 1min * (扫描时间内采集到的脉冲个数 / 发动机转过一圈对应的脉冲个数) / 扫描时间
                转换成单片机能计算的形式：
                1min转过的圈数 == 扫描时间内采集到的脉冲个数 * 1min / 发动机转过一圈对应的脉冲个数 / 扫描时间
                1min转过的圈数 == 扫描时间内采集到的脉冲个数 * 1min / 扫描时间 / 发动机转过一圈对应的脉冲个数
            */
            // rpm = (u32)cur_engine_speed_scan_cnt * ((u32)CONVER_ONE_MINUTE_TO_MS / ENGINE_SPEED_SCAN_PULSE_PER_TURN) / cur_engine_speed_scan_ms;

            /*
                扫描时间内转过的圈数 == 一个脉冲对应转过的圈数 *　扫描时间内采集到的脉冲个数
                1min转过的圈数　== 扫描时间内转过的圈数 / 扫描时间 * 1min
            */
            rpm = (u32)cur_engine_speed_scan_cnt *
                  ENGINE_SPEED_SCAN_A_PULSE_PER_TURNS *
                  CONVER_ONE_MINUTE_TO_MS /
                  cur_engine_speed_scan_ms;
        }

        // printf("cur engine speed pulse cnt:%lu\n",cur_engine_speed_scan_cnt);

        cur_engine_speed_scan_cnt = 0;
        cur_engine_speed_scan_ms = 0;

        // 限制待发送的发动机转速
        if (rpm >= 65535)
        {
            rpm = 65535;
        }

        // printf("cur rpm %lu\n", rpm);

        instrument.engine_speed = rpm; // 向全局变量存放发动机转速
    }
}

// // 定义发动机转速和挡位的映射关系类型
// typedef struct
// {
//     u8 level_index;
//     u8 engine_speed_per_k_rpm;
// } engine_speed_map_t;
// static const engine_speed_map_t engine_speed_map[] = {
//     {0, 1}, // 挡位0 1000rpm
//     {1, 2}, // 挡位1 2000rpm
// };

void aip3368h_display_engine_speed_refresh_time_add(void)
{
    if (aip3368h_display_engine_speed_refresh_time_cnt < ((u16)-1))
    {
        aip3368h_display_engine_speed_refresh_time_cnt++;
    }

    if (aip3368h_display_exclamation_point_refresh_time_cnt < ((u16)-1))
    {
        aip3368h_display_exclamation_point_refresh_time_cnt++;
    }
}

// 将采集到的发动机转速转换为仪表对应的转速滑动条挡位
u8 engine_speed_get_level(void)
{
    u8 level = instrument.engine_speed / 1000; // 仪表上的一格对应1000rpm

    if (level > 12)
    {
        level = 12;
    }

    return level;
}

void aip3368h_display_engine_speed_handle(void)
{
    // 延迟显示的发动机转速
    static u8 engine_speed_level_of_lag = 0;
    static u8 is_initialized = 0;
    u8 cur_engine_speed_level = 0;

    if (is_initialized == 0)
    {
        is_initialized = 1;

        engine_speed_level_of_lag = engine_speed_get_level();
    }

    if (aip3368h_display_engine_speed_refresh_time_cnt >= AIP3368H_DISPLAY_ENGINE_SPEED_REFRESH_TIME)
    {
        aip3368h_display_engine_speed_refresh_time_cnt = 0;
        cur_engine_speed_level = engine_speed_get_level();

        if (engine_speed_level_of_lag < cur_engine_speed_level)
        {
            engine_speed_level_of_lag++;
        }
        else if (engine_speed_level_of_lag > cur_engine_speed_level)
        {
            if (engine_speed_level_of_lag > 0)
            {
                engine_speed_level_of_lag--;
            }
        }

        // printf("instrument.engine_speed == %lu\n", instrument.engine_speed);
        // printf("engine_speed_level_of_lag == %u\n", (u16)engine_speed_level_of_lag);

        if (engine_speed_level_of_lag >= 8)
        {
            instrument.flag_is_engine_speed_warning_enable = 1;
        }
        else
        {
            instrument.flag_is_engine_speed_warning_enable = 0;
            // 取消警报之后，需要立即取消显示：
            aip3368h_display_exclamation_point(0);
        }

        aip3368h_display_engine_speed_scale_bar(engine_speed_level_of_lag);
    } 
}

#endif // #if ENGINE_SPEED_SCAN_ENABLE
