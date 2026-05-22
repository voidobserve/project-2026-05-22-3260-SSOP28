#include "uart0.h"

#if USER_DEBUG_ENABLE
// 设置的波特率需要适配单片机的时钟，这里直接使用了官方的代码
#define USER_UART0_BAUD ((SYSCLK - UART0_BAUDRATE) / (UART0_BAUDRATE))

#if 1 // 将uart0用作串口打印
// 重写putchar()函数
extern void uart0_send_byte(u8 byte); // 函数声明
char putchar(char c)
{
    uart0_send_byte(c);
    return c;
}
#endif // 将uart0用作串口打印

void uart0_debug_init(void)
{ 
    P0_MD0 &= ~(GPIO_P00_MODE_SEL(0x03)); // 清空寄存器配置
    P0_MD0 |= GPIO_P00_MODE_SEL(0x01);    // 输出模式
    FOUT_S00 |= GPIO_FOUT_UART0_TX;       // 配置为UART0_TX

    UART0_BAUD1 = (USER_UART0_BAUD >> 8) & 0xFF; // 配置波特率高八位
    UART0_BAUD0 = USER_UART0_BAUD & 0xFF;        // 配置波特率低八位
    UART0_CON0 = UART_STOP_BIT(0x0) |            // 1bit停止位
                 UART_EN(0x1);                   // 使能 uart
}

void uart0_send_byte(u8 byte)
{
    // 等待之前的数据发送完成
    while (!(UART0_STA & UART_TX_DONE(0x01)))
        ;
    UART0_DATA = byte;
}

#endif  
