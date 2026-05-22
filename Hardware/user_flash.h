#ifndef __USER_FLASH_H__
#define __USER_FLASH_H__

#include "include.h"
#include "my_config.h"

#if USE_INTERNAL_FLASH_SAVE_DATA

#define FLASH_START_ADDR (0x00) // 起始地址128byte对齐

#define USER_FLASH_DATA_VALID_VAL ((u8)0xC5)

void flash_erase_sector(u8 addr);
void flash_program(u8 addr, u8 *p_data, u8 len);
void flash_read(u8 addr, u8 *p_data, u8 len);

#endif
#endif
