#include "lib.h"

#define 	LP_MAX_BUF 100
void user_lp_Print(void (*output)(void *, char *, int), void * arg, char *fmt, __builtin_va_list ap);

static void user_myoutput(void *arg, char *s, int l)
{
    int i;

    if ((l==1) && (s[0] == '\0'))
        return;

    for (i=0; i< l; i++)
        syscall_putchar(s[i]);
}

void writef(char *fmt, ...)
{
    __builtin_va_list ap;
    __builtin_va_start(ap, fmt);
    user_lp_Print(user_myoutput, 0, fmt, ap);
    __builtin_va_end(ap);
}

/* macros */
#define     IsDigit(x)  ( ((x) >= '0') && ((x) <= '9') )
#define     Ctod(x)     ( (x) - '0')

/* forward declaration */
extern int user_PrintChar(char *, char, int, int);
extern int user_PrintString(char *, char *, int, int);
extern int user_PrintNum(char *, unsigned long, int, int, int, int, char, int);

/* private variable */
static const char theFatalMsg[] = "fatal error in lp_Print!";

/* -*-
 * A low level printf() function.
 */

void
user_lp_Print(void (*output)(void *, char *, int), void * arg, char *fmt, __builtin_va_list ap) {

#define     OUTPUT(arg, s, l)  \
  { if (((l) < 0) || ((l) > LP_MAX_BUF)) { \
       (*output)(arg, (char*)theFatalMsg, sizeof(theFatalMsg)-1); for(;;); \
    } else { \
      (*output)(arg, s, l); \
    } \
  }

    char buf[LP_MAX_BUF];

    char c;
    char *s;
    long int num;



    int longFlag;
    //int negFlag;
    int width;
    int prec;
    int ladjust;
    char padc = '0';
    int length;

    while(1) {

	s = fmt;
        /* scan for the next '%' */
        while (*fmt != '%' && *fmt != '\0') {
		OUTPUT(arg, fmt, 1);
        	fmt++;
        }
        /* flush the string found so far */
        /* check "are we hitting the end?" */
        if (*fmt == '\0') {
        	break;
        }

    	/* we found a '%' */
    	fmt++;
    	/* check for long */
    	width = 0;
    	ladjust = 0;
    	if (*fmt == '-' || *fmt == '0') {
        	if (*fmt == '-') {
        		ladjust = 1;
        	}
        	fmt++;
        	while (*fmt >= '0' && *fmt <= '9') {
        		width = (width << 3) + (width << 1) + *fmt - '0';
        		fmt++;
        	}
    	}
    	prec = 0;
    	if (*fmt == '.') {
        	fmt++;
        	while (*fmt >= '0' && *fmt <= '9') {
            	width = (width << 3) + (width << 1) + *fmt - '0';
        		fmt++;
        	}
    	}
    	longFlag = 0;
    	if (*fmt == 'l') {
        	longFlag = 1;
        	fmt++;
    	}

   		switch (*fmt) {
     		case 'b':
				if (longFlag) {
					num = __builtin_va_arg(ap, long int);
				} else {
					num = __builtin_va_arg(ap, int);
				}
				length = user_PrintNum(buf, num, 2, 0, width, ladjust, padc, 0);
				OUTPUT(arg, buf, length);
				break;

			 case 'd':
			 case 'D':
				if (longFlag) {
					num = __builtin_va_arg(ap, long int);
				} else {
					num = __builtin_va_arg(ap, int);
				}

				if (num < 0) {
					length = user_PrintNum(buf, -num, 10, 1, width, ladjust, padc, 0);
				} else {
					length = user_PrintNum(buf, num, 10, 0, width, ladjust, padc, 0);
				}
				OUTPUT(arg, buf, length);
				break;

			 case 'o':
			 case 'O':
				if (longFlag) {
					num = __builtin_va_arg(ap, long int);
				} else {
					num = __builtin_va_arg(ap, int);
				}
				length = user_PrintNum(buf, num, 8, 0, width, ladjust, padc, 0);
				OUTPUT(arg, buf, length);
				break;

			 case 'u':
			 case 'U':
				if (longFlag) {
					num = __builtin_va_arg(ap, long int);
				} else {
					num = __builtin_va_arg(ap, int);
				}
				length = user_PrintNum(buf, num, 10, 0, width, ladjust, padc, 0);
				OUTPUT(arg, buf, length);
				break;

			 case 'x':
				if (longFlag) {
					num = __builtin_va_arg(ap, long int);
				} else {
					num = __builtin_va_arg(ap, int);
				}
				length = user_PrintNum(buf, num, 16, 0, width, ladjust, padc, 0);
				OUTPUT(arg, buf, length);
				break;

			 case 'X':
				if (longFlag) {
					num = __builtin_va_arg(ap, long int);
				} else {
					num = __builtin_va_arg(ap, int);
				}
				length = user_PrintNum(buf, num, 16, 0, width, ladjust, padc, 1);
				OUTPUT(arg, buf, length);
				break;

			 case 'c':
				c = (char)__builtin_va_arg(ap, int);
				length = user_PrintChar(buf, c, width, ladjust);
				OUTPUT(arg, buf, length);
				break;

			 case 's':
				s = (char*)__builtin_va_arg(ap, char *);
				length = user_PrintString(buf, s, width, ladjust);
				OUTPUT(arg, buf, length);
				break;

			 case '\0':
				fmt --;
				break;

			default:
				OUTPUT(arg, fmt, 1);
		}
    	fmt++;
    }

    OUTPUT(arg, "\0", 1);
}

