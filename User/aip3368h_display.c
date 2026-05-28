#include "aip3368h_display.h"

// ========================================================================
/**
 * @brief 7段数码管段码定义 (a,b,c,d,e,f,g)
 *        对应二进制位: bit0=a, bit1=b, bit2=c, bit3=d, bit4=e, bit5=f, bit6=g
 */
static const u8 digit_segment_code[10] = {
    0x3F, // 0: abcdef
    0x06, // 1: bc
    0x5B, // 2: abdeg
    0x4F, // 3: abcdg
    0x66, // 4: bcfg
    0x6D, // 5: acdfg
    0x7D, // 6: acdefg
    0x07, // 7: abc
    0x7F, // 8: abcdefg
    0x6F  // 9: abcdfg
};

/**
 * @brief 里程各位数码管的段映射表
 *        每个位置有7个段(a-g)，需要映射到不同的buff数组和bit位
 *        结构: [bit_position][segment] ->(映射)-> {buff_index, bit_offset}
 */
typedef struct
{
    u8 buff_index; // aip3368h_display_buff 数组索引
    u8 bit_offset; // 在aip3368h_display_buff 数组索引中的bit偏移
} segment_mapping_t;

// 里程6位数码管，每位的7个段(a-g)的映射关系
static const segment_mapping_t mileage_segment_map[6][7] = {
    // 第0位 ( 最左边 )
    {
        {3, 13}, // a段 -> buff[3], bit13
        {3, 12}, // b段 -> buff[3], bit12
        {2, 13}, // c段 -> buff[2], bit13
        {3, 8},  // d段 -> buff[3], bit8
        {3, 9},  // e段 -> buff[3], bit9
        {3, 11}, // f段 -> buff[3], bit11
        {3, 10}  // g段 -> buff[3], bit10
    },
    // 第1位 (   )
    {
        {3, 0}, // a段 -> buff[3], bit0
        {3, 2}, // b段 -> buff[3], bit2
        {3, 5}, // c段 -> buff[3], bit5
        {3, 7}, // d段 -> buff[3], bit7
        {3, 6}, // e段 -> buff[3], bit6
        {3, 3}, // f段 -> buff[3], bit3
        {3, 4}  // g段 -> buff[3], bit4
    },
    // 第2位 (   )
    {
        {4, 14}, // a段 -> buff[4], bit14
        {4, 12}, // b段 -> buff[4], bit12
        {4, 9},  // c段 -> buff[4], bit9
        {4, 8},  // d段 -> buff[4], bit8
        {4, 10}, // e段 -> buff[4], bit10
        {4, 13}, // f段 -> buff[4], bit13
        {4, 11}  // g段 -> buff[4], bit11
    },
    // 第3位 (   )
    {
        {4, 1},  // a段 -> buff[4], bit1
        {5, 15}, // b段 -> buff[5], bit15
        {4, 4},  // c段 -> buff[4], bit4
        {4, 6},  // d段 -> buff[4], bit6
        {4, 7},  // e段 -> buff[4], bit7
        {4, 0},  // f段 -> buff[4], bit0
        {4, 2}   // g段 -> buff[4], bit2
    },
    // 第4位 (   )
    {
        {5, 11}, // a段 -> buff[5], bit11
        {5, 10}, // b段 -> buff[5], bit10
        {4, 3},  // c段 -> buff[4], bit3
        {4, 5},  // d段 -> buff[4], bit5
        {5, 13}, // e段 -> buff[5], bit13
        {5, 14}, // f段 -> buff[5], bit14
        {5, 12}  // g段 -> buff[5], bit12
    },
    // 第5位 ( 最右边 )
    {
        {5, 3}, // a段 -> buff[5], bit3
        {5, 2}, // b段 -> buff[5], bit2
        {5, 4}, // c段 -> buff[5], bit4
        {5, 7}, // d段 -> buff[5], bit7
        {5, 9}, // e段 -> buff[5], bit9
        {5, 6}, // f段 -> buff[5], bit6
        {5, 5}  // g段 -> buff[5], bit5
    }};

// ========================================================================

typedef struct
{
    u8 buff_index; // aip3368h_display_buff[] 中对应元素索引
    u8 bit_offset; // aip3368h_display_buff[] 中对应元素中的第 x 位
} speed_scale_bar_map_t;

/*
    将 时速刻度条 与 aip3368h_display_buff[] 建立映射关系
    例如：
    点亮 第 0 个 刻度条对应的指示灯，给 aip3368h_display_buff[] 中第 x 个元素 第 y 位 置一
*/
static const speed_scale_bar_map_t speed_scale_bar_map[16] = {
    {7, 7},  // 0
    {7, 6},  // 1
    {7, 5},  // 2
    {7, 4},  // 3
    {7, 8},  // 4
    {7, 9},  // 5
    {7, 10}, // 6
    {7, 11}, // 7
    {6, 3},  // 8
    {6, 4},  // 9
    {6, 5},  // 10
    {6, 6},  // 11
    {6, 7},  // 12
    {6, 8},  // 13
    {6, 9},  // 14
    {6, 10}, // 15
};

// ========================================================================

volatile aip3368h_display_obj_t aip3368h_display_obj = {0};

static volatile u16 aip3368h_display_boot_animation_time_cnt = 0;
// 给开机动画处理函数提供时基：
static volatile bit aip3368h_display_boot_animation_time_add_flag = 0;

static volatile u16 aip3368h_display_err_handle_time_cnt = 0;

// 背光灯刻度条的映射关系表：
static const aip3368h_display_mapping_t back_light_scale_bar_map[] = {
    {10, 5},
    {10, 2},
    {9, 2},
    {9, 4},
    {9, 7},

    {9, 10}, // 第 5 个灯
    {9, 9},
    {8, 2},
    {8, 4},
    {8, 7},

    {8, 6}, // 第 10 个灯
    {8, 9},
    {8, 10},
    {8, 12},
    {8, 13},

    {6, 10}, // 第 15 个灯
    {6, 9},
    {6, 13},
    {6, 15},
    {0, 5},

    {0, 3}, // 第 20 个灯
    {0, 2},
    {1, 13},
    {1, 12},
    {1, 3},

    {1, 2}, // 第 25 个灯
    {1, 1},
    {2, 4},
    {2, 7},
    {2, 6},

    {2, 8}, // 第 30 个灯
    {2, 9},
    {2, 10},
};

// 发动机转速刻度条的映射关系表：
static const aip3368h_display_mapping_t engine_speed_scale_bar_map[] = {
    {10, 7},
    {10, 6},
    {10, 4},
    {10, 3},
    {10, 1},

    {10, 0},
    {9, 0},
    {9, 3},
    {9, 5},
    {9, 11},

    {9, 8},
    {8, 1},
    {8, 3},
    {8, 5},
    {8, 8},

    {8, 11},
    {8, 14},
    {6, 4},
    {6, 5},
    {6, 6},

    {6, 12},
    {6, 14},
    {0, 6},
    {0, 4},
};

// 发动机转速的数字刻度对应的映射关系：
static const aip3368h_display_mapping_t engine_speed_digit_scale_map[] = {
    {5, 9},
    {7, 6},
    {7, 7},
    {7, 8},
    {9, 1},

    {9, 12},
    {9, 13},
    {8, 0},
    {8, 15},
    {6, 7},

    {6, 2},
    {0, 7},
    {1, 11},
};

// 挡位数码管的映射关系表：
static const aip3368h_display_mapping_t gear_digit_map[] = {
    {2, 2},  // a 段
    {2, 3},  // b 段
    {2, 12}, // c 段
    {2, 14}, // d 段
    {2, 15}, // e 段

    {2, 0}, // f 段
    {2, 1}, // g 段
};

// 时速的第1位 ~ 第2位数码管(不包括第0位)，每位的7个段(a-g)的映射关系
static const aip3368h_display_mapping_t speed_segment_map[][7] = {
    // 第0位 ( 最左边。时速的第0位只有b段和c段)
    {
        {0, 0},  // 无效数据段
        {9, 14}, // b 段
        {9, 15}, // c 段
        {0, 0},  // 无效数据段
        {0, 0},  // 无效数据段
        {0, 0},  // 无效数据段
        {0, 0}   // 无效数据段
    },

    // 第1位 数码管
    {
        {6, 3},  // a 段
        {7, 15}, // b 段
        {7, 12}, // c 段
        {7, 9},  // d 段
        {7, 11}, // e 段
        {7, 14}, // f 段
        {7, 13}  // g 段
    },

    // 第2位 数码管
    {
        {6, 1}, // a 段
        {7, 3}, // b 段
        {7, 4}, // c 段
        {7, 5}, // d 段
        {7, 1}, // e 段
        {7, 0}, // f 段
        {7, 2}  // g 段
    },
};

