/* 控制台输出 */

#ifndef _LINUX_CONSOLE_H
#define _LINUX_CONSOLE_H

#include <asm-i386/vga.h>
#include <asm-i386/types.h>

// 显存的地址
static uint16_t *video_memory = (uint16_t *) 0xB8000;

void print_char(int ch);

void print_str(char *message);

void print_int(int num);

void print_hex(unsigned int num);

void set_cursor(uint16_t cursor_loc);

void flush_screen();

void scroll_screen(int direction);

// 设置光标在屏幕中的位置
void console_set_cursor(uint32_t cursor_pos);
// 清屏
void console_clear();
// 屏幕打印一个字符串，默认是黑底白色
void console_write(char *cstr);

// 屏幕打印一个指定颜色的字符串
void console_write_with_color(char *cstr, real_color_t back, real_color_t fore);

// 在屏幕上打印一个十六进制的整型数
void console_with_hex(uint32_t n, real_color_t back, real_color_t fore);



#endif
