#include "misc.h"
#include <stdarg.h>

void (*writef_output) (char*);

#define FMT_STR            1
#define FMT_DEC            2
#define FMT_HEX            3
#define FMT_PTR            4
#define FMT_CUST           5
static int parse_fmt (const char** fmt_ptr)
{
    const char* fmt = *fmt_ptr;
    if (*fmt++ != '{')
        return 0;

    int kind = FMT_STR;
    for (; *fmt != '}'; fmt++) {
        switch (*fmt) {
        case 'd': kind = FMT_DEC; break;
        case 'x': kind = FMT_HEX; break;
        case 'p': kind = FMT_PTR; break;
        case '?': kind = FMT_CUST; break;
        case '\0': fmt--; goto fin;
        default: break;
        }
    }
    *fmt_ptr = fmt + 1;

 fin:
    return kind;
}

/// write formatted text to stream
void writef (const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char buf[64 + 1];
    u32 buf_pos = 0;

#define FLUSH_BUF do {                          \
        buf[buf_pos] = '\0';                    \
        if (buf_pos) writef_output(buf);        \
        buf_pos = 0;                            \
    } while (0)

    while (*fmt) {
        if (*fmt == '{') {
            FLUSH_BUF;
            i32 k; char* s;
            switch (parse_fmt(&fmt)) {
            case FMT_HEX:
                k = va_arg(args, i32);
                buf_pos = itoa(buf, k, 16);
                break;

            case FMT_DEC:
                k = va_arg(args, i32);
                buf_pos = itoa(buf, k, 10);
                break;

            case FMT_STR:
                s = va_arg(args, char*);
                writef_output(s);
                break;

            case FMT_CUST:
                {
                    void (*writer)(void*) = va_arg(args, void*);
                    void* p = va_arg(args, void*);
                    writer(p);
                    break;
                }

            default:
            case FMT_PTR:
                {
                    uptr p = (uptr) va_arg(args, void*);
                    const u32 len = sizeof(uptr) * 2;
                    buf[0] = '0';
                    buf[1] = 'x';
                    for (u32 i = 0; i < len; i++) {
                        char c;
                        if ((p & 0xf) < 10)
                            c = '0' + (p & 0xf);
                        else
                            c = 'a' + (p & 0xf) - 10;
                        buf[2 + len - i - 1] = c;
                        p = p >> 4;
                    }
                    buf_pos = 2 + len;
                }
            }

        }
        else {
            buf[buf_pos++] = *fmt++;
            if (buf_pos == (sizeof buf) - 1)
                FLUSH_BUF;
        }
    }
    FLUSH_BUF;

#undef FLUSH_BUF
}


/// make writef write to character buffer
static char* out_buf;
static u32 out_buf_cap;
static void write_to_out_buf (char* s)
{
    u32 i;
    for (i = 0; s[i] && i + 1 < out_buf_cap; i++) {
        out_buf[i] = s[i];
    }
    out_buf[i] = '\0';
    out_buf += i;
    out_buf_cap -= i;
}

void set_writef_output_buffer (char* buf, u32 cap)
{
    out_buf = buf;
    out_buf_cap = cap;
    writef_output = write_to_out_buf;
}