// 显示 左转向 指示灯
void aip3368h_display_left_turn_light(u8 is_enable)
{
    aip3368h_display_buff[6] &= ~(0x01 << 8); // 左转向灯（绿）

    if (is_enable)
    {
        aip3368h_display_buff[6] |= 0x01 << 8; // 左转向灯（绿）
    }
}

// 显示 右转向 指示灯
void aip3368h_display_right_turn_light(u8 is_enable)
{
    aip3368h_display_buff[1] &= ~(0x01 << 0); // 右转向灯

    if (is_enable)
    {
        aip3368h_display_buff[1] |= 0x01 << 0; // 右转向灯
    }
}

// 显示 故障 指示灯
void aip3368h_display_err_light(u8 is_enable)
{
    aip3368h_display_buff[6] &= ~(0x01 << 11); // 故障指示灯（红）

    if (is_enable)
    {
        aip3368h_display_buff[6] |= 0x01 << 11; // 故障指示灯（红）
    }
}

// 显示 小灯（近光灯） 指示灯
void aip3368h_display_low_beam_indicator_light(u8 is_enable)
{
    aip3368h_display_buff[0] &= ~(0x01 << 12); // 小灯（近光灯）

    if (is_enable)
    {
        aip3368h_display_buff[0] |= 0x01 << 12; // 小灯（近光灯）
    }
}

// 显示 大灯（远光灯） 指示灯
void aip3368h_display_high_beam_indicator_light(u8 is_enable)
{
    aip3368h_display_buff[1] &= ~(0x01 << 14); // 大灯（远光灯）

    if (is_enable)
    {
        aip3368h_display_buff[1] |= 0x01 << 14; // 大灯（远光灯）
    }
}

/**
 * @brief 显示 背光刻度条
 *
 * @param level 0 ~ 33，0：不显示，1：显示第 0 个灯，32：显示第 32 个灯
 *
 * @return * void
 */
void aip3368h_display_back_light_scale_bar(u8 level)
{
    u8 i = 0;
    for (i = 0; i < ARRAY_SIZE(back_light_scale_bar_map); i++)
    {
        aip3368h_display_buff[back_light_scale_bar_map[i].buff_index] &=
            ~(0x01 << back_light_scale_bar_map[i].bit_offset);
    }

    if (level > 0)
    {
        for (i = 0; i <= level - 1; i++)
        {
            aip3368h_display_buff[back_light_scale_bar_map[i].buff_index] |=
                (0x01 << back_light_scale_bar_map[i].bit_offset);
        }
    }
}

/**
 * @brief 显示 发动机转速刻度条
 *
 * @param level 0 ~ 24，0：不显示，1：显示第 0 个灯，24：显示第 23 个灯
 */
void aip3368h_display_engine_speed_scale_bar(u8 level)
{
    // aip3368h_display_buff[10] |= 0x01 << 7;  // 发动机转速刻度条 第 0 个灯（白）
    // aip3368h_display_buff[10] |= 0x01 << 6;  // 发动机转速刻度条 第 1 个灯（白）
    // aip3368h_display_buff[10] |= 0x01 << 4;  // 发动机转速刻度条 第 2 个灯（白）
    // aip3368h_display_buff[10] |= 0x01 << 3;  // 发动机转速刻度条 第 3 个灯（白）
    // aip3368h_display_buff[10] |= 0x01 << 1; // 发动机转速刻度条 第 4 个灯（白）

    // aip3368h_display_buff[10] |= 0x01 << 0; // 发动机转速刻度条 第 5 个灯（白）
    // aip3368h_display_buff[9] |= 0x01 << 0;  // 发动机转速刻度条 第 6 个灯（白）
    // aip3368h_display_buff[9] |= 0x01 << 3;  // 发动机转速刻度条 第 7 个灯（白）
    // aip3368h_display_buff[9] |= 0x01 << 5;  // 发动机转速刻度条 第 8 个灯（白）
    // aip3368h_display_buff[9] |= 0x01 << 11; // 发动机转速刻度条 第 9 个灯（白）

    // aip3368h_display_buff[9] |= 0x01 << 8;  // 发动机转速刻度条 第 10 个灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 1;  // 发动机转速刻度条 第 11 个灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 3;  // 发动机转速刻度条 第 12 个灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 5;  // 发动机转速刻度条 第 13 个灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 8;  // 发动机转速刻度条 第 14 个灯（白）

    // aip3368h_display_buff[8] |= 0x01 << 11; // 发动机转速刻度条 第 15 个灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 14; // 发动机转速刻度条 第 16 个灯（白）
    // aip3368h_display_buff[6] |= 0x01 << 4;  // 发动机转速刻度条 第 17 个 指示灯（白）
    // aip3368h_display_buff[6] |= 0x01 << 5;  // 发动机转速刻度条 第 18 个 指示灯（白）
    // aip3368h_display_buff[6] |= 0x01 << 6;  // 发动机转速刻度条 第 19 个 指示灯（白）

    // aip3368h_display_buff[6] |= 0x01 << 12; // 发动机转速刻度条 第 20 个灯（红）
    // aip3368h_display_buff[6] |= 0x01 << 14; // 发动机转速刻度条 第 21 个灯（红）
    // aip3368h_display_buff[0] |= 0x01 << 6;  // 发动机转速刻度条 第 22 个 指示灯（红）
    // aip3368h_display_buff[0] |= 0x01 << 4;  // 发动机转速刻度条 第 23 个 指示灯（红）

    u8 i;
    for (i = 0; i < ARRAY_SIZE(engine_speed_scale_bar_map); i++)
    {
        aip3368h_display_buff[engine_speed_scale_bar_map[i].buff_index] &=
            ~(0x01 << engine_speed_scale_bar_map[i].bit_offset);
    }

    if (level > 0)
    {
        for (i = 0; i <= level - 1; i++)
        {
            aip3368h_display_buff[engine_speed_scale_bar_map[i].buff_index] |=
                (0x01 << engine_speed_scale_bar_map[i].bit_offset);
        }
    }
}

/**
 * @brief 显示 x1000rpm 字样 指示灯
 *
 */
void aip3368h_display_x1000rpm_light(u8 is_enable)
{
    aip3368h_display_buff[1] &= ~(0x01 << 5);  // "x1000r/min"字样 第 2 个灯（白）
    aip3368h_display_buff[1] &= ~(0x01 << 6);  // "x1000r/min"字样 第 1 个灯（白）
    aip3368h_display_buff[1] &= ~(0x01 << 10); // "x1000r/min"字样 第 0 个灯（白）

    if (is_enable)
    {
        aip3368h_display_buff[1] |= 0x01 << 5;  // "x1000r/min"字样 第 2 个灯（白）
        aip3368h_display_buff[1] |= 0x01 << 6;  // "x1000r/min"字样 第 1 个灯（白）
        aip3368h_display_buff[1] |= 0x01 << 10; // "x1000r/min"字样 第 0 个灯（白）
    }
}

/**
 * @brief 显示 发动机转速的数字刻度
 *
 * @param level 0 ~ 13，0：不显示，1：显示第 0 个灯，13：显示第 12 个灯
 *
 */
void aip3368h_display_engine_speed_digit_scale(u8 level)
{
    // aip3368h_display_buff[5] |= 0x01 << 9; // 发动机转速 "0" 字样 指示灯（白）
    // aip3368h_display_buff[7] |= 0x01 << 6; // 发动机转速 "1" 字样 指示灯（白）
    // aip3368h_display_buff[7] |= 0x01 << 7; // 发动机转速 "2" 字样 指示灯（白）
    // aip3368h_display_buff[7] |= 0x01 << 8; // 发动机转速 "3" 字样 指示灯（白）
    // aip3368h_display_buff[9] |= 0x01 << 1; // 发动机转速 "4" 字样 指示灯（白）

    // aip3368h_display_buff[9] |= 0x01 << 12; // 发动机转速 "5" 字样 指示灯（白）
    // aip3368h_display_buff[9] |= 0x01 << 13; // 发动机转速 "6" 字样 指示灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 0;  // 发动机转速 "7" 字样 指示灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 15; // 发动机转速 "8" 字样 指示灯（白）
    // aip3368h_display_buff[6] |= 0x01 << 7;  // 发动机转速 "9" 字样 指示灯（白）

    // aip3368h_display_buff[6] |= 0x01 << 2;  // 发动机转速 "10" 字样 指示灯（红）
    // aip3368h_display_buff[0] |= 0x01 << 7;  // 发动机转速 "11" 字样 指示灯（红）
    // aip3368h_display_buff[1] |= 0x01 << 11; // 发动机转速 "12"字样 指示灯（红）

    u8 i;
    for (i = 0; i < ARRAY_SIZE(engine_speed_digit_scale_map); i++)
    {
        aip3368h_display_buff[engine_speed_digit_scale_map[i].buff_index] &=
            ~(0x01 << engine_speed_digit_scale_map[i].bit_offset);
    }

    if (level > 0)
    {
        for (i = 0; i <= level - 1; i++)
        {
            aip3368h_display_buff[engine_speed_digit_scale_map[i].buff_index] |=
                (0x01 << engine_speed_digit_scale_map[i].bit_offset);
        }
    }
}

