#include <linux/console.h>
#include <asm-i386/vga.h>
#include <asm-i386/io.h>
#include <linux/string.h>
#include <asm-i386/types.h>

// 屏幕的光标位置，默认是 80 * 25的二维数组,一个字符显示为 2 个字节，一个表示值，一个表示属性
uint16_t cursor_x = 0;          // 当前所在的行坐标
uint16_t cursor_y = 0;          // 当前所在的列坐标

/**
 * @brief 在控制台上输出字符
 * 
 * @param ch 要输出的字符
 */
void print_char(int ch)
{  
    // 获取当前的光标位置
    char* vram = (char *)(V_MEM_BASE + (cursor_x * SCREEN_WIDTH + cursor_y) * 2);

    // 区别特殊的字符
    switch(ch){
        
        case '\r':      // 回车
            break;
        case '\n':      // 换行
            *vram++ = '\0';
            *vram = COLOR_DEFAULT;      // 默认黑底白字
            cursor_y = 0;
            cursor_x++;
            break;

        case '\b':      // 退格
            if(cursor_x >= 0 & cursor_y >= 0){
                cursor_x --;
                /* 调整为上一行的行末 */
                if(cursor_y < 0){
                    cursor_y = SCREEN_WIDTH - 1;
                    cursor_x--;
                    // 调整行数
                    if(cursor_x < 0) {
                        cursor_x = 0;
                    }
                }
                *(vram-2) = '\0';
                *(vram-1) = COLOR_DEFAULT;
            }
            break;

        default:        // 其他默认的字符
            *vram++ = ch;
            *vram = COLOR_DEFAULT;

            cursor_y++;
            if(cursor_y > SCREEN_WIDTH - 1){
                cursor_y = 0;
                cursor_x++;
            }
            break;
    }
    
    /* 滚屏，调整光标位置 */
    while(cursor_x > SCREEN_HEIGHT - 1){
        scroll_screen(SCREEN_DOWN);
    }

    flush_screen();
}

/**
 * @brief 在控制台上输出字符串
 * 
 * @param str 
 */
void print_str(char *str)
{
    while(*str){
        print_char(*str);
        str++;
    }
}

/**
 * @brief 在控制台输出int类型数据
 * 
 * @param num 
 */
void print_int(int num)
{
    char buf[24];
    memset(buf, 0, 24);
    itoa(num, buf, 10);
    print_str(buf);
}

void print_hex(unsigned int num)
{
    char buf[24];
    memset(buf, 0, 24);
    print_str("0x");
    uitoa(num, buf, 16);
    print_str(buf);
}

/** 
 * 设置光标在屏幕中的位置
 */ 
void set_cursor(uint16_t cursor_loc)
{
    uint16_t cursor = cursor_x * SCREEN_WIDTH + cursor_y;
    // 光标位置为 14和15号内部寄存器，分别是高8位和低8位
    outb(0x3D4, 14); 
    outb(0x3D5, (cursor >> 8) & 0xFF);   // 写入位置的高8位
    outb(0x3D4, 15);
    outb(0x3D5, cursor & 0xFF);       // 写入位置的低8位
}

/**
 * @brief 刷新屏幕和光标的起始位置
 * 
 */
void flush_screen()
{
    set_cursor(cursor_x * SCREEN_WIDTH + cursor_y);
}

/**
 * @brief 滚动屏幕，将24行的数据全部挪动一行，最后一行清空
 * @param direction 表示滚动的方向, SCREEN_UP 向上， SCREEN_DOWN 向下
 */
void scroll_screen(int direction)
{  
    unsigned char *vram = (unsigned char *)(V_MEM_BASE);
    int i;

    
    if(direction == SCREEN_UP){
        // 从底部开始，复制当前行的上一行内容
        for(i = SCREEN_WIDTH * 2 * 24; i > SCREEN_WIDTH * 2; i -= 2){
            vram[i] = vram[i - SCREEN_WIDTH * 2];
            vram[i + 1] = vram[i - SCREEN_WIDTH * 2 + 1];
        }
        // 清空第一行
        for(i = 0; i < SCREEN_WIDTH * 2; i += 2 ){
            vram[i] = '\0';
            vram[i + 1] = COLOR_DEFAULT;
        }
    }
    else if(direction == SCREEN_DOWN){
        // 从顶部开始，复制当前行的下一行内容
         for( i = 0; i < SCREEN_WIDTH * 2 * 24; i += 2){
             vram[i] = vram[i + SCREEN_WIDTH * 2];
             vram[i + 1] = vram[i + 1 + SCREEN_WIDTH * 2];
         }
         // 清空最后一行
         for (i = SCREEN_WIDTH * 2 * 24; i < SCREEN_WIDTH * 2 * 25; i += 2) {
            vram[i] = '\0';
            vram[i + 1] = COLOR_DEFAULT;
        }
        cursor_x--;     // 当前所在行
    }
    flush_screen();
}


/**
 * @brief 清除屏幕，全部覆盖为空即可
 * 
 */
void console_clear()
{
    // 利用 白底黑字 直接覆盖整个屏幕
    unsigned char *vram = (unsigned char *)(V_MEM_BASE);
    uint16_t blank = MAKE_COLOR(TEXT_WHITE, TEXT_BLACK);
    int i;

    for(i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++ ){
            vram[i] = blank;
    }

    set_cursor(cursor_x * 80 + cursor_y);
}

// 屏幕打印一个字符串，默认是黑底白色
void console_write(char *cstr)
{

}

// 屏幕打印一个指定颜色的字符串
void console_write_with_color(char *cstr, real_color_t back, real_color_t fore)
{

}

// 在屏幕上打印一个十六进制的整型数
void console_with_hex(uint32_t n, real_color_t back, real_color_t fore)
{

}