int user_PrintChar(char * buf, char c, int length, int ladjust)
{
    int i;

    if (length < 1) length = 1;
    if (ladjust) {
    *buf = c;
    for (i=1; i< length; i++) buf[i] = ' ';
    } else {
    for (i=0; i< length-1; i++) buf[i] = ' ';
    buf[length - 1] = c;
    }
    return length;
}

int user_PrintString(char * buf, char* s, int length, int ladjust)
{
    int i;
    int len=0;
    char* s1 = s;
    while (*s1++) len++;
    if (length < len) length = len;

    if (ladjust) {
    	for (i=0; i< len; i++) buf[i] = s[i];
    	for (i=len; i< length; i++) buf[i] = ' ';
    } else {
    	for (i=0; i< length-len; i++) buf[i] = ' ';
    	for (i=length-len; i < length; i++) buf[i] = s[i-length+len];
    }
    return length;
}

int user_PrintNum(char * buf, unsigned long u, int base, int negFlag, int length, int ladjust, char padc, int upcase)
{
    /* algorithm :
     *  1. prints the number from left to right in reverse form.
     *  2. fill the remaining spaces with padc if length is longer than
     *     the actual length
     *     TRICKY : if left adjusted, no "0" padding.
     *          if negtive, insert  "0" padding between "0" and number.
     *  3. if (!ladjust) we reverse the whole string including paddings
     *  4. otherwise we only reverse the actual string representing the num.
     */

    int actualLength =0;
    char *p = buf;
    int i;

    do {
    	long int tmp = u %base;
    	if (tmp <= 9) {
        	*p++ = '0' + tmp;
    	} else if (upcase) {
        	*p++ = 'A' + tmp - 10;
    	} else {
        	*p++ = 'a' + tmp - 10;
    	}
    	u /= base;
    } while (u != 0);

    if (negFlag) {
    	*p++ = '-';
    }

    /* figure out actual length and adjust the maximum length */
    actualLength = p - buf;
    if (length < actualLength) length = actualLength;

    /* add padding */
    if (ladjust) {
    	padc = ' ';
    }
    
    if (negFlag && !ladjust && (padc == '0')) {
    	for (i = actualLength-1; i< length-1; i++) buf[i] = padc;
    	buf[length - 1] = '-';
    } else {
    	for (i = actualLength; i< length; i++) buf[i] = padc;
    }


    /* prepare to reverse the string */
    int begin = 0;
    int end;
    if (ladjust) {
        end = actualLength - 1;
    } else {
        end = length -1;
    }

    while (end > begin) {
        char tmp = buf[begin];
        buf[begin] = buf[end];
        buf[end] = tmp;
        begin ++;
        end --;
    }

    /* adjust the string pointer */
    return length;
}


void
_user_panic(const char *file, int line, const char *fmt,...) {
    __builtin_va_list ap;


    __builtin_va_start(ap, fmt);
    writef("panic at %s:%d: ", file, line);
    user_lp_Print(user_myoutput, 0, (char *)fmt, ap);
    writef("\n");
    __builtin_va_end(ap);
	
    for(;;);
}