/**
 * @brief 显示 挡位 "GEAR" 字样指示灯（白）
 *
 */
void aip3368h_display_gear_light(u8 is_enable)
{
    aip3368h_display_buff[2] &= ~(0x01 << 11); // "GEAR" 字样 第 0 个灯（白）

    if (is_enable)
    {
        aip3368h_display_buff[2] |= 0x01 << 11; // 挡位 "GEAR" 字样指示灯（白）
    }
}

/**
 * @brief 显示 挡位 "N" 字样指示灯
 *
 */
void __aip3368h_display_gear_n_light__(u8 is_enable)
{
    aip3368h_display_buff[2] &= ~(0x01 << 5); // 挡位 "N" 字样指示灯（绿）

    if (is_enable)
    {
        aip3368h_display_buff[2] |= 0x01 << 5; // 挡位 "N" 字样指示灯（绿）
    }
}

/**
 * @brief 显示 挡位 1 ~ 6
 *
 * @param level 0 ~ 6，0：不显示，1：显示数字"1"，2：显示数字"2"
 */
void __aip3368h_display_gear_digit__(u8 level)
{
    u8 i;
    u8 segment_code;
    for (i = 0; i < ARRAY_SIZE(gear_digit_map); i++)
    {
        aip3368h_display_buff[gear_digit_map[i].buff_index] &=
            ~(0x01 << gear_digit_map[i].bit_offset);
    }

    if (level <= 0)
    {
        return;
    }

    // 获取该数字对应的7段码 (要显示的数字 --> 七段码)
    segment_code = digit_segment_code[level];

    // 遍历 a ~ g 段数码管
    for (i = 0; i <= 7; i++)
    {
        // 检查该段是否需要点亮 (segment_code的对应bit是否为1)
        if (segment_code & (0x01 << i))
        {
            aip3368h_display_buff[gear_digit_map[i].buff_index] |=
                (0x01 << gear_digit_map[i].bit_offset);
        }
    }
}

/**
 * @brief 显示 挡位（不能开机动画调用这个函数）
 *
 * @param level 0 ~ 6
 *              0：显示挡位 "N" 字样指示灯，
 *              1：显示 1 挡
 *              2: 显示 2 挡
 */
void aip3368h_display_gear(u8 level)
{
    // 清空显示
    __aip3368h_display_gear_n_light__(0);
    __aip3368h_display_gear_digit__(0);

    if (0 == level)
    {
        __aip3368h_display_gear_n_light__(1);
    }
    else
    {
        __aip3368h_display_gear_digit__(level);
    }
}

/**
 * @brief 显示速度数码管上的哪一段
 *
 * @param bit_x 0 ~ 2，对应第 0 ~ 2 位数码管
 *
 * @param seg 0 ~ 6，对应 a ~ g 段
 *
 * @param is_enable 0: 关闭 1: 打开
 */
void __aip3368h_display_speed_seg__(u8 bit_x, seg_index_t seg, u8 is_enable)
{
    aip3368h_display_buff[speed_segment_map[bit_x][seg].buff_index] &=
        ~(1 << speed_segment_map[bit_x][seg].bit_offset);

    if (is_enable)
    {
        aip3368h_display_buff[speed_segment_map[bit_x][seg].buff_index] |=
            (1 << speed_segment_map[bit_x][seg].bit_offset);
    }
}

/**
 * @brief 控制数码管 第 x 位 显示的数字
 *
 * @attention 只在 aip3368h_display_speed() 被调用，
 *              函数内省略了清空显示的操作
 *
 * @param bit_x 0 ~ 2，对应第 0 ~ 2 位数码管
 * @param number 0 ~ 9，需要显示的数字
 */
void __aip3368h_display_speed_bit_x__(u8 bit_x, u8 number)
{
    u8 i;
    // u8 seg;
    // 获取该数字对应的7段码 (要显示的数字 --> 七段码)
    u8 segment_code = digit_segment_code[number];

    // // 清除第 x 位数码管显示
    // for (i = 0; i < 7; i++)
    // {
    //     aip3368h_display_buff[speed_segment_map[bit_x][i].buff_index] &=
    //         ~(0x01 << speed_segment_map[bit_x][i].bit_offset);
    // }

    // 根据段码设置对应的buff位
    for (i = 0; i < 7; i++)
    {
        // 检查该段是否需要点亮 (segment_code的对应bit是否为1)
        if (segment_code & (1 << i))
        {
            // 点亮该段
            aip3368h_display_buff[speed_segment_map[bit_x][i].buff_index] |=
                (0x01 << speed_segment_map[bit_x][i].bit_offset);
        }
    }
}

/**
 * @brief 显示时速
 *
 * @param speed 0 ~ 199
 *
 * @attention 时速不显示无效数据位，例如：时速 == 9，则显示 9 km，不会显示 09 km
 *
 */
void aip3368h_display_speed(u8 speed)
{
    u8 i;
    u8 j;
    // u8 segment_code;
    // 传参的有效数据位：
    u8 valid_bits = 0;
    u8 tmp = 0;

    // 默认先清空时速的显示
    // 时速 第 0 位：
    __aip3368h_display_speed_seg__(0, SEG_INDEX_B, 0);
    __aip3368h_display_speed_seg__(0, SEG_INDEX_C, 0);
    // 时速 第 1 ~ 2 位：
    for (i = 1; i < 3; i++)
    {
        for (j = 0; j < 7; j++) // 遍历 A ~ G 段
        {
            aip3368h_display_buff[speed_segment_map[i][j].buff_index] &=
                ~(0x01 << speed_segment_map[i][j].bit_offset);
        }
    }

    // aip3368h_display_speed_km_icon(1);
    // USER_TO_DO 显示 km/h 或者是 mile/h

    // 判断 speed 的有效数据位
    tmp = speed;
    while (1)
    {
        valid_bits++; // 刚进入，默认至少有1位有效数据
        tmp /= 10;
        if (tmp == 0)
        {
            break;
        }
    }

    // printf("valid_bits == %u\n", (u16)valid_bits);

    if (speed >= 100)
    {
        // 显示时速第 0 位的 1：
        __aip3368h_display_speed_seg__(0, SEG_INDEX_B, 1);
        __aip3368h_display_speed_seg__(0, SEG_INDEX_C, 1);
    }

    // USER_TO_DO 如果只有个位数，需要处理：

    for (i = 1; i < 3; i++) // 第 1 ~ 2 位数码管
    {
        if (1 == valid_bits && 2 == i)
        {
            /*
                用于显示的数码管只有三位，
                如果有效数据位只有1位，不显示第0位的"1"字样，
                也不显示第1位数码管的任何内容
            */
            continue;
        }

        // i 初始值为 1，需要从第 0 位数码管开始准备显示的数据：
        __aip3368h_display_speed_bit_x__(3 - i, speed % 10);
        speed /= 10;
    }
}

// USER_TO_DO 待验证
/**
 * @brief 显示时速的单位对应的指示灯
 * 
 * @param type
 * 
 * @param is_enable
 */
void __aip3368h_display_speed_unit_type__(speed_unit_type_t type, u8 is_enable)
{
    if (type == SPEED_UNIT_TYPE_KM_H)
    {
        aip3368h_display_buff[5] &= ~(0x01 << 11); // 时速 "mph" 字样 指示灯（绿）
        if (is_enable)
        {
            aip3368h_display_buff[5] |= 0x01 << 11;
        }
    }
    else
    {
        aip3368h_display_buff[5] &= ~(0x01 << 10); // 时速 "km/h" 字样 指示灯（绿）
        if (is_enable)
        {
            aip3368h_display_buff[5] |= 0x01 << 10;
        } 
    } 
}

/**
 * @brief 显示时速的单位
 * 
 * @attention 不能再开机动画中调用
 *
 * @param type
 *
 * @param is_enable
 *
 */
