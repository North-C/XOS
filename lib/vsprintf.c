#include<linux/stdio.h>
#include <linux/ctype.h>
#include <asm-i386/div64.h>
#include <linux/string.h>


#define ZEROPAD  1   // 用0填充
#define SIGN     2   // 有符号或无符号long
#define PLUS     4   // 显示 `+`
#define SPACE    8   // 
#define LEFT     16  // 左对齐
#define SPECIAL  32  // 0x
#define LARGE    64  // 使用大写 如ABCDEF




/*
	buf 
	base 数字的基数/进制 
	num 
	size 字符串的长度
*/
static char * number(char * buf, char * end, long long num, int base, int size, int precision, int type)
{
	char c, sign, tmp[66];
	const char *digits;		// 大写还是小写
	static const char small_digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	static const char large_digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i;

	digits = (type & LARGE) ? large_digits : small_digits;
	if(type & LEFT)		// 左对齐,则不需要用0填充
		type &= ~ZEROPAD;
	if(base < 2 || base > 36)  // 基数不合适
		return 0;

	c = (type & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (type & SIGN) { 			// 符号显示
		if (num < 0) {
			sign = '-';
			num = -num;
			size--;
		} else if (type & PLUS) {
			sign = '+';
			size--;
		} else if (type & SPACE) {
			sign = ' ';
			size--;
		}
	}
	if (type & SPECIAL) {  // 16进制的前缀
		if (base == 16)
			size -= 2;
		else if (base == 8)
			size--;
	}

	i = 0;
	if (num == 0)
		tmp[i++]='0';
	else while (num != 0)    // 只要除数不为0，就一直除下去
		tmp[i++] = digits[do_div(num,base)];

	if (i > precision)     
		precision = i;
	size -= precision;
	if (!(type&(ZEROPAD+LEFT))) {
		while(size-->0) {
			if (buf <= end)
				*buf = ' ';
			++buf;
		}
	}
	if (sign) {
		if (buf <= end)
			*buf = sign;
		++buf;
	}
	if (type & SPECIAL) {
		if (base==8) {
			if (buf <= end)
				*buf = '0';
			++buf;
		} else if (base==16) {
			if (buf <= end)
				*buf = '0';
			++buf;
			if (buf <= end)
				*buf = digits[33];
			++buf;
		}
	}
	if (!(type & LEFT)) {
		while (size-- > 0) {
			if (buf <= end)
				*buf = c;
			++buf;
		}
	}
	while (i < precision--) {
		if (buf <= end)
			*buf = '0';
		++buf;
	}
	while (i-- > 0) {
		if (buf <= end)
			*buf = tmp[i];
		++buf;
	}
	while (size-- > 0) {
		if (buf <= end)
			*buf = ' ';
		++buf;
	}
	return buf;
}


// 字符串 转换为 整数
static int skip_atoi(const char **s)
{
	int i = 0;

	while(isdigit(**s)){
		i = i*10 + *((*s)++) - '0';
	}
	return i;
}


/* 类型转换：字符串变为 有符号long型
* cp： 字符串起始位置
* endp: 指向被解析字符串的末尾位置
* base: 进制基数
*/ 
long simple_strtol(const char* cp, char **endp, unsigned int base)
{
	if(*cp=='-'){
		return -simple_strtoul(cp+1, endp, base);
	}
	return simple_strtoul(cp, endp, base);
}

unsigned long simple_strtoul(const char* cp, char **endp, unsigned int base)
{
	unsigned long result = 0, value;

	if(!base){		// 默认进制设置为 10
		base = 10;
		if(*cp == '0'){		// 8进制 或者 16进制
			base = 8;
			cp++;
			if((*cp == 'x') && isxdigit(cp[1])){
				cp++;
				base = 16;
			}
		}
	}
	// 
	while(isxdigit(*cp) && 
			(value = isdigit(*cp) ? *cp-'0' : toupper(*cp) -'A' + 10) < base){
		result = result * base + value;
		cp++;
	}
	if(endp)
		*endp = (char *)cp;
	return result;
}

// 用户格式化输出
int printf(const char* format, ...)
{
    char buf[MAX_LENGTH] = {0};
    va_list ap;
    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);
    return 0;                // 输出的方式
}

/* 将format格式化，填充到buf中 */
int sprintf(char *buf, const char* format, ...)
{
    va_list args;
	int i;

	va_start(args, format);
	i=vsprintf(buf, format, args);  // 调用vsprintf函数
	va_end(args);
	return i;
}

/* 构建一个字符串，装入大小为size的缓冲区buf当中 */
int snprintf(char * buf, size_t size, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i=vsnprintf(buf,size,fmt,args);     // 调用基础的vsnprintf函数
	va_end(args);
	return i;
}


int vsprintf(char *buf, const char* format, va_list args)
{
    return vsnprintf(buf, 0xFFFFFFFFUL, format, args);
}

