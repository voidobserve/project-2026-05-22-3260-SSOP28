#include "beep.h"
#include "include.h"

static volatile u8 beep_time_cnt = 0;
static volatile u8 is_beep_enable = 0;

void beep_init(void)
{
	// 蜂鸣器（高电平驱动）
	P2_MD1 &= ~GPIO_P24_MODE_SEL(0x03);
	P2_MD1 |= GPIO_P24_MODE_SEL(0x01); // 输出模式
	FOUT_S24 = GPIO_FOUT_AF_FUNC;
	P24 = 0;
}

/**
 * @brief 让蜂鸣器鸣叫一声，到时间后停止
 *
 */
void beep_play(u8 beep_time)
{
	is_beep_enable = 0;
	beep_time_cnt = beep_time;
	is_beep_enable = 1;
}

void beep_handle_1ms_isr(void)
{
	if (is_beep_enable)
	{
		if (beep_time_cnt)
		{
			beep_time_cnt--;
			BEEP_ON();
		}
		else
		{
			BEEP_OFF();
			is_beep_enable = 0;
		}
	}
	else
	{
		BEEP_OFF();
		beep_time_cnt = 0;
	}
}