void aip3368h_display_speed_unit_type(speed_unit_type_t type, u8 is_enable)
{
    if (type == SPEED_UNIT_TYPE_KM_H)
    {
        // aip3368h_display_buff[5] &= ~(0x01 << 11); // 时速 "mph" 字样 指示灯（绿）
        // aip3368h_display_buff[5] |= 0x01 << 10; // 时速 "km/h" 字样 指示灯（绿）
    }

    // aip3368h_display_buff[5] |= 0x01 << 10; // 时速 "km/h" 字样 指示灯（绿）
    // aip3368h_display_buff[5] |= 0x01 << 11; // 时速 "mph" 字样 指示灯（绿）

    // aip3368h_display_buff[5] |= 0x01 << 10; // 时速 "km/h" 字样 指示灯（绿）
    // aip3368h_display_buff[5] |= 0x01 << 11; // 时速 "mph" 字样 指示灯（绿）
}

/**
 * @brief
 *
 * @param level
 */
// void aip3368h_display_fuel_level(aip3368h_display_fuel_level_t level)
// {
//     // 先清空显示，之后再根据传参点亮对应的指示灯
//     aip3368h_display_buff[2] &= ~(0x01 << 15); // 油量，第 0 格指示灯（红）
//     aip3368h_display_buff[2] &= ~(0x01 << 14); // 油量，第 1 格指示灯（绿）
//     aip3368h_display_buff[2] &= ~(0x01 << 0);  // 油量，第 2 格指示灯（绿）
//     aip3368h_display_buff[2] &= ~(0x01 << 1);  // 油量，第 3 格指示灯（绿）
//     aip3368h_display_buff[2] &= ~(0x01 << 2);  // 油量，第 4 格指示灯（绿）

//     // 如果 level == AIP3368H_DISPLAY_FUEL_LEVEL_4，那么0~4对应的指示灯都点亮
//     switch (level)
//     {
//     case AIP3368H_DISPLAY_FUEL_LEVEL_4:
//         aip3368h_display_buff[2] |= (0x01 << 2); // 油量，第 4 格指示灯（绿）
//     case AIP3368H_DISPLAY_FUEL_LEVEL_3:
//         aip3368h_display_buff[2] |= (0x01 << 1); // 油量，第 3 格指示灯（绿）
//     case AIP3368H_DISPLAY_FUEL_LEVEL_2:
//         aip3368h_display_buff[2] |= (0x01 << 0); // 油量，第 2 格指示灯（绿）
//     case AIP3368H_DISPLAY_FUEL_LEVEL_1:
//         aip3368h_display_buff[2] |= (0x01 << 14); // 油量，第 1 格指示灯（绿）
//     case AIP3368H_DISPLAY_FUEL_LEVEL_0:
//         aip3368h_display_buff[2] |= (0x01 << 15); // 油量，第 0 格指示灯（红）
//     default:
//         break;
//     }
// }

/**
 * @brief 显示里程的km字样图标
 *
 */
void aip3368h_display_mileage_km_icon(u8 is_enable)
{
    aip3368h_display_buff[5] &= 0x01 << 0; // 里程 km字样 指示灯，第 1 格
    aip3368h_display_buff[5] &= 0x01 << 1; // 里程 km字样 指示灯，第 0 格

    if (is_enable)
    {
        aip3368h_display_buff[5] |= 0x01 << 0; // 里程 km字样 指示灯，第 1 格
        aip3368h_display_buff[5] |= 0x01 << 1; // 里程 km字样 指示灯，第 0 格
    }
}

/**
 * @brief 里程显示中，在第 x 位显示数字
 *
 * @attention 是 aip3368h_display_mileage() 的子函数
 *
 * @param bit_x 0 ~ 5 (0=个位, 5=十万位)
 * @param number 0 ~ 9
 */
void __aip3368h_display_mileage_bit_x__(u8 bit_x, u8 number)
{
    u8 i;
    // u8 j; // 循环计数值
    // 参数有效性检查（为了节省程序空间，这里可以省略）
    // if (bit_x > 5 || number > 9)
    // {
    //     return;
    // }

    // 获取该数字对应的7段码 (要显示的数字 --> 七段码)
    u8 segment_code = digit_segment_code[number];

    // 清除 bit_x 对应数码管 a ~ g 段的显示
    for (i = 0; i < 7; i++) //
    {
        aip3368h_display_buff[mileage_segment_map[bit_x][i].buff_index] &=
            ~(0x01 << mileage_segment_map[bit_x][i].bit_offset);

        // 检查该段是否需要点亮 (segment_code的对应bit是否为1)
        if (segment_code & (1 << i))
        {
            // 点亮该段
            aip3368h_display_buff[mileage_segment_map[bit_x][i].buff_index] |=
                (0x01 << mileage_segment_map[bit_x][i].bit_offset);
        }
    }
}

/**
 * @brief 只根据传参在里程中显示数字
 *
 * @param mileage
 */
void __aip3368h_display_mileage__(u32 mileage)
{
    u8 i; // 循环计数值

    // 显示里程的数值，不包括单位、小数点：
    for (i = 0; i < 6; i++)
    {
        __aip3368h_display_mileage_bit_x__(5 - i, mileage % 10);
        mileage /= 10;
    }
}

/**
 * @brief 显示里程
 *
 * @param mileage
 *          如果显示总里程，mileage == 12345，则显示 12345 km
 *          如果显示当前里程，mileage == 12345，则显示 1234.5 km
 *
 * @param is_displaying_total_mileage 是否显示总里程。0：显示当前里程(TRIP)，1：显示总里程(ODO)
 */
void aip3368h_display_mileage(u32 mileage, u8 is_displaying_total_mileage)
{
    // 默认先清空有关里程的显示
    aip3368h_display_buff[3] &= ~(0x01 << 15); // 大计里程 ODO 指示灯，第 0 格
    aip3368h_display_buff[3] &= ~(0x01 << 14); // 大计里程 ODO 指示灯，第 1 格

    aip3368h_display_buff[3] &= ~(0x01 << 1);  // 小计里程 TRIP 指示灯，第 0 格
    aip3368h_display_buff[4] &= ~(0x01 << 15); // 小计里程 TRIP 指示灯，第 1 格
    aip3368h_display_buff[5] &= ~(0x01 << 8);  // 里程 小数点 指示灯
    aip3368h_display_mileage_km_icon(1);       // 显示里程 km字样 指示灯

#if 1
    // 显示里程的数值，不包括单位、小数点：
    __aip3368h_display_mileage__(mileage);

    if (is_displaying_total_mileage)
    {
        aip3368h_display_buff[3] |= (0x01 << 15); // 大计里程 ODO 指示灯，第 0 格
        aip3368h_display_buff[3] |= (0x01 << 14); // 大计里程 ODO 指示灯，第 1 格
    }
    else
    {
        aip3368h_display_buff[3] |= (0x01 << 1);  // 小计里程 TRIP 指示灯，第 0 格
        aip3368h_display_buff[4] |= (0x01 << 15); // 小计里程 TRIP 指示灯，第 1 格
        aip3368h_display_buff[5] |= (0x01 << 8);  // 里程 小数点 指示灯
    }
#endif
}

/**
 * @brief 显示时速的km字样图标
 *
 * @param is_enable
 */
void aip3368h_display_speed_km_icon(u8 is_enable)
{
    // 默认先清空显示
    aip3368h_display_buff[6] &= ~(0x01 << 11); // 时速 km字样 指示灯，第 1 格（绿）
    aip3368h_display_buff[6] &= ~(0x01 << 12); // 时速 km字样 指示灯，第 0 格（绿）

    if (is_enable)
    {
        aip3368h_display_buff[6] |= (0x01 << 11); // 时速 km字样 指示灯，第 1 格（绿）
        aip3368h_display_buff[6] |= (0x01 << 12); // 时速 km字样 指示灯，第 0 格（绿）
    }
}

/**
 * @brief 显示时速对应的刻度条
 *
 * @param level 0 ~ 16，
 *          0：不显示，
 *          1：亮起1个指示灯，...
 *          16：所有速度刻度的指示灯亮起
 *
 */
void aip3368h_display_speed_scale_bar(u8 level)
{
    u8 i;

    // 清空速度刻度条的显示
    for (i = 0; i < 16; i++)
    {
        aip3368h_display_buff[speed_scale_bar_map[i].buff_index] &=
            ~(0x01 << speed_scale_bar_map[i].bit_offset);
    }

    for (i = 0; i < level; i++) // 亮起 level 个指示灯
    {
        aip3368h_display_buff[speed_scale_bar_map[i].buff_index] |=
            (0x01 << speed_scale_bar_map[i].bit_offset);
    }
}

/**
 * @brief 发送机转速滑动条的开机动画
 *
 */