/* 
 * 可变参数的格式化字符串构造
 * buf: 存放结果的缓存区 
 * size: 缓存区大小
 * fmt 格式化字符串
 * args: 可变参数指针
 */
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
	int len;
	unsigned long long num;
	int i, base;
	char *str, *end, c;		// 分别指向buf
	const char *s;

	int flags;		/* number()函数中与数字类型相关的flags  */

	int field_width;	/* 输出域的宽度 */
	int precision;		/* 整数值的最小位；字符串中的最大字符数 */

	int qualifier;	// 限定符，表明类型	/* 'h', 'l', or 'L' for integer fields */
				/* 'z' support added 23/7/1999 S.H.    */
				/* 'z' changed to 'Z' --davidm 1/25/99 */

	str = buf;
	end = buf + size - 1;

	if (end < buf - 1) {
		end = ((void *) -1);
		size = end - buf + 1;
	}

	for (; *fmt ; ++fmt) {
		if (*fmt != '%') {
			if (str <= end)
				*str = *fmt;
			++str;
			continue;
		}

		/* process flags */
		flags = 0;
		repeat:
			++fmt;		/* this also skips first '%' */
			switch (*fmt) {
				case '-': flags |= LEFT; goto repeat;
				case '+': flags |= PLUS; goto repeat;
				case ' ': flags |= SPACE; goto repeat;
				case '#': flags |= SPECIAL; goto repeat;
				case '0': flags |= ZEROPAD; goto repeat;
			}

		/* get field width */
		field_width = -1;
		if (isdigit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {
			++fmt;
			/* it's the next argument */
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision */
		precision = -1;
		if (*fmt == '.') {
			++fmt;	
			if (isdigit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*') {
				++fmt;
				/* it's the next argument */
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}

		/* get the conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' ||
		    *fmt =='Z' || *fmt == 'z') {
			qualifier = *fmt;
			++fmt;
			if (qualifier == 'l' && *fmt == 'l') {
				qualifier = 'L';
				++fmt;
			}
		}

		/* default base */
		base = 10;

		switch (*fmt) {
			case 'c':
				if (!(flags & LEFT)) {
					while (--field_width > 0) {
						if (str <= end)
							*str = ' ';
						++str;
					}
				}
				c = (unsigned char) va_arg(args, int);
				if (str <= end)
					*str = c;
				++str;
				while (--field_width > 0) {
					if (str <= end)
						*str = ' ';
					++str;
				}
				continue;

			case 's':
				s = va_arg(args, char *);
				if (!s)
					s = "<NULL>";

				len = strnlen(s, precision);

				if (!(flags & LEFT)) {
					while (len < field_width--) {
						if (str <= end)
							*str = ' ';
						++str;
					}
				}
				for (i = 0; i < len; ++i) {
					if (str <= end)
						*str = *s;
					++str; ++s;
				}
				while (len < field_width--) {
					if (str <= end)
						*str = ' ';
					++str;
				}
				continue;

			case 'p':
				if (field_width == -1) {
					field_width = 2*sizeof(void *);
					flags |= ZEROPAD;
				}
				str = number(str, end,
						(unsigned long) va_arg(args, void *),
						16, field_width, precision, flags);
				continue;


			case 'n':
				/* FIXME:
				* What does C99 say about the overflow case here? */
				if (qualifier == 'l') {
					long * ip = va_arg(args, long *);
					*ip = (str - buf);
				} else if (qualifier == 'Z' || qualifier == 'z') {
					size_t * ip = va_arg(args, size_t *);
					*ip = (str - buf);
				} else {
					int * ip = va_arg(args, int *);
					*ip = (str - buf);
				}
				continue;

			case '%':
				if (str <= end)
					*str = '%';
				++str;
				continue;

				/* integer number formats - set up the flags and "break" */
			case 'o':
				base = 8;
				break;

			case 'X':
				flags |= LARGE;
			case 'x':
				base = 16;
				break;

			case 'd':
			case 'i':
				flags |= SIGN;
			case 'u':
				break;

			default:
				if (str <= end)
					*str = '%';
				++str;
				if (*fmt) {
					if (str <= end)
						*str = *fmt;
					++str;
				} else {
					--fmt;
				}
				continue;
		}
		if (qualifier == 'L')
			num = va_arg(args, long long);
		else if (qualifier == 'l') {
			num = va_arg(args, unsigned long);
			if (flags & SIGN)
				num = (signed long) num;
		} else if (qualifier == 'Z' || qualifier == 'z') {
			num = va_arg(args, size_t);
		} else if (qualifier == 'h') {
			num = (unsigned short) va_arg(args, int);
			if (flags & SIGN)
				num = (signed short) num;
		} else {
			num = va_arg(args, unsigned int);
			if (flags & SIGN)
				num = (signed int) num;
		}
		str = number(str, end, num, base,
				field_width, precision, flags);
	}
	if (str <= end)
		*str = '\0';
	else if (size > 0)
		/* don't write out a null byte if the buf size is zero */
		*end = '\0';
	/* the trailing null byte doesn't count towards the total
	* ++str;
	*/
	return str-buf;
}
