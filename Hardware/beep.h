#ifndef __BEEP_H__
#define __BEEP_H__

#include "include.h"

#define BEEP_PIN P24
#define BEEP_ON() (BEEP_PIN = 1)
#define BEEP_OFF() (BEEP_PIN = 0)

void beep_init(void);
void beep_play(u8 beep_time);
void beep_handle_1ms_isr(void);

#endif