void __aip3368h_display_boot_animation_in_engine_speed_scale_bar__(void)
{
    static u16 animation_engine_speed_scale_bar_step = 0;
    static u8 animation_engine_speed_scale_bar_phase = 0; // 0:渐渐递增，1:保持最高，2:渐渐递减
    static u8 animation_engine_speed_scale_bar_level = 0;

    // 动画分为三个阶段：
    // 阶段1 : 从低到高递增
    // 阶段2 (中间时段): 保持最高亮度显示一段时间
    // 阶段3 (最后时段): 从高到低递减
    animation_engine_speed_scale_bar_step++;

    if (animation_engine_speed_scale_bar_phase == 0)
    {
        // 阶段1: 递增阶段
        if (animation_engine_speed_scale_bar_step >= 150) // 每 xx ms变化一次
        {
            animation_engine_speed_scale_bar_step = 0;
            animation_engine_speed_scale_bar_level++;

            if (animation_engine_speed_scale_bar_level >= 12)
            {
                // animation_engine_speed_scale_bar_level = 12;
                animation_engine_speed_scale_bar_phase = 1; // 进入保持阶段
                animation_engine_speed_scale_bar_step = 0;
            }

            // aip3368h_display_engine_speed_scale_bar(animation_engine_speed_scale_bar_level);
        }
    }
    else if (animation_engine_speed_scale_bar_phase == 1)
    {
        // 阶段2: 保持最高阶段
        // 假设保持时间为 xx ms (可根据需要调整)
        if (animation_engine_speed_scale_bar_step >= 200)
        {
            animation_engine_speed_scale_bar_step = 0;
            animation_engine_speed_scale_bar_phase = 2; // 进入递减阶段
        }
        // 保持显示最高级别
        // aip3368h_display_engine_speed_scale_bar(12);
    }
    else if (animation_engine_speed_scale_bar_phase == 2)
    {
        // 阶段3: 递减阶段
        if (animation_engine_speed_scale_bar_step >= 150) // 每 xx ms变化一次
        {
            animation_engine_speed_scale_bar_step = 0;

            if (animation_engine_speed_scale_bar_level > 0)
            {
                animation_engine_speed_scale_bar_level--;
            }

            // aip3368h_display_engine_speed_scale_bar(animation_engine_speed_scale_bar_level);
        }
    }
}

void __aip3368h_display_boot_animation_in_speed_scale_bar__(void)
{
    static u16 animation_speed_scale_bar_step = 0; // 控制时速刻度条的步长
    static u8 animation_phase = 0;                 // 0:渐渐递增，1:保持最高，2:渐渐递减
    static u8 animation_speed_scale_bar_level = 0;

    animation_speed_scale_bar_step++;
    if (0 == animation_phase)
    {
        if (animation_speed_scale_bar_step >= 100)
        {
            animation_speed_scale_bar_step = 0;
            animation_speed_scale_bar_level++;

            if (animation_speed_scale_bar_level >= 16)
            {
                // animation_speed_scale_bar_level = 12;
                animation_phase = 1; // 进入保持阶段
            }

            aip3368h_display_speed_scale_bar(animation_speed_scale_bar_level);
        }
    }
    else if (1 == animation_phase)
    {
        if (animation_speed_scale_bar_step >= 400)
        {
            animation_speed_scale_bar_step = 0;
            animation_phase = 2; // 进入递减阶段
        }
    }
    else if (2 == animation_phase)
    {
        if (animation_speed_scale_bar_step >= 100)
        {
            animation_speed_scale_bar_step = 0;

            if (animation_speed_scale_bar_level > 0)
            {
                animation_speed_scale_bar_level--;
            }

            aip3368h_display_speed_scale_bar(animation_speed_scale_bar_level);
        }
    }
}

// 时速和里程的开机动画显示
void __aip3368h_display_boot_animation_in_speed_and_mileage__(void)
{
    static u16 animation_step = 0; // 控制动画的步长
    static u8 animation_phase = 0; // 0:渐渐递增，1:保持最高，2:渐渐递减
    static u8 animation_val = 0;
    volatile u8 i;

    animation_step++;
    if (0 == animation_phase)
    {
        if (animation_step >= 200)
        {
            animation_step = 0;
            // __aip3368h_display_speed_bit_x__(0, animation_val);
            // __aip3368h_display_speed_bit_x__(1, animation_val);

            for (i = 0; i < 6; i++)
            {
                __aip3368h_display_mileage_bit_x__(i, animation_val);
            }

            animation_val++;

            if (animation_val >= 10)
            {
                animation_val = 9;
                animation_phase = 1;
            }
        }
    }
    else if (1 == animation_phase)
    {
        if (animation_step >= 200)
        {
            animation_step = 0;

            if (animation_val > 0)
            {
                animation_val--;
            }

            // __aip3368h_display_speed_bit_x__(0, animation_val);
            // __aip3368h_display_speed_bit_x__(1, animation_val);
            for (i = 0; i < 6; i++)
            {
                __aip3368h_display_mileage_bit_x__(i, animation_val);
            }
        }
    }
}

void __aip3368h_display_boot_animation_in_fuel_level__(void)
{
    static u16 animation_step = 0; // 控制动画的步长
    static u8 animation_phase = 0; // 0:渐渐递增，1:保持最高，2:渐渐递减
    static u8 level = 0;

    animation_step++;
    if (0 == animation_phase)
    {
        if (animation_step >= 300)
        {
            animation_step = 0;

            if (level < AIP3368H_DISPLAY_FUEL_LEVEL_4)
            {
                level++;
            }
            else
            {
                animation_phase = 1;
            }

            // aip3368h_display_fuel_level(level);
        }
    }
    else if (1 == animation_phase)
    {
        if (animation_step >= 400)
        {
            animation_step = 0;
            animation_phase = 2;
        }
    }
    else if (2 == animation_phase)
    {
        if (animation_step >= 300)
        {
            animation_step = 0;

            if (level > 0)
            {
                level--;
            }

            // aip3368h_display_fuel_level(level);
        }
    }
}

void aip3368h_display_boot_animation_time_add(void)
{
    if (aip3368h_display_obj.is_in_boot_animiation == 1)
    {
        // 在开机动画中，累加开机动画的时间
        aip3368h_display_boot_animation_time_cnt++;
        aip3368h_display_boot_animation_time_add_flag = 1;
    }
}

// 开机动画处理函数
void aip3368h_display_boot_animation_handle(void)
{
    aip3368h_display_obj.is_in_boot_animiation = 1;
    // aip3368h_display_engine_speed_back_light(); // 点亮背光
    // aip3368h_display_exclamation_point(1); // 点亮感叹号

    // aip3368h_display_mileage_km_icon(1); // 点亮公里的km字样图标
    // aip3368h_display_speed_km_icon(1);   // 点亮速度的km字样图标
    // // 点亮 ODO 、 TRIP 字样的图标
    // aip3368h_display_buff[3] |= (0x01 << 15); // 大计里程 ODO 指示灯，第 0 格
    // aip3368h_display_buff[3] |= (0x01 << 14); // 大计里程 ODO 指示灯，第 1 格
    // aip3368h_display_buff[3] |= (0x01 << 1);  // 小计里程 TRIP 指示灯，第 0 格
    // aip3368h_display_buff[4] |= (0x01 << 15); // 小计里程 TRIP 指示灯，第 1 格
    // aip3368h_display_buff[5] |= (0x01 << 8);  // 里程 小数点 指示灯

    // aip3368h_display_bat_err_icon(1);
    // aip3368h_display_err_icon(1);

    // 显示时速第 0 位的 1：
    // aip3368h_display_buff[2] |= (0x01 << 8);
    // aip3368h_display_buff[7] |= (0x01 << 3);

    // while (aip3368h_display_obj.is_in_boot_animiation)
    // {
    //     WDT_KEY = WDT_KEY_VAL(0xAA); // 喂狗并清除 wdt_pending

    //     if (aip3368h_display_boot_animation_time_add_flag)
    //     {
    //         aip3368h_display_boot_animation_time_add_flag = 0;
    //     }
    //     else
    //     {
    //         continue;
    //     }

    //     __aip3368h_display_boot_animation_in_engine_speed_scale_bar__();
    //     __aip3368h_display_boot_animation_in_speed_scale_bar__();
    //     __aip3368h_display_boot_animation_in_speed_and_mileage__();
    //     __aip3368h_display_boot_animation_in_fuel_level__();

    //     // 4s的开机动画结束
    //     if (aip3368h_display_boot_animation_time_cnt >= 4000)
    //     {
    //         aip3368h_display_obj.is_in_boot_animiation = 0;
    //         // 动画结束后清空显示

    //         // 后面添加了相关的检测和更新函数之后，下面的操作可以省略
    //         // aip3368h_display_engine_speed_scale_bar(0); // 不显示发动机转速刻度条
    //         // aip3368h_display_exclamation_point(0);      // 不显示感叹号

    //         aip3368h_display_bat_err_icon(0);
    //         aip3368h_display_err_icon(0);
    //         aip3368h_display_speed_scale_bar(16);
    //         // USER_TO_DO 需要根据记忆的 ODO、TRIP ，显示对应的图标
    //     }

    //     aip3368h_module_display();
    // }
}

