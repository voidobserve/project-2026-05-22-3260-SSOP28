#ifndef __UART0_H__
#define __UART0_H__

#include "include.h" // 使用芯片官方提供的头文件
#include "my_config.h" // 包含自定义的头文件
 
// 串口0波特率
#define UART0_BAUDRATE (115200)

#if USER_DEBUG_ENABLE
void uart0_debug_init(void);
#endif

#endif
