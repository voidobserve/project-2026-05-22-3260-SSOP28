// 定时器TMR2的驱动头文件
#ifndef __TMR2_H
#define __TMR2_H

#include "include.h" // 包含芯片官方提供的头文件
#include "my_config.h"

// extern volatile u32 tmr2_cnt; // 定时器TMR2的计数值（每次在中断服务函数中会加一）

extern void tmr2_config(void);
// extern void tmr2_enable(void);
// extern void tmr2_disable(void);

#endif