void aip3368h_display_err_handle_time_add(void)
{
    if (aip3368h_display_err_handle_time_cnt < ((u16)-1))
    {
        aip3368h_display_err_handle_time_cnt++;
    }
}
void aip3368h_display_err_handle(void)
{
    if (aip3368h_display_err_handle_time_cnt < 475)
    {
        return;
    }
    else
    {
        aip3368h_display_err_handle_time_cnt = 0;
    }

    // 发动机转速过高报警
    if (instrument.flag_is_engine_speed_warning_enable)
    {
        // 直接操作显存，判断当前感叹号对应的指示灯是否点亮，进而让它闪烁
        if ((aip3368h_display_buff[0] >> 1) & 0x01)
        {
            aip3368h_display_buff[0] &= ~(0x01 << 1);
        }
        else
        {
            aip3368h_display_buff[0] |= (0x01 << 1);
        }
    }

    // 低电量报警
    if (instrument.flag_is_in_warning_of_low_voltage)
    {
        // 直接操作显存，判断当前感叹号对应的指示灯是否点亮，进而让它闪烁
        if ((aip3368h_display_buff[2] >> 10) & 0x01)
        {
            aip3368h_display_buff[2] &= ~(0x01 << 10); // 电池电量低，第 1 格指示灯（红）
            aip3368h_display_buff[2] &= ~(0x01 << 11); // 电池电量低，第 0 格指示灯（红）
        }
        else
        {
            aip3368h_display_buff[2] |= 0x01 << 10; // 电池电量低，第 1 格指示灯（红）
            aip3368h_display_buff[2] |= 0x01 << 11; // 电池电量低，第 0 格指示灯（红）
        }
    }

    // 低油量 报警
    if (instrument.flag_is_in_warning_of_low_fuel)
    {
        // 直接操作显存，判断当前感叹号对应的指示灯是否点亮，进而让它闪烁

        if ((aip3368h_display_buff[2] >> 15) & 0x01)
        {
            aip3368h_display_buff[2] &= ~(0x01 << 15); // 油量，第 0 格指示灯（红）
        }
        else
        {
            aip3368h_display_buff[2] |= (0x01 << 15); // 油量，第 0 格指示灯（红）
        }
    }
}

#if AIP3368H_DISPLAY_TEST_ENABLE

/**
 * @brief 测试指示灯闪烁，放在1ms的中断内调用
 *
 * @return * void
 */
void aip3368h_display_test_light_blink_1ms_isr(void)
{
    static u16 cnt = 0;
    static u8 is_enable = 0;

    cnt++;
    if (cnt < 500)
    {
        return;
    }
    else
    {
        cnt = 0;
    }

    // aip3368h_display_left_turn_light(is_enable);
    // aip3368h_display_right_turn_light(is_enable);
    // aip3368h_display_err_light(is_enable);
    // aip3368h_display_low_beam_indicator_light(is_enable);
    // aip3368h_display_high_beam_indicator_light(is_enable);
    // aip3368h_display_x1000rpm_light(is_enable);
    // aip3368h_display_gear_light(is_enable);
    // aip3368h_display_gear_n_light(is_enable);

    is_enable = !is_enable;
}

/**
 * @brief 测试背光灯刻度条
 *
 */
void aip3368h_display_test_back_light_scale_bar(void)
{
    static u16 cnt = 0;
    static u8 level = 0;

    cnt++;
    if (cnt < 500)
    {
        return;
    }
    else
    {
        cnt = 0;
    }

    aip3368h_display_back_light_scale_bar(level);

    level++;
    if (level >= 34)
    {
        level = 0;
    }
}

/**
 * @brief 测试 发动机转速刻度条
 *
 */
void aip3368h_display_test_engine_speed_scale_bar(void)
{
    static u16 cnt = 0;
    static u8 level = 0;

    cnt++;
    if (cnt < 500)
    {
        return;
    }
    else
    {
        cnt = 0;
    }

    aip3368h_display_engine_speed_scale_bar(level);
    level++;
    if (level >= 25)
    {
        level = 0;
    }
}

/**
 * @brief 测试 发动机转速的数字刻度
 *
 */
void aip3368h_display_test_engine_speed_digit_scale(void)
{
    static u16 cnt = 0;
    static u8 level = 0;

    cnt++;
    if (cnt < 500)
    {
        return;
    }
    else
    {
        cnt = 0;
    }

    aip3368h_display_engine_speed_digit_scale(level);
    level++;
    if (level >= 14)
    {
        level = 0;
    }
}

// 测试 挡位 显示
void aip3368h_display_test_gear(void)
{
    static u16 cnt = 0;
    static u8 level = 0;

    cnt++;
    if (cnt < 500)
    {
        return;
    }
    else
    {
        cnt = 0;
    }

    if (level == 0)
    {
        __aip3368h_display_gear_n_light__(0);
        __aip3368h_display_gear_digit__(0);
    }
    else
    {
        // 最大传入 6
        aip3368h_display_gear(level - 1);
    }

    level++;
    if (level >= 8)
    {
        level = 0;
    }
}

void aip3368h_display_test_speed(void)
{
    static u16 cnt = 0;
    static u8 speed = 0;

    cnt++;
    if (cnt < 200)
    {
        return;
    }
    else
    {
        cnt = 0;
    }

    aip3368h_display_speed(speed);
    speed++;
    if (speed >= 200)
    {
        speed = 0;
    }
}

#if 1
void aip3368h_display_test(void)
{
#if 1
    // aip3368h_display_buff[0] |= 0x01 << 0;  // NC
    // aip3368h_display_buff[0] |= 0x01 << 1;  // NC
    // aip3368h_display_buff[0] |= 0x01 << 2;  // 背光刻度条 第 21 个灯（白）
    // aip3368h_display_buff[0] |= 0x01 << 3;  // 背光刻度条 第 20 个灯（白）
    // aip3368h_display_buff[0] |= 0x01 << 4;  // 发动机转速刻度条 第 23 个 指示灯（红）
    // aip3368h_display_buff[0] |= 0x01 << 5;  // 背光刻度条 第 19 个灯（白）
    // aip3368h_display_buff[0] |= 0x01 << 6;  // 发动机转速刻度条 第 22 个 指示灯（红）
    // aip3368h_display_buff[0] |= 0x01 << 7;  // 发动机转速 "11" 字样 指示灯（红）
    // aip3368h_display_buff[0] |= 0x01 << 8;  // NC 样板上没有该指示灯
    // aip3368h_display_buff[0] |= 0x01 << 9;  // NC 样板上没有该指示灯
    // aip3368h_display_buff[0] |= 0x01 << 10; // NC 样板上没有该指示灯
    // aip3368h_display_buff[0] |= 0x01 << 11; // NC 样板上没有该指示灯
    // aip3368h_display_buff[0] |= 0x01 << 12; // 小灯/近光灯（绿）
    // aip3368h_display_buff[0] |= 0x01 << 13; // NC
    // aip3368h_display_buff[0] |= 0x01 << 14; // NC
    // aip3368h_display_buff[0] |= 0x01 << 15; // NC

    // aip3368h_display_buff[1] |= 0x01 << 0;  // 右转向灯
    // aip3368h_display_buff[1] |= 0x01 << 1;  // 背光刻度条 第 26 个灯（白）
    // aip3368h_display_buff[1] |= 0x01 << 2;  // 背光刻度条 第 25 个灯（白）
    // aip3368h_display_buff[1] |= 0x01 << 3;  // 背光刻度条 第 24 个灯（白）
    // aip3368h_display_buff[1] |= 0x01 << 4;  // NC 样板上没有该指示灯
    // aip3368h_display_buff[1] |= 0x01 << 5; // "x1000r/min"字样 第 2 个灯（白）
    // aip3368h_display_buff[1] |= 0x01 << 6; // "x1000r/min"字样 第 1 个灯（白）
    // aip3368h_display_buff[1] |= 0x01 << 7;  // NC 样板上没有该指示灯
    // aip3368h_display_buff[1] |= 0x01 << 8;  // "TRIP" 字样 指示灯（绿）
    // aip3368h_display_buff[1] |= 0x01 << 9;  // "TOTAL" 字样 指示灯（绿）
    // aip3368h_display_buff[1] |= 0x01 << 10; // "x1000r/min"字样 第 0 个灯（白）
    // aip3368h_display_buff[1] |= 0x01 << 11; // 发动机转速 "12"字样 指示灯（红）
    // aip3368h_display_buff[1] |= 0x01 << 12; // 背光刻度条 第 23 个灯（白）
    // aip3368h_display_buff[1] |= 0x01 << 13; // 背光刻度条 第 22 个灯（白）
    // aip3368h_display_buff[1] |= 0x01 << 14; // 大灯（蓝）
    // aip3368h_display_buff[1] |= 0x01 << 15; // NC

    // aip3368h_display_buff[2] |= 0x01 << 0;  // 挡位 F 段 指示灯（红）
    // aip3368h_display_buff[2] |= 0x01 << 1;  // 挡位 G 段 指示灯（红）
    // aip3368h_display_buff[2] |= 0x01 << 2;  // 挡位 A 段 指示灯（红）
    // aip3368h_display_buff[2] |= 0x01 << 3;  // 挡位 B 段 指示灯（红）
    // aip3368h_display_buff[2] |= 0x01 << 4;  // 背光刻度条 第 27 个灯（白）
    // aip3368h_display_buff[2] |= 0x01 << 5;  // 挡位 "N" 字样指示灯（绿）
    // aip3368h_display_buff[2] |= 0x01 << 6;  // 背光刻度条 第 29 个灯（白）
    // aip3368h_display_buff[2] |= 0x01 << 7;  // 背光刻度条 第 28 个灯（白）
    // aip3368h_display_buff[2] |= 0x01 << 8;  // 背光刻度条 第 30 个灯（白）
    // aip3368h_display_buff[2] |= 0x01 << 9;  // 背光刻度条 第 31 个灯（白）
    // aip3368h_display_buff[2] |= 0x01 << 10; // 背光刻度条 第 32 个灯（白）
    // aip3368h_display_buff[2] |= 0x01 << 11; // 挡位 "GEAR" 字样指示灯（白）
    // aip3368h_display_buff[2] |= 0x01 << 12; // 挡位 C 段 指示灯（红）
    // aip3368h_display_buff[2] |= 0x01 << 13; // 里程 "mile" 字样指示灯（绿）
    // aip3368h_display_buff[2] |= 0x01 << 14; // 挡位 D 段 指示灯（红）
    // aip3368h_display_buff[2] |= 0x01 << 15; // 挡位 E 段 指示灯（红）

    // aip3368h_display_buff[3] |= 0x01 << 0;  // 里程 第 5 位 A 段 指示灯（白）
    // aip3368h_display_buff[3] |= 0x01 << 1; // 里程 第 5 位 F 段 指示灯（白）
    // aip3368h_display_buff[3] |= 0x01 << 2; // 里程 第 5 位 B 段 指示灯（白）
    // aip3368h_display_buff[3] |= 0x01 << 3; // 里程 第 5 位 G 段 指示灯（白）
    // aip3368h_display_buff[3] |= 0x01 << 4;  // 里程 "km" 字样指示灯（绿）
    // aip3368h_display_buff[3] |= 0x01 << 5;  // 里程 第 5 位 C 段 指示灯（白）
    // aip3368h_display_buff[3] |= 0x01 << 6;  // 里程 第 5 位 E 段 指示灯（白）
    // aip3368h_display_buff[3] |= 0x01 << 7;  // 里程 第 5 位 D 段 指示灯（白）
    // aip3368h_display_buff[3] |= 0x01 << 8;  // 里程 小数点 指示灯（白）
    // aip3368h_display_buff[3] |= 0x01 << 9;  // 里程 第 4 位 D 段 指示灯（白）
    // aip3368h_display_buff[3] |= 0x01 << 10; // 里程 第 4 位 E 段 指示灯（白）
    // aip3368h_display_buff[3] |= 0x01 << 11; // 里程 第 4 位 G 段 指示灯（白）
    // aip3368h_display_buff[3] |= 0x01 << 12; // 里程 第 4 位 C 段 指示灯（白）
    // aip3368h_display_buff[3] |= 0x01 << 13; // 里程 第 4 位 F 段 指示灯（白）
    // aip3368h_display_buff[3] |= 0x01 << 14; // 里程 第 4 位 B 段 指示灯（白）
    // aip3368h_display_buff[3] |= 0x01 << 15; // 里程 第 4 位 A 段 指示灯（白）

    // aip3368h_display_buff[4] |= 0x01 << 0;  // 里程 第 3 位 A 段 指示灯（白）
    // aip3368h_display_buff[4] |= 0x01 << 1;  // 里程 第 3 位 B 段 指示灯（白）
    // aip3368h_display_buff[4] |= 0x01 << 2;  // 里程 第 3 位 F 段 指示灯（白）
    // aip3368h_display_buff[4] |= 0x01 << 3;  // 里程 第 3 位 G 段 指示灯（白）
    // aip3368h_display_buff[4] |= 0x01 << 4;  // 里程 第 3 位 C 段 指示灯（白）
    // aip3368h_display_buff[4] |= 0x01 << 5;  // 里程 第 3 位 E 段 指示灯（白）
    // aip3368h_display_buff[4] |= 0x01 << 6;  // 里程 第 2 位 C 段 指示灯（白）
    // aip3368h_display_buff[4] |= 0x01 << 7;  // 里程 第 3 位 D 段 指示灯（白）
    // aip3368h_display_buff[4] |= 0x01 << 8;  // 里程 第 2 位 D 段 指示灯（白）
    // aip3368h_display_buff[4] |= 0x01 << 9;  // 里程 第 2 位 E 段 指示灯（白）
    // aip3368h_display_buff[4] |= 0x01 << 10; // 里程 第 2 位 G 段 指示灯（白）
    // aip3368h_display_buff[4] |= 0x01 << 11; // 里程 第 1 位 B 段 指示灯（白）
    // aip3368h_display_buff[4] |= 0x01 << 12; // 里程 第 1 位 A 段 指示灯（白）
    // aip3368h_display_buff[4] |= 0x01 << 13; // 里程 第 2 位 F 段 指示灯（白）
    // aip3368h_display_buff[4] |= 0x01 << 14; // 里程 第 2 位 B 段 指示灯（白）
    // aip3368h_display_buff[4] |= 0x01 << 15; // 里程 第 2 位 A 段 指示灯（白）

    // aip3368h_display_buff[5] |= 0x01 << 0;  // 里程 第 1 位 G 段 指示灯（白）
    // aip3368h_display_buff[5] |= 0x01 << 1;  // 里程 第 1 位 F 段 指示灯（白）
    // aip3368h_display_buff[5] |= 0x01 << 2;  // 里程 第 0 位 A 段 指示灯（白）
    // aip3368h_display_buff[5] |= 0x01 << 3;  // 里程 第 0 位 B 段 指示灯（白）
    // aip3368h_display_buff[5] |= 0x01 << 4;  // 里程 第 0 位 F 段 指示灯（白）
    // aip3368h_display_buff[5] |= 0x01 << 5;  // 里程 第 1 位 C 段 指示灯（白）
    // aip3368h_display_buff[5] |= 0x01 << 6;  // 里程 第 1 位 E 段 指示灯（白）
    // aip3368h_display_buff[5] |= 0x01 << 7;  // 里程 第 1 位 D 段 指示灯（白）
    // aip3368h_display_buff[5] |= 0x01 << 8;  // NC 样板上没有该指示灯
    // aip3368h_display_buff[5] |= 0x01 << 9;  // 发动机转速 "0" 字样 指示灯（白）
    // aip3368h_display_buff[5] |= 0x01 << 10; // 时速 "km/h" 字样 指示灯（绿）
    // aip3368h_display_buff[5] |= 0x01 << 11; // 时速 "mph" 字样 指示灯（绿）
    // aip3368h_display_buff[5] |= 0x01 << 12; // 里程 第 0 位 D 段 指示灯（白）
    // aip3368h_display_buff[5] |= 0x01 << 13; // 里程 第 0 位 E 段 指示灯（白）
    // aip3368h_display_buff[5] |= 0x01 << 14; // 里程 第 0 位 C 段 指示灯（白）
    // aip3368h_display_buff[5] |= 0x01 << 15; // 里程 第 0 位 G 段 指示灯（白）

    // aip3368h_display_buff[6] |= 0x01 << 0;  // NC 样板上没有该指示灯
    // aip3368h_display_buff[6] |= 0x01 << 1;  // 时速 第 2 位 指示灯 A 段（白）
    // aip3368h_display_buff[6] |= 0x01 << 2;  // 发动机转速 "10" 字样 指示灯（红）
    // aip3368h_display_buff[6] |= 0x01 << 3;  // 时速 第 1 位 指示灯 A 段（白）
    // aip3368h_display_buff[6] |= 0x01 << 4;  // 发动机转速刻度条 第 17 个 指示灯（白）
    // aip3368h_display_buff[6] |= 0x01 << 5;  // 发动机转速刻度条 第 18 个 指示灯（白）
    // aip3368h_display_buff[6] |= 0x01 << 6;  // 发动机转速刻度条 第 19 个 指示灯（白）
    // aip3368h_display_buff[6] |= 0x01 << 7;  // 发动机转速 "9" 字样 指示灯（白）
    // aip3368h_display_buff[6] |= 0x01 << 8;  // 左转向灯（绿）
    // aip3368h_display_buff[6] |= 0x01 << 9;  // 背光刻度条 第 16 个灯（白）
    // aip3368h_display_buff[6] |= 0x01 << 10; // 背光刻度条 第 15 个灯（白）
    // aip3368h_display_buff[6] |= 0x01 << 11; // 故障指示灯（红）
    // aip3368h_display_buff[6] |= 0x01 << 12; // 发动机转速刻度条 第 20 个灯（红）
    // aip3368h_display_buff[6] |= 0x01 << 13; // 背光刻度条 第 17 个灯（白）
    // aip3368h_display_buff[6] |= 0x01 << 14; // 发动机转速刻度条 第 21 个灯（红）
    // aip3368h_display_buff[6] |= 0x01 << 15; // 背光刻度条 第 18 个灯（白）

    // aip3368h_display_buff[7] |= 0x01 << 0;  // 时速 第 2 位 指示灯 F 段（白）
    // aip3368h_display_buff[7] |= 0x01 << 1;  // 时速 第 2 位 指示灯 E 段（白）
    // aip3368h_display_buff[7] |= 0x01 << 2;  // 时速 第 2 位 指示灯 G 段（白）
    // aip3368h_display_buff[7] |= 0x01 << 3; // 时速 第 2 位 指示灯 B 段（白）
    // aip3368h_display_buff[7] |= 0x01 << 4; // 时速 第 2 位 指示灯 C 段（白）
    // aip3368h_display_buff[7] |= 0x01 << 5;  // 时速 第 2 位 指示灯 D 段（白）
    // aip3368h_display_buff[7] |= 0x01 << 6;  // 发动机转速 "1" 字样 指示灯（白）
    // aip3368h_display_buff[7] |= 0x01 << 7; // 发动机转速 "2" 字样 指示灯（白）
    // aip3368h_display_buff[7] |= 0x01 << 8; // 发动机转速 "3" 字样 指示灯（白）
    // aip3368h_display_buff[7] |= 0x01 << 9;  // 时速 第 1 位 指示灯 D 段（白）
    // aip3368h_display_buff[7] |= 0x01 << 10; // NC
    // aip3368h_display_buff[7] |= 0x01 << 11; // 时速 第 1 位 指示灯 E 段（白）
    // aip3368h_display_buff[7] |= 0x01 << 12; // 时速 第 1 位 指示灯 C 段（白）
    // aip3368h_display_buff[7] |= 0x01 << 13; // 时速 第 1 位 指示灯 G 段（白）
    // aip3368h_display_buff[7] |= 0x01 << 14; // 时速 第 1 位 指示灯 F 段（白）
    // aip3368h_display_buff[7] |= 0x01 << 15; // 时速 第 1 位 指示灯 B 段（白）

    // aip3368h_display_buff[8] |= 0x01 << 0;  // 发动机转速 "7" 字样 指示灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 1;  // 发动机转速刻度条 第 11 个灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 2;  // 背光刻度条 第 7 个灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 3;  // 发动机转速刻度条 第 12 个灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 4;  // 背光刻度条 第 8 个灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 5;  // 发动机转速刻度条 第 13 个灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 6;  // 背光刻度条 第 10 个灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 7;  // 背光刻度条 第 9 个灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 8;  // 发动机转速刻度条 第 14 个灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 9;  // 背光刻度条 第 11 个灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 10; // 背光刻度条 第 12 个灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 11; // 发动机转速刻度条 第 15 个灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 12; // 背光刻度条 第 13 个灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 13; // 背光刻度条 第 14 个灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 14; // 发动机转速刻度条 第 16 个灯（白）
    // aip3368h_display_buff[8] |= 0x01 << 15; // 发动机转速 "8" 字样 指示灯（白）

    // aip3368h_display_buff[9] |= 0x01 << 0;  // 发动机转速刻度条 第 6 个灯（白）
    // aip3368h_display_buff[9] |= 0x01 << 1;  // 发动机转速 "4" 字样 指示灯（白）
    // aip3368h_display_buff[9] |= 0x01 << 2;  // 背光刻度条 第 2 个灯（白）
    // aip3368h_display_buff[9] |= 0x01 << 3;  // 发动机转速刻度条 第 7 个灯（白）
    // aip3368h_display_buff[9] |= 0x01 << 4;  // 背光刻度条 第 3 个灯（白）
    // aip3368h_display_buff[9] |= 0x01 << 5;  // 发动机转速刻度条 第 8 个灯（白）
    // aip3368h_display_buff[9] |= 0x01 << 6;  // 油量 "E" 字样 指示灯（红）
    // aip3368h_display_buff[9] |= 0x01 << 7;  // 背光刻度条 第 4 个灯（白）
    // aip3368h_display_buff[9] |= 0x01 << 8;  // 发动机转速刻度条 第 10 个灯（白）
    // aip3368h_display_buff[9] |= 0x01 << 9;  // 背光刻度条 第 6 个灯（白）
    // aip3368h_display_buff[9] |= 0x01 << 10; // 背光刻度条 第 5 个灯（白）
    // aip3368h_display_buff[9] |= 0x01 << 11; // 发动机转速刻度条 第 9 个灯（白）
    // aip3368h_display_buff[9] |= 0x01 << 12; // 发动机转速 "5" 字样 指示灯（白）
    // aip3368h_display_buff[9] |= 0x01 << 13; // 发动机转速 "6" 字样 指示灯（白）
    // aip3368h_display_buff[9] |= 0x01 << 14; // 时速 第 0 位 指示灯 B 段（白）
    // aip3368h_display_buff[9] |= 0x01 << 15; // 时速 第 0 位 指示灯 C 段（白）

    // aip3368h_display_buff[10] |= 0x01 << 0; // 发动机转速刻度条 第 5 个灯（白）
    // aip3368h_display_buff[10] |= 0x01 << 1; // 发动机转速刻度条 第 4 个灯（白）
    // aip3368h_display_buff[10] |= 0x01 << 2;  // 背光刻度条 第 1 个灯（白）
    // aip3368h_display_buff[10] |= 0x01 << 3;  // 发动机转速刻度条 第 3 个灯（白）
    // aip3368h_display_buff[10] |= 0x01 << 4;  // 发动机转速刻度条 第 2 个灯（白）
    // aip3368h_display_buff[10] |= 0x01 << 5;  // 背光刻度条 第 0 个灯（白）
    // aip3368h_display_buff[10] |= 0x01 << 6;  // 发动机转速刻度条 第 1 个灯（白）
    // aip3368h_display_buff[10] |= 0x01 << 7;  // 发动机转速刻度条 第 0 个灯（白）
    // aip3368h_display_buff[10] |= 0x01 << 8;  // 油量 "F" 字样 指示灯（白）
    // aip3368h_display_buff[10] |= 0x01 << 9;  // 油量 第 5 格 指示灯（白）
    // aip3368h_display_buff[10] |= 0x01 << 10; // 油量 第 4 格 指示灯（白）
    // aip3368h_display_buff[10] |= 0x01 << 11; // 油量 第 3 格 指示灯（白）
    // aip3368h_display_buff[10] |= 0x01 << 12; // 油量 第 2 格 指示灯（白）
    // aip3368h_display_buff[10] |= 0x01 << 13; // 油量 图标 （红）
    // aip3368h_display_buff[10] |= 0x01 << 14; // 油量 第 1 格 指示灯（白）
    // aip3368h_display_buff[10] |= 0x01 << 15; // 油量 第 0 格 指示灯（红）
#endif

// 所有灯整体闪烁
#if 0
    static u8 dir = 0;
    static u16 cnt = 0;
    cnt++;

    if (cnt >= 500)
    {
        cnt = 0;
    }
    else
    {
        return;
    }

    if (dir == 0)
    {
        memset(aip3368h_display_buff, 0x00, sizeof(aip3368h_display_buff));
        dir = 1;
    }
    else
    {
        memset(aip3368h_display_buff, (u8)0xFF, sizeof(aip3368h_display_buff));
        dir = 0;
    }
#endif
}
#endif
#endif